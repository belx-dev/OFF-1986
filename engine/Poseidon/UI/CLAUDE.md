# engine/Poseidon/UI/ — menus, editor, options, localization

Most major UI areas follow a **module pattern**: a `*Module.hpp` with a
`Register()` entry point (e.g. `EditorModule`, `CampaignsModule`,
`MultiplayerModule`, `ModsModule`).

## Subfolder map

| Dir | Contents |
|---|---|
| `Campaigns/` | Campaign selection/progression (`CampaignsModule`). |
| `Controls/` | UI widgets (buttons, menus, pages). |
| `Editor/` | Mission editor UI (`EditorModule`). |
| `InGame/` | Pause menu / pause-state UI. |
| `Locale/` | Localization: language registry, mission-language detection, HTML localization, autocomplete. |
| `Locale/Stringtable/` | `Stringtable.hpp` — string table, codepage transcoding, Meyer's singleton. |
| `Map/` | Map UI / editor map view. |
| `Missions/` | Single-mission UI. |
| `Mods/` | Mod browser/installer (`ModsModule`). |
| `Multiplayer/` | Lobby, server list (`MultiplayerModule`). |
| `Options/` | Game options pages (`DisplayPage`, `AudioPage`, etc.). |
| `Settings/` | Settings persistence. |
| `Text/` | Text rendering helpers. |

## Key files

- `Locale/Stringtable/Stringtable.hpp` — Meyer's singleton `GetLanguage()`,
  `LoadStringtable`, `LocalizeString` (by int ID or raw string),
  `LookupStringtableCsv` (per-mission fallback that doesn't pollute the
  global mission table).
- `Locale/Languages.hpp` / `SupportedLanguages.hpp` — language enum/registry.
- `Locale/MissionLanguageDetector.hpp` — auto-detects a mission's language.
- `Locale/Stringtable/CodepageTranscode.hpp` — UTF8/ANSI conversion.
- `OptionsUI.hpp` / `GameModule.hpp` — top-level options UI / module base class.

## Gotchas

- `Stringtable` is the one explicitly thread-safe-by-design Meyer's singleton
  in the engine (most of `Poseidon` is not thread-safe) — don't assume the
  same guarantee elsewhere just because it holds here.
- `LookupStringtableCsv` is deliberately separate from the global
  `LocalizeString` path so per-mission strings don't leak into global state —
  use it (not a manual table merge) for mission-scoped strings.
- New UI surfaces should follow the existing Module pattern
  (`*Module.hpp` + `Register()`) for consistency with Editor/Campaigns/Multiplayer/Mods.

## Tests

`tests/unit/engine/Poseidon/UI/` — 30+ tests across `Controls/`, `InGame/`,
`Locale/`, `Map/`, `Settings/`: options pages (audio/display/graphics/gamepad
+ tuning), pause overlay, cursor overlay, button modal/flat menu/confirm
pages, font rendering, stringtable, map/mission UI, settings persistence.
