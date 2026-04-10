# HUD BGM Soft Load Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Stop eagerly loading HUD background music at widget construction time and instead load the lobby/gameplay BGM only when playback is requested.

**Architecture:** `UPvPArenaHUDWidget` already owns match-phase-based BGM selection, so this change stays localized to the widget and its test. We convert the asset properties to `TSoftObjectPtr`, preserve path-based routing, and lazily resolve the desired `USoundBase` inside `RefreshBackgroundMusic()`.

**Tech Stack:** Unreal Engine 5 C++, `TSoftObjectPtr`, `LoadSynchronous`, automation tests.

---

### Task 1: Lock in lazy-load expectations for HUD BGM

**Files:**
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Tests/HUDWidgetBackgroundMusicTest.cpp`

**Step 1: Write the failing test**

Assert that:

- HUD BGM properties are exposed as soft object references
- the default asset paths still point at the same menu and gameplay cues

**Step 2: Run test to verify it fails**

Run the focused HUD BGM automation test.

**Step 3: Write minimal implementation**

Convert the properties and loading path to soft references with on-demand resolution.

**Step 4: Run test to verify it passes**

Run the same HUD BGM automation again.

### Task 2: Convert HUD BGM loading to soft references

**Files:**
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Public/UI/PvPArenaHUDWidget.h`
- Modify: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/UI/PvPArenaHUDWidget.cpp`
- Verify through: `/mnt/c/UE5CPP/PvPArena/Source/PvPArena/Private/Tests/HUDWidgetBackgroundMusicTest.cpp`

**Step 1: Write the failing test**

Use the soft-reference assertions from Task 1.

**Step 2: Run test to verify it fails**

Run the focused HUD BGM automation test.

**Step 3: Write minimal implementation**

Remove eager constructor loads, store the asset paths as `TSoftObjectPtr<USoundBase>`, and lazily resolve the desired cue when playback is requested.

**Step 4: Run test to verify it passes**

Run the focused HUD BGM automation test again.

### Task 3: Verify build and focused automation

**Files:**
- Verify only

**Step 1: Run verification**

Run:

- project build
- `PvPArena.UI.HUDWidgetBackgroundMusic`

**Step 2: Summarize outcome**

Report:

- files changed
- which eager audio loads were removed
- any remaining soft-load candidates deferred from this pass
