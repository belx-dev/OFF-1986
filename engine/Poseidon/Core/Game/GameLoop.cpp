#include <Poseidon/Core/Game/GameLoop.hpp>
#include <Poseidon/Core/Game/FixedTick.hpp>
#include <Poseidon/Core/Config/EngineConfig.hpp>
#include <Poseidon/Core/Config/Config.hpp>
#include <Poseidon/Foundation/Platform/AppConfig.hpp>
#include <Poseidon/Core/Application.hpp>
#include <Poseidon/Core/PendingConnect.hpp>
#include <Poseidon/Core/Global.hpp>
#include <Poseidon/Audio/IAudioSystem.hpp>
#include <Poseidon/Input/CheatCode.hpp>
#include <Poseidon/World/World.hpp>
#include <Poseidon/World/Scene/Scene.hpp>
#include <Poseidon/World/Scene/Camera/Camera.hpp>
#include <Poseidon/Dev/Debug/DebugTrap.hpp>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_scancode.h>
#include <string>
#include <Poseidon/Foundation/Common/FltOpts.hpp>
#include <Poseidon/Foundation/Framework/AppFrame.hpp>
#include <Poseidon/Foundation/Framework/Log.hpp>
#include <Poseidon/Foundation/Types/Memtype.h>
#include <Poseidon/Foundation/Memory/MemFreeReq.hpp>
#include <Poseidon/Foundation/platform.hpp>

using namespace Poseidon::Dev;
#include <Poseidon/Graphics/Core/Engine.hpp>
#include <Poseidon/Foundation/Common/Win.h>
#include <Poseidon/Input/InputSubsystem.hpp>
#include <Poseidon/Graphics/Rendering/Draw/Font.hpp>

#include <SDL3/SDL.h>

// External references (defined at global ns in Input subsystem)
extern void ProcessMouse(DWORD timeDelta);
extern void ProcessKeyboard(DWORD sysTime, DWORD timeDelta);
extern void ProcessJoystick();
extern bool IsMouseAcquired();
extern void SDLInput_DispatchUIKeys();

namespace Poseidon
{

// User-driven FPS cap (Graphics screen → graphics.cfg → fpsCap field).
// 0 = uncapped; nonzero values trigger the same sleep-to-target path
// used by the debug "limit FPS" cheat.  Read by the frame-pacer at the
// bottom of RenderFrame.
int gUserFpsCap = 0;

// Idle heartbeat used by blocking waits (progress screen, debug window,
// triWaitFrames). Tickles the watchdog and pumps OS events so the window
// stays responsive; does not run a full AppIdle — callers that want that
// call AppIdle() directly.
void ProcessMessagesNoWait()
{
    GDebugger.ProcessAlive();
    if (GEngine)
        GEngine->HandleEvents();
    else
        SDL_PumpEvents();
}

// Fixed tick (decision 0008 D1, package 2.1). Off by default: --fixed-tick=0 keeps
// the variable step, where World::Simulate runs once per frame with the frame time.
// With --fixed-tick=N the frame time (x acceleration) is banked and World::Simulate
// runs once per due tick with the constant quantum. Only the last tick of a batch
// draws, so presentation stays once per frame.
//
// Input is sampled once per frame and its edge flags (keysToDo, fireToDo) are
// cleared every frame, so FixedTickFrameBegin decides *before* input sampling
// whether this frame runs a tick and skips the frame otherwise. In a catch-up
// batch every tick of the batch sees the same frame's flags; that only happens
// when the machine cannot keep up and goes away with 2.2.
static FixedTickAccumulator gFixedTick;

static void ConfigureFixedTick()
{
    static bool configured = false;
    if (configured)
        return;
    configured = true;
    gFixedTick.Configure(AppConfig::Instance().GetFixedTickHz());
    if (gFixedTick.Enabled())
        LOG_INFO(Core, "Fixed tick enabled: {} Hz ({:.2f} ms quantum)", AppConfig::Instance().GetFixedTickHz(),
                 gFixedTick.Quantum() * 1000.0f);
}

// Called once per loop iteration before input is sampled. Banks the frame time
// and returns false when no tick is due, after resting until the next one.
static bool FixedTickFrameBegin(float deltaT)
{
    ConfigureFixedTick();
    if (!gFixedTick.Enabled() || !GWorld)
        return true;
    if (!GWorld->IsSimulationEnabled())
    {
        // Paused: game time does not advance. UI, camera and drawing keep their
        // per-frame cadence, and the backlog is dropped so resuming does not
        // burst catch-up ticks (D1.5).
        gFixedTick.Reset();
        return true;
    }
    // Acceleration multiplies the accumulator input (D1.4). The value is the one
    // the previous tick left behind; a change made by this frame's input takes
    // effect from the next frame.
    const float acceleration = GWorld->GetAcceleratedTime();
    gFixedTick.Add(deltaT * acceleration, acceleration);
    if (gFixedTick.TickDue())
        return true;
    // Rest until the next tick instead of spinning; round up so the next frame
    // is due rather than one Sleep() granule short.
    const int sleepMs = static_cast<int>(gFixedTick.SecondsUntilNextTick(acceleration) * 1000.0f + 0.999f);
    if (sleepMs > 0)
        Sleep(sleepMs);
    return false;
}

static void FixedTickReportPace(float acceleration)
{
    // Pace diagnostics every ~5 s of wall time (profiling only; nothing here decides
    // what is simulated, decision 0008 D4.1).
    static DWORD lastReport = 0;
    static uint64_t lastTicks = 0;
    static int frames = 0;
    ++frames;
    const DWORD now = Poseidon::Foundation::GlobalTickCount();
    if (lastReport == 0)
        lastReport = now;
    else if (now - lastReport >= 5000)
    {
        const float seconds = (now - lastReport) * 0.001f;
        LOG_INFO(Core, "Fixed tick pace: {:.1f} ticks/s ({:.0f} expected), {:.1f} frames/s, backlog {:.0f} ms",
                 (gFixedTick.TickCount() - lastTicks) / seconds, acceleration / gFixedTick.Quantum(), frames / seconds,
                 gFixedTick.Backlog() * 1000.0f);
        lastReport = now;
        lastTicks = gFixedTick.TickCount();
        frames = 0;
    }
}

static void SimulateWorld(float deltaT, bool& enableDraw)
{
    ConfigureFixedTick();
    if (!gFixedTick.Enabled() || !GWorld->IsSimulationEnabled())
    {
        // Variable step, or paused under the fixed tick (see FixedTickFrameBegin).
        if (gFixedTick.Enabled())
            gFixedTick.Reset();
        GWorld->Simulate(deltaT, enableDraw);
        return;
    }

    const float acceleration = GWorld->GetAcceleratedTime();
    const int ticks = gFixedTick.TicksDue();
    for (int i = 0; i < ticks && gFixedTick.ConsumeTick(); ++i)
    {
        bool drawThisTick = enableDraw && i == ticks - 1;
        GWorld->SetFixedTick(true);
        GWorld->Simulate(gFixedTick.Quantum(), drawThisTick);
        if (i == ticks - 1)
            enableDraw = drawThisTick;
        if (!GWorld)
            return; // the tick replaced the world; the next frame starts over
        GWorld->SetFixedTick(false);
        if (Glob.exit)
            return;
        if (!GWorld->IsSimulationEnabled())
        {
            // The tick paused the game: the rest of the batch must not run.
            gFixedTick.Reset();
            break;
        }
    }
    FixedTickReportPace(acceleration);
}

void RenderFrame(float deltaT, bool enableDraw)
{
    if (Glob.exit)
    {
        GApp->m_closeRequest = true;
    }
    if (!GWorld)
    {
        return;
    }

    int startTime = Poseidon::Foundation::GlobalTickCount();
    GDebugger.NextAliveExpected(10000);

    GWorld->SetSimulationFocus(enableDraw);
    if (GSoundsys)
        GSoundsys->SetSimulationRunning(GWorld->IsSimulationEnabled());
    SimulateWorld(deltaT, enableDraw);

    GApp->m_forceRender = false;

#if _ENABLE_CHEATS
    static int limitFpsCoef = 0;
#define limitFps (limitFpsCoef > 0 ? 40 / limitFpsCoef : 0)
    auto& renderInput = InputSubsystem::Instance();
    if (renderInput.GetCheat2ToDo(SDL_SCANCODE_S))
    {
        limitFpsCoef++;
        if (limitFpsCoef > 4)
            limitFpsCoef = 0;
        GlobalShowMessage(500, "Limit FPS %d", limitFps);
    }
#else
    const bool limitFps = false;
    auto& renderInput = InputSubsystem::Instance();
#endif

    // PAUSE key toggles HW T&L.  All F-keys are taken (F1=help,
    // F2-F11 = units / lang via DisplayMain, F12 = language cycle
    // via displayUI.hpp:1040), and modifier-prefixed keys don't pass
    // through ConsumeKeyPress; PAUSE has no other handler.
    extern bool EnableHWTLState;
    if (renderInput.ConsumeKeyPress(SDL_SCANCODE_PAUSE))
    {
        EnableHWTLState = !EnableHWTLState;
        GlobalShowMessage(500, "DX T&L %s", EnableHWTLState ? "On" : "Off");
        LOG_INFO(Graphics, "DX T&L toggled to {} (PAUSE)", EnableHWTLState ? "On" : "Off");
    }

    if (!enableDraw || GWorld->GetRenderingDisabled() || limitFps)
    {
#if _ENABLE_CHEATS
        int maxFps = limitFpsCoef ? limitFps : 50;
#else
        const int maxFps = 50;
#endif
        const int minMsPerFrame = 1000 / maxFps;

        int durationMs = Poseidon::Foundation::GlobalTickCount() - startTime;
        int sleepMillis = minMsPerFrame - durationMs;
        if (sleepMillis > 0)
        {
            Sleep(sleepMillis);
        }
    }
    else if (gUserFpsCap > 0)
    {
        // User-driven FPS cap from the Graphics screen — same sleep-
        // to-target shape as the unfocused / debug-limit branches
        // above, but applied during normal gameplay rendering.  Soft
        // cap (sleep-based) — cheap to implement, accurate to within
        // OS scheduler granularity (~1 ms on Win10+).
        const int minMsPerFrame = 1000 / gUserFpsCap;
        int durationMs = Poseidon::Foundation::GlobalTickCount() - startTime;
        int sleepMillis = minMsPerFrame - durationMs;
        if (sleepMillis > 0)
            Sleep(sleepMillis);
    }
}

bool AppIdle()
{
    // Service a deferred re-mount request between frames (set by triRemount / dev panel),
    // before any simulate/draw touches the world it tears down. The reload itself runs
    // StartIntro, so consume this tick and resume on the rebuilt world next call.
    if (GApp->m_remountRequested)
    {
        GApp->m_remountRequested = false;
        if (GApp->m_remountHasModPath)
            GApp->ReloadGameContentWithMods(GApp->m_remountModPath.c_str());
        else
            GApp->ReloadGameContent();
        return true;
    }

    // Finish a deferred MP join: after a mod-apply re-mount lands back on the menu,
    // connect to the server stashed before the re-mount.
    // Armed only alongside a re-mount request, so the re-mount tick above always
    // runs first and this never fires on the torn-down world; it fires once the
    // rebuilt menu (GModeIntro + Options) is live.
    if (Poseidon::GPendingConnect().IsArmed() && GWorld != nullptr && GWorld->GetMode() == GModeIntro &&
        GWorld->Options() != nullptr)
    {
        Poseidon::PendingConnect& pc = Poseidon::GPendingConnect();
        // Drive the non-blocking deferred connect one step per frame: it starts host
        // enumeration, then later frames poll + join. Do NOT return here — the rest of
        // the frame (SDL pump / simulate) is what advances the enumeration between
        // steps, so blocking the loop the way WaitForSession does would deadlock it.
        extern bool __cdecl CreateClientDeferred(RString ip, int port, RString password);
        if (CreateClientDeferred(RString(pc.Address().c_str()), pc.Port(), RString(pc.Password().c_str())))
        {
            pc.Disarm();
        }
    }

    // Report a failed mod re-mount on the rolled-back menu, once it is live again
    // (same GModeIntro + Options gate as the deferred MP-join above).
    if (GApp->m_remountFailed && GWorld != nullptr && GWorld->GetMode() == GModeIntro && GWorld->Options() != nullptr)
    {
        GApp->m_remountFailed = false;
        extern void __cdecl ReportRemountFailure();
        ReportRemountFailure();
    }

    // Once-per-frame memory maintenance, off the allocation path: when over the
    // soft watermark it trims caches to their declared budgets, and over the hard
    // ceiling it claws the overflow back with cost-ordered eviction. A handful of
    // atomic loads when under budget (the common case — both limits default off).
    Poseidon::Foundation::MemoryFrameMaintenance();

    bool focused = (GApp->m_keepFocus || GApp->m_appActive) && !GApp->m_appPaused && !GApp->m_appIconic;
    if (GSoundsys)
        GSoundsys->Activate(focused);
    const bool testMissionActive = !AppConfig::Instance().GetTestMissionPath().empty();
    bool enableDraw = (focused || (GApp->m_forceRender || testMissionActive) &&
                                      (ENGINE_CONFIG.landEditor || ENGINE_CONFIG.useWindow));

    static DWORD lastTime;
    DWORD actTime = Poseidon::Foundation::GlobalTickCount();
    DWORD deltaTMs = actTime - lastTime;

    if (!enableDraw)
    {
        if (deltaTMs < 50)
        {
            Sleep(50 - deltaTMs);
            actTime = Poseidon::Foundation::GlobalTickCount();
            deltaTMs = actTime - lastTime;
        }
    }

    float deltaT = deltaTMs * 0.001;
    lastTime = actTime;

    static DWORD lastSysTime;
    DWORD sysTime = ::GetTickCount();
    DWORD timeDelta = sysTime - lastSysTime;
    lastSysTime = sysTime;

    saturateMin(deltaT, 0.3);

    auto& input = InputSubsystem::Instance();

#if _ENABLE_CHEATS
    static bool fixedSimulation = false;
    if (input.GetCheat2ToDo(SDL_SCANCODE_O))
    {
        fixedSimulation = !fixedSimulation;
        GEngine->ShowMessage(500, "Fix Sim %s", fixedSimulation ? "On" : "Off");
    }
    if (fixedSimulation)
    {
        deltaT = 1.0 / 10;
    }
#endif

    // Fixed tick: skip this frame, before any input is sampled, if no tick is due.
    if (!FixedTickFrameBegin(deltaT))
        return false;

    if (GApp->m_canRender)
    {
        // Dispatch UI key events buffered by SDL input processing
        SDLInput_DispatchUIKeys();
        if (ENGINE_CONFIG.useWindow && !IsMouseAcquired() || !enableDraw)
        {
            RenderFrame(deltaT, enableDraw);
        }
        else
        {
            ProcessMouse(timeDelta);
            ProcessKeyboard(sysTime, timeDelta);

            if (input.CheatActivated() == CheatFreeze)
            {
                input.CheatServed();
                for (;;)
                {
                }
            }

            if (input.IsJoystickEnabled())
            {
                ProcessJoystick();
            }

            input.Update();

            if (input.FreelookChanged())
            {
                if (GWorld)
                {
                    GWorld->FreelookChange(input.IsLookAroundEnabled());
                }
            }

            Object* cam = GWorld->CameraOn();
            if (cam)
            {
                cam->DetectControlMode();
            }

            RenderFrame(deltaT, true);
        }
        return false;
    }
    else
    {
        return true;
    }
}

} // namespace Poseidon
