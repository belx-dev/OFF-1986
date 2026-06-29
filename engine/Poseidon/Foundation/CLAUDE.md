# engine/Poseidon/Foundation/ — low-level utilities and primitives

Everything else in the engine depends on this. Replaces STL with custom
containers/strings/memory; read this before touching anything else in
`Poseidon` if you're not already familiar with the codebase's idioms.

## Subfolder map

| Dir | Contents |
|---|---|
| `Algorithms/` | CRC, binary search, sorting, spline interpolation. |
| `Common/` | Platform paths, game paths, player prefs, NET/console utils. |
| `Containers/` | `AutoArray`, `SmallArray`, `BigArray`, `HashMap`, quadtree, bitmask. |
| `Enums/` | Enum-name registry system. |
| `Framework/` | App frame, console base, logging hooks, timing, global lifecycle. |
| `Logging/` | spdlog-based `LoggingSystem` with category filtering. |
| `Math/` | Vectors/matrices/quaternions, interpolation, fixed-point, statistics. |
| `Memory/` | `FastAlloc` (pool allocator), `MemGrow`, `MemHeap`, allocator policies, memory-budget tracking. |
| `Modules/` | Module registration/lifetime system. |
| `PCH/` | Shared precompiled headers (`stdIncludes.h`, etc.). |
| `Platform/` | App init (`AppConfig`, `PoseidonInit`), crash handler, FPU setup, Win/Linux/POSIX-specific init. |
| `Strings/` | `RString` (ref-counted copy-on-write string), format utilities, MBCS support. |
| `Threads/` | `PoCritical` (mutex), `PoSemaphore`, `PoThread`. |
| `Time/` | Frame timing. |
| `Types/` | `Ref`/`RefCount` smart pointers, intrusive linked list, `ScopeLock`, `EnumDecl` macro. |

## Key files

- `Containers/Array.hpp` — `AutoArray`, the workhorse container; explicit
  construct/destruct via `ModernTraits` (move-aware, C++11-style).
- `Memory/FastAlloc.hpp` — pool allocator, opt in via `USE_FAST_ALLOCATOR` macro for hot classes.
- `Memory/MemAlloc.hpp` — allocator policies (`MemAllocD` heap, `MemAllocS` static-with-fallback, ...).
- `Strings/RString.hpp` — ref-counted string; identical strings share allocation, splits (copies) on mutation.
- `Logging/Logging.hpp` — spdlog-wrapped `LoggingSystem`, `Category` enum, `appTag` tracking.
- `Types/Pointers.hpp` — `Ref`/`RefCount` ownership templates.
- `Platform/PoseidonInit.hpp` — init-callback registration; the actual entry point for engine foundation setup.

## Gotchas

- **Don't use namespace-scope static objects with constructors for init** —
  use `Poseidon::InitDefaults()`/init-callback registration instead; static
  init order across translation units is not guaranteed and linkers can drop
  unreferenced TUs.
- `AutoArray` et al. take an explicit allocator template param and call
  `DoConstruct()`/`DoDestruct()` via traits — don't assume standard
  copy/move semantics without checking the traits.
- `RString` is copy-on-write — mutating a shared string copies first; don't
  assume `&str[0]` is stable across a mutating call on an aliased string.

## Tests

`tests/unit/engine/Poseidon/Foundation/{Algorithms,Common,Containers,Enums,Framework,Math,Memory,Modules,Platform,Strings,Threads,Time,Types}/`
