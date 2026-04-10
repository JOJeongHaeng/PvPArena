# HUD BGM Soft Load Design

**Goal:** Reduce default memory usage by removing eager HUD background-music loading and loading menu/gameplay BGM only when the HUD actually needs to play it.

**Current Context**

- `UPvPArenaHUDWidget` hard-loads `Starter_Music_Cue` and `Starter_Background_Cue` in its constructor.
- These assets are non-combat and local-only, so first-use loading risk is much lower than attack montages or projectile effects.
- The HUD already centralizes BGM switching in `RefreshBackgroundMusic()`.

## Recommended Approach

Convert the two BGM properties from hard `USoundBase*` references to `TSoftObjectPtr<USoundBase>`. Keep the existing match-phase routing, but resolve the chosen asset with `LoadSynchronous()` only when playback is needed.

This removes default eager audio loading from the HUD CDO while preserving current BGM behavior once the user reaches lobby or gameplay.

## Code Changes

- Replace HUD background music properties with soft object pointers.
- Remove constructor-time `ConstructorHelpers::FObjectFinder` audio loads.
- Resolve the chosen BGM asset in `RefreshBackgroundMusic()` on demand.
- Keep `CurrentBackgroundMusic` as the loaded `USoundBase*` currently assigned to the audio component.

## Testing Strategy

- Update the HUD BGM automation test to assert the properties are soft references and keep the same asset paths.
- Verify match-phase path mapping remains unchanged.
- Build and run focused HUD BGM automation after the change.
