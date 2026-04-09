# Overhead Widget Soft Class Design

**Goal:** Reduce default character-side memory pressure by removing eager loading of the overhead status widget blueprint class.

**Recommended Approach:** Store the overhead widget blueprint as a `TSoftClassPtr<UUserWidget>` on `APvPArenaCharacter`, and resolve it only when the widget component is about to initialize its runtime widget. Keep the native `UPvPArenaOverheadStatusWidget` class as a fallback if the blueprint cannot be loaded.
