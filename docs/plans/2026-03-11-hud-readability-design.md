# HUD Readability Polish Design

## Summary

This pass improves HUD readability without changing data sources, ownership, or the current code-built UMG direction. The goal is to make the always-on status block easier to scan and the round-end announcement easier to notice during fast iteration playtests.

## Decisions

- Keep `UPvPArenaHUDWidget` as a runtime-built widget.
- Preserve the existing widget count and data flow.
- Add a background panel and stronger spacing to the left status cluster.
- Increase text hierarchy so health, score, timer, and round-end messaging read with clearer priority.
- Improve the health bar's visual weight and contrast.

## Rationale

The current HUD is functionally correct but visually flat. Every line reads with similar weight, so players must actively parse the text instead of quickly scanning it. A lightweight presentation pass can fix that without reopening the broader HUD architecture or introducing Blueprint UI debt.

Keeping the same widget structure limits regression risk. Existing layout tests already assert the main widget tree shape, so the polish pass can focus on styling, padding, alignment, and readable defaults rather than new UI behaviors.

## Implementation Scope

- Add a panel-like container feel to the status block through overlay styling and padding.
- Increase font sizes and text color contrast for status labels.
- Enlarge the health bar and keep its readable fallback text.
- Strengthen `ResultText` and `NextRoundText` emphasis for round-end readability.
- Extend automation coverage to verify key readability-related layout properties.

## Non-Goals

- Do not migrate to Widget Blueprint.
- Do not change HUD ownership or refresh logic.
- Do not add new gameplay information or round-state behavior.

## Verification

- Rebuild the editor target.
- Run `PvPArena.UI` automation tests.
- Re-check the widget layout test assertions for the new readability defaults.
