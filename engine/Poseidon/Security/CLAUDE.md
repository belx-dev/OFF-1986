# engine/Poseidon/Security/ — CD-key validation

Flat directory, one main file: `Serial.hpp` — `CDKey` class, 120-bit
(`KEY_BITS=120`, 15 bytes) fixed-size key. `DECODE_ON_GET=1` means the key is
kept encrypted at rest and only decrypted inside `Get`/`Check` calls (reduces
plaintext exposure in memory) — don't "simplify" this into decrypt-once-at-load.
`Check`/`GetValue` take an `offset` param for bit-level key validation.

## Tests

`tests/unit/engine/Poseidon/Security/` — decoder hardening, decoder fuzzing
PoC, server network exploits PoC. Small subsystem but security-sensitive —
treat any change here as needing extra scrutiny, and check whether a fuzz
harness in `apps/fuzzers` should cover it.
