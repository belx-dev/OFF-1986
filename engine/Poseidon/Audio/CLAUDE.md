# engine/Poseidon/Audio/ — abstract audio engine

Backend-pluggable: this subsystem defines the interface + state model + a
`Dummy` no-op backend; the real implementation is `engine/PoseidonOpenAL`.

## Subfolder map

| Dir | Contents |
|---|---|
| `Core/` | Playback state machine, lifecycle (`AudioState`, `WaveLifecycle`, `WaveLogic`). |
| `Core/Format/` | Wave/RIFF parsing, PCM/delta-pack samples, wave buffers. |
| `Capture/` | Microphone input abstraction. |
| `Dummy/` | No-op backend. |
| `Shared/` | Audio math, DX8 reference implementation, PCM cache. |
| `Streaming/` | Streaming playback management. |
| `Text/` | Audio-related UI text. |
| `Voice/` | VoN (voice-over-network) VOIP: `OpusCodec`, `VoiceBackend`, `MicLoopback`, `VonApp`, `VonBuffer`. |

## Key files

- `Core/AudioState.hpp` — ~40-field POD state struct (playing, position, 3D
  pos/vel, volume, frequency, channels) — no behavior, just data.
- `AudioFactory.hpp` — backend registration (`AudioBackendDescriptor`:
  codeName/displayName/priority + create/isAvailable function pointers); how
  `PoseidonOpenAL` plugs in.
- `Core/WaveLifecycle.hpp` — playback lifecycle tracking.
- `Voice/OpusCodec.hpp` — VOIP compression.
- `Shared/AudioMath.hpp` — volume/Doppler calculations.

## Gotchas

- `AudioState` is pure POD; updates follow a "commit" pattern for async
  playback changes rather than direct mutation mid-flight.
- `WaveKind` (`WaveEffect`/`WaveMusic`/`WaveSpeech`) drives volume
  accommodation rules — picking the wrong kind changes how this sound
  ducks/is ducked by others.
- 3D audio fields (position/velocity) feed Doppler + spatial mixing — leaving
  velocity at zero silently disables Doppler for that source.

## Tests

`tests/unit/engine/Poseidon/Audio/` — 24+ files: init, wave lifecycle, voice
budget, VON integration, VOIP codec, mixing parity, DX8/EAX/EFX reference,
PCM cache, OpenAL loopback, capture, device preview offset.
