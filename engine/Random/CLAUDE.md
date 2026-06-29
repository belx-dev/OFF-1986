# engine/Random/ — deterministic PRNG

Header-mostly library (`randomGen.hpp`/`.cpp`) providing a seeded 32K-entry
lookup-table PRNG (`RandomTable`/`RandomGenerator`, singleton accessor
`GRandGen()`) used for reproducible world/NPC generation — determinism matters
here for replay/multiplayer consistency, don't swap in a different RNG without
checking callers. `isaac.hpp` (Isaac cipher) exists as an unused fallback.

Only depends on `engine/Poseidon/Foundation/platform.hpp`. Consumed by
`Poseidon`'s world/terrain generation and NPC behavior spawning.
