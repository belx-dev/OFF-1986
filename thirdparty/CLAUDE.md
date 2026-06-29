# thirdparty/ — vendored dependencies

Only two directories are vendored directly in-tree (everything else comes
via vcpkg, see `vcpkg.json` and root `CLAUDE.md`):

| Dir | License | Why vendored |
|---|---|---|
| `glad/` | Generated | OpenGL 4.5 Core loader — generated headers must match an exact loader version, vendoring avoids vcpkg version drift. |
| `renderdoc/` | MIT | RenderDoc in-application API header, for in-app capture hooks (`engine/Poseidon/Graphics/Shared/`) — header-only, small, rarely changes. |

This directory is **excluded from the repo's GPL-3.0-or-later license** (see
root `LICENSE` and `THIRD_PARTY_NOTICES.md`) — code here keeps its own
upstream license. Don't add new dependencies here casually; prefer adding to
`vcpkg.json` unless there's a specific reason (version pinning, no vcpkg
port, header-only single file) to vendor directly.
