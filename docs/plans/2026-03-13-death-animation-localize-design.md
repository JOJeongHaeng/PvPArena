# Death Animation Localization Design

## Summary

Move the character death animation dependency away from the external mocap pack path and onto a project-owned asset under `/Game/PvPArena/Animations`. This keeps runtime behavior unchanged while making the asset dependency explicit and safer to maintain.

## Decision

- Keep the current `APvPArenaCharacter` death animation flow unchanged.
- Replace only the default asset reference path used in C++.
- Reuse the already duplicated asset:
  - `/Game/PvPArena/Animations/Stand_Relaxed_Death`
- Do not refactor attack animations, montage setup, or the broader imported pack structure in this pass.

## Recommended Approach

Use the smallest possible change:

1. copy the duplicated death animation asset into the feature worktree content
2. update the `ConstructorHelpers::FObjectFinder<UAnimationAsset>` path in `APvPArenaCharacter`
3. verify the existing death animation default regression test still passes
4. rebuild the editor target

This preserves the current character behavior and test surface while removing the direct dependency on `/Game/MCO_Mocap_Basics/...`.

## Scope

- project-owned death animation asset path
- current character death animation default
- focused regression verification

## Non-Goals

- no new animation montage work
- no melee/ranged animation hookup
- no cleanup of all third-party animation content
- no animation blueprint redesign

## Verification

- `PvPArena.Character.DeathAnimationDefaults`
- `PvPArenaEditor Win64 Development` build
