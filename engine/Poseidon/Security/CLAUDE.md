# engine/Poseidon/Security/ — CD-key validation (stubbed in this fork)

Flat directory: `Serial.hpp`/`Serial.cpp` (`CDKey` class) + `XOR1024.cpp`.

**The implementation is already neutered in this fork**: `Serial.cpp` says
"CD-key / RSA validation is unused: these methods are no-op stubs" — `Init()`
and `Encrypt()` are empty, `Check()` always returns `true`, `GetValue()`
returns `0`. The symbols are kept only so the CD-key publishing path in
`NetworkServerMission` still links; MP identity is handled by `NetworkConfig`
and `MultiplayerAuth` instead. The header still declares the original design
(120-bit key, `KEY_BITS=120`; `DECODE_ON_GET=1` decrypt-inside-`Get`/`Check`),
but don't mistake the header for live behavior.

Modernization note: this is retail-era DRM inherited from the original engine
and a clear candidate for full removal once the remaining link-time consumers
are cleaned up (see root `CLAUDE.md`, Modernization posture).

## Tests

`tests/unit/engine/Poseidon/Security/` — decoder hardening, decoder fuzzing
PoC, server network exploits PoC. Anything touching auth/identity remains
security-sensitive — scrutinize changes and consider whether a fuzz harness
in `apps/fuzzers` should cover them.
