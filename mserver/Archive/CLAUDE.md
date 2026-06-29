# mserver/Archive/ — `papa-bear-archive`

Byte-compatible PBO (Packed Bank Of files) archive codec — reads/writes
`.pbo` mod packages compatibly with `engine/Poseidon/IO/Streams/QBStream.cpp`.
Implements LZSS compression and PBO entry serialization independently in
Rust (not a binding to the C++ code).

- `src/lib.rs` — public API: `Pbo`, `PboEntry`, MIME constants.
- `src/pbo.rs` — PBO format reader/writer.
- `src/lzss.rs` — LZSS decompressor.

Zero dependency on other `mserver/` crates. Deps: `walkdir`, `anyhow`.
Test with `cargo test` (uses `tempfile` as a dev-dependency).

If you change PBO framing in `engine/Poseidon/IO/Streams/`, this crate's
`pbo.rs`/`lzss.rs` need a matching update — they're parallel implementations,
not shared code.
