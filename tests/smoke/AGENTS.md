# tests/smoke/ — Pester boot/log smoke tests

PowerShell (Pester) tests, **Windows-only**. `*.tests.ps1` files doing quick
boot/crash/hang verification and basic config persistence checks — boot logs,
display config, audio config, difficulty/volume persistence, menu chrome
rendering. Run with:

```powershell
Invoke-Pester -Path tests/smoke/*.tests.ps1
```

These are the fastest signal in the test pyramid (boot + sanity, not full
scenarios) — prefer adding here over `tests/integration/` if you just need to
confirm the game still launches and logs look sane after a change.
