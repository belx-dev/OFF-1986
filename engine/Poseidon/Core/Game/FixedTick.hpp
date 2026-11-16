#pragma once

#include <cstdint>

namespace Poseidon
{

// Fixed-tick accumulator (decision 0008, D1; roadmap package 2.1).
//
// Simulated time advances in whole quanta of 1/hz seconds. The game loop banks
// elapsed wall time multiplied by time acceleration with Add(), then runs
// World::Simulate once per ConsumeTick() with Quantum() as deltaT. Acceleration
// therefore runs more ticks of the same size (D1.4); the quantum never grows.
//
// Pure arithmetic with no engine dependencies, so tests/unit covers it directly.
class FixedTickAccumulator
{
  public:
    // hz <= 0 disables the accumulator (the variable-step path stays).
    // maxBacklogSeconds is the catch-up budget per loop iteration at 1x (D1.3).
    void Configure(int hz, float maxBacklogSeconds = 0.3f)
    {
        _quantum = hz > 0 ? 1.0f / static_cast<float>(hz) : 0.0f;
        _maxBacklog = maxBacklogSeconds;
        _backlog = 0.0f;
    }

    bool Enabled() const { return _quantum > 0.0f; }
    float Quantum() const { return _quantum; }

    // Bank simulated seconds (wall seconds x acceleration). The backlog is capped
    // at the catch-up budget x acceleration: beyond that the excess is dropped
    // and simulated time falls behind wall time (D1.3).
    void Add(float simulatedSeconds, float acceleration)
    {
        if (simulatedSeconds > 0.0f)
            _backlog += simulatedSeconds;
        const float cap = _maxBacklog * (acceleration > 1.0f ? acceleration : 1.0f);
        if (_backlog > cap)
            _backlog = cap;
    }

    // A whole tick is due. A backlog within 0.01% of a quantum counts, so that
    // banked float sums such as 3 x 0.1 s / (1/30 s) do not lose a tick to rounding.
    bool TickDue() const { return Enabled() && _backlog >= _quantum * (1.0f - 1e-4f); }

    // How many ConsumeTick() calls would succeed now.
    int TicksDue() const
    {
        if (!Enabled())
            return 0;
        return static_cast<int>((_backlog + _quantum * 1e-4f) / _quantum);
    }

    // Take one quantum off the backlog if a whole tick is due.
    bool ConsumeTick()
    {
        if (!TickDue())
            return false;
        _backlog -= _quantum;
        if (_backlog < 0.0f)
            _backlog = 0.0f;
        ++_ticks;
        return true;
    }

    // Drop the backlog, so resuming from pause does not burst catch-up ticks (D1.5).
    void Reset() { _backlog = 0.0f; }

    float Backlog() const { return _backlog; }

    // Monotonic tick counter: the simulation's clock for diagnostics (D1.8).
    uint64_t TickCount() const { return _ticks; }

    // Wall seconds until the next tick is due at the given acceleration.
    float SecondsUntilNextTick(float acceleration) const
    {
        if (!Enabled() || TickDue())
            return 0.0f;
        return (_quantum - _backlog) / (acceleration > 0.0f ? acceleration : 1.0f);
    }

  private:
    float _quantum = 0.0f;
    float _maxBacklog = 0.3f;
    float _backlog = 0.0f;
    uint64_t _ticks = 0;
};

} // namespace Poseidon
