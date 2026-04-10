# Lobby Menu And Korean UI Design

## Goal
- Add a lobby-only menu button in the top-right corner that opens the existing settings panel.
- Translate the code-built HUD text from English to Korean across lobby, settings, match, round-end, match-end, spectator help, and network status labels.

## Approach
- Reuse the existing `UPvPArenaHUDWidget` settings menu flow instead of creating a second settings path.
- Add a new button widget to the root overlay, position it in the top-right corner, and show it only while the lobby is visible.
- Keep all changes in the current C++ HUD implementation so the translation remains consistent with runtime-generated widgets.

## UI Scope
- Lobby panel text
- Settings panel text
- Match HUD labels
- Round-end announcement text
- Match-end summary text
- Spectator help text
- Network connection status text

## Constraints
- Preserve the current code-built UMG structure.
- Do not replace the existing settings panel behavior.
- Keep the new button hidden outside the lobby to avoid overlapping the combat HUD.

## Validation
- Extend layout automation to assert the new top-right lobby menu button exists.
- Extend settings/menu automation to assert the new button handler is exposed.
- Verify translated labels through focused HUD automation that reads widget text values.
