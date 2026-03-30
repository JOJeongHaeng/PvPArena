# PvPArena Next Session Note

## Current State
- `main` is at commit `2ba7bdd` (`Add settings menu and lower default graphics load`).
- In-game settings menu was added and opens with `Esc`.
- Direct-IP lobby flow is still the active multiplayer flow.
- Default packaged graphics load was lowered:
  - DX11 default
  - Ray tracing off
  - Lumen GI off
  - Virtual Shadow Maps off
  - lower startup resolution/quality in `Config/DefaultGameUserSettings.ini`
- `Fab` plugin was explicitly disabled in the project to stop `UnrealEditor-Cmd` automation crashes.

## Verified
- Editor build passed.
- Automation passed:
  - `PvPArena.UI.HUDWidgetLayout`
  - `PvPArena.UI.HUDWidgetSettingsMenu`
  - `PvPArena.UI.HUDWidgetOnlineControls`
  - `PvPArena.UI.HUDWidgetBackgroundMusic`
  - `PvPArena.Character.AttackAudioDefaults`
  - `PvPArena.Network.DirectIpDefaults`

## Known Follow-Ups
- Repackage and retest on the friend's lower-spec PC.
- Check whether the remaining hitching is GPU-bound or caused by map content/post process.
- If performance is still poor, profile with `stat unit` and `stat gpu` in a packaged build.
- Decide whether to keep `Fab` disabled permanently or only for automation stability.
- Evaluate `G2: Mercenaries` only after current packaged performance is acceptable.

## Recommended Next Step
1. Package a fresh Development build.
2. Run it from a clean folder so old saved config does not override new defaults.
3. Verify performance on both PCs.
4. If needed, profile and reduce map/render cost before changing character assets.
