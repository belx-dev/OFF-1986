# engine/Poseidon/Network/ — multiplayer transport and master-server protocol

## Subfolder map

| Dir | Contents |
|---|---|
| `Legacy/` | Legacy protocol code (may be superseded — check before extending). |
| `XML/` | XML-based protocol layer. |

## Key files

- `Network.hpp` — `NetworkMessage` struct (time, size, values array, `RefCount` base).
- `NetworkManagerState.hpp` — server/client state resolver (`simulateMode`, `hasServer`, `hasClient` logic).
- `IntegrityCheck.hpp` — CRC calculation over exe/data files.
- `IpBan.hpp` — IPv4 parse/format, ban-list load/save. IPv4 is a canonical
  `uint32` in **host order**: `a<<24 | b<<16 | c<<8 | d`, via
  `ParseIPv4`/`FormatIPv4`.
- `MessageFactory.hpp` — message creation abstraction.
- `MultiplayerAuth.hpp` — auth protocol.
- `MasterServerBrowser.hpp` / `MasterServerPublisher.hpp` — query/advertise
  against the master server (the Rust `mserver/` stack — see `mserver/CLAUDE.md`;
  wire-compatible, not code-shared).
- `RateLimit.hpp` — rate-limit enforcement.

## Gotchas

- `BanMode` (`Id`/`Ip`/`Both`) determines exactly what `Ban()` records —
  don't assume banning by ID also blocks the IP unless mode is `Both`.
- `NetworkMessage` holds **temporary** pointers
  (`objectUpdateInfo`/`objectServerInfo`) valid only for the duration of an
  `OnSendComplete` callback — don't retain them past that callback.
- `TRANSFER_FORMAT` macro optimizes indexed message-format field access with
  **no bounds checks in NDEBUG builds** — this is exactly the kind of code
  the `fuzz_decode_msg` harness (`apps/fuzzers`) targets; be careful with
  untrusted input here.
- This C++ code defines the wire protocol that the Rust `mserver/Client`
  crate independently re-implements (transcribed, not shared) — if you change
  framing/CRC here, `mserver/Client/src/codec.rs` and `framing.rs` need a
  matching update.

## Tests

`tests/unit/engine/Poseidon/Network/` — 15+ tests: multiplayer auth, IP ban,
chat UTF8 wire format, master-server version tags, player mute/ignore,
transport hardening, wire bounds checks, MP join validation, network
dispatch, script-value codec, encoded matrix3, legacy/XML protocol tests.
