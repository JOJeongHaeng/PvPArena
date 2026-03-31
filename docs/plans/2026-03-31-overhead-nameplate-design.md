# Overhead Nameplate Design

**Goal:** Show each character's nickname and health bar above their head, add lobby nickname entry with server-side default names, and show the current lobby participant list in the HUD.

## Scope

- Add a replicated display nickname to `APvPArenaPlayerState`.
- Let each local player edit their nickname in the lobby.
- Normalize empty input on the server and assign unique defaults such as `Player1`, `Player2`, and beyond, so the flow still works when the match expands up to 6 players later.
- Attach a world-space overhead widget to every `APvPArenaCharacter`.
- Show nickname and health in that widget for all characters, including self.
- Show a participant list in the lobby based on replicated player states.

## Architecture

`APvPArenaPlayerState` becomes the single replicated source of truth for the visible nickname. The server owns normalization and default-name generation so all clients see the same result. `APvPArenaPlayerController` exposes an RPC to submit nickname changes from the lobby HUD.

`APvPArenaCharacter` owns a `UWidgetComponent` positioned above the mesh. The component hosts a lightweight `UUserWidget` that reads the character's current health and player state's display nickname and refreshes when possession, replication, or health changes occur.

The lobby HUD adds a nickname entry field and a participant list. The field submits local edits through the player controller. The participant list renders from `GameState->PlayerArray`, so it scales from the current player count to a future 6-player lobby without changing the data flow.

## Data Flow

1. The local player types a nickname in the lobby HUD.
2. The HUD sends the raw string through the player controller to the server.
3. The server trims the string and writes either the requested nickname or the next unique default nickname into `APvPArenaPlayerState`.
4. Replication updates all clients.
5. The lobby participant list and every overhead widget rebuild their displayed nickname from the replicated player state.
6. Health changes continue to come from `APvPArenaCharacter::CurrentHealth`, and the overhead bar reads that value directly.

## Error Handling

- Blank or whitespace-only nickname input resolves to a server-generated unique default.
- Duplicate manual nicknames can be left as-is unless the existing codebase already enforces uniqueness; only default-generated names must be unique.
- If a character has no valid player state yet, the overhead widget shows a temporary fallback label and full health placeholder until replication finishes.

## Testing

- Add a player state test for display nickname defaults and normalization.
- Add a HUD test for the lobby participant list and nickname input defaults.
- Add an overhead widget test for resolved nickname and health percentage display state.
