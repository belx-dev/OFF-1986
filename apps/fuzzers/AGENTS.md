# apps/fuzzers/Fuzzer/ — libFuzzer harnesses

16 independent libFuzzer executables targeting asset-format parsers and
network message decoding, built only when `POSEIDON_BUILD_FUZZERS` is on
(`*-fuzz` CMake presets). Each harness is `fuzz_<thing>.cpp` implementing
`extern "C" int LLVMFuzzerTestOneInput(const uint8_t*, size_t)`.

Targets: `fuzz_decode_msg`, `fuzz_rtm`, `fuzz_p3d`, `fuzz_paa`,
`fuzz_paramfile`, `fuzz_pbo`, `fuzz_wrp`, `fuzz_wss`, `fuzz_stringtable`,
`fuzz_wav`, `fuzz_sqf`, `fuzz_sqs`, `fuzz_sqf_exec`, `fuzz_savegame`,
`fuzz_lip`, `fuzz_shape`.

- `fuzz_decode_msg.cpp` targets `NetworkComponent::DecodeMessage()` (the
  server's wire-decode path) — deliberately skips per-message `OnMessage()`
  handlers to keep the fuzz surface to parsing only.
- Built with ASan+UBSan+SanitizerCoverage; `NDEBUG` is active (matching
  release/server builds) so `PoseidonAssert`/bounds checks compile out and
  real OOB defects surface instead of asserting first.
- Each harness has its own corpus/crash-seed directory; they are not wired
  together.

If you add a new binary parser anywhere in `engine/Poseidon/Asset/Formats/`
or `engine/Poseidon/Network/`, consider whether it needs a fuzz harness here.
