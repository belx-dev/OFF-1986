# mserver/Client/ — `papa-bear-client`

PapaBear protocol client SDK: a UDP session-enumeration query codec for
probing game-server status, transcribed from the engine's
`NetTransportNetDecls.hpp`/`NetGlobal.hpp` framing/CRC. Phase 1 (current):
server-status queries. Phase 2 (planned): C ABI export for direct engine consumption.

- `src/lib.rs` — public modules: `codec`, `framing`, `query`.
- `src/codec.rs` — `EnumPacket`/`SessionPacket` encoding, engine-compatible `#[repr(packed)]` little-endian layout.
- `src/query.rs` — `query_server()`, a live UDP round-trip probe.
- `examples/probe.rs` — example usage.

**Zero runtime dependencies by design** (keeps the door open for the planned
C ABI export). Test with `cargo test` (includes a standalone UDP socket example).

Consumed by `mserver/CLI` (server query command) and `mserver/MasterService`
(reachability `probe` mode) — this is the one crate the other two share.
