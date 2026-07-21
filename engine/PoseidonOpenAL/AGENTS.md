# engine/PoseidonOpenAL/ — OpenAL audio backend

Implements the abstract audio interface declared in `engine/Poseidon/Audio/`
(see `engine/Poseidon/Audio/CLAUDE.md`): sound mixing/playback, voice-over-network
(VoN) capture/playback, and microphone loopback.

## Key files

- `SoundSystemOAL.hpp`/`.cpp` — main sound system implementation.
- `WaveOAL.hpp`/`.cpp` — wave file streaming/playback.
- `Voice/VoiceBackendOpenAL.cpp` — VoN backend.
- `Voice/VoNCaptureOpenAL.cpp`, `Voice/MicLoopbackOpenAL.cpp` — capture/mic loopback.
- `EFXPresets.hpp` — environmental audio effect presets.
- `OpenALRuntime.hpp` — OpenAL context lifecycle.

Depends on `Poseidon`, `OpenAL::OpenAL` (vcpkg), and `Avrt` on Windows (audio
thread priority). Linked by every GUI/tool target that needs sound
(`apps/cwr/Game`/`GameDemo`, `apps/tetris`, `apps/tools/Studio`/`Tools`).
`apps/cwr/Server` does not link this.
