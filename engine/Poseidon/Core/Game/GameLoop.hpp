#pragma once

namespace Poseidon
{

extern int gUserFpsCap;

void ProcessMessagesNoWait();
// Fixed tick (--fixed-tick, decision 0008 D1). Every loop that calls RenderFrame
// (AppIdle, AppServerLoop) must call this first, before it samples input, with the
// frame's wall deltaT. Returns false when the frame must be skipped: no tick is
// due, and the call has already rested until the next one.
bool FixedTickFrameBegin(float deltaT);
void RenderFrame(float deltaT, bool enableDraw);
bool AppIdle();

} // namespace Poseidon
