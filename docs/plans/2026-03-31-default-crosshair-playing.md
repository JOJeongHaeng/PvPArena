# Default Crosshair During Match Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Keep the crosshair visible throughout active match play while leaving charge camera behavior unchanged.

**Architecture:** `UPvPArenaHUDWidget` will decide crosshair visibility from match phase plus existing character context. Tests will cover lobby, playing, and match-end visibility states so the HUD policy is explicit and regression-proof.

**Tech Stack:** Unreal Engine 5 C++, automation tests

---

### Task 1: Update HUD crosshair regression test

**Files:**
- Modify: `Source/PvPArena/Private/Tests/HUDWidgetRangedCrosshairTest.cpp`

**Step 1: Write the failing test**

Extend the test so it expects:
- hidden with no match state
- visible during `EPvPAMatchPhase::Playing`
- hidden during `EPvPAMatchPhase::Lobby`
- hidden during `EPvPAMatchPhase::MatchEnd`

**Step 2: Run test to verify it fails**

Run: `Build.bat PvPArenaEditor Win64 Development -Project="C:\UE5CPP\PvPArena\PvPArena.uproject" -WaitMutex -NoHotReloadFromIDE`

Then run the HUD widget automation test in the editor automation flow and confirm the old implementation does not satisfy the new expectation.

**Step 3: Write minimal implementation**

Update `UPvPArenaHUDWidget::BuildRangedCrosshairVisibilityState` to return visible only while the owning match phase is `Playing`.

**Step 4: Run test to verify it passes**

Re-run the same build and automation test and confirm the HUD widget crosshair test passes.

**Step 5: Commit**

Commit only the HUD widget test, HUD widget implementation, and the new docs if requested.
