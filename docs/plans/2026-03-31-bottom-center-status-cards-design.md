# Bottom Center Status Cards Design

## Goal

Move the current HP, sprint, and ranged cooldown HUD from the top-right corner to the bottom center of the screen, and present them as three independent horizontal cards.

## Current State

`UPvPArenaHUDWidget` currently builds a single `StatusPanel` anchored to the upper-right corner. Inside it, a `StatusBox` stacks six rows vertically:

- health bar
- health text
- sprint bar
- sprint text
- ranged cooldown bar
- ranged cooldown text

This keeps the data readable, but the layout competes with the match information and leaves the lower HUD area unused.

## Chosen Approach

Replace the single stacked status panel with a bottom-center card row:

- keep one outer `StatusPanel`
- anchor it to the bottom center of the root overlay
- replace the internal layout with a horizontal container
- create three child cards inside that row:
  - health card
  - sprint card
  - ranged cooldown card
- keep each card's internal structure vertical so the gauge stays above its label

This preserves all existing refresh logic and data sources. Only the widget tree structure and positioning change.

## Alternatives Considered

### 1. Re-anchor existing vertical stack

Move the current `StatusPanel` to the bottom center and keep the six-row vertical stack.

Why rejected:
- does not satisfy the requested horizontal layout
- creates an unnecessarily tall block near the crosshair area

### 2. Full HUD region redesign

Rebuild both the status panel and info panel into a full-width bottom HUD.

Why rejected:
- larger blast radius than needed
- higher regression risk for unrelated UI

## Layout Details

The new status area should:

- sit at the bottom center of the screen
- leave breathing room above the screen edge
- keep readable dark panel backgrounds
- keep current color coding for health, sprint, and ranged cooldown
- avoid overlapping the crosshair by remaining below it

The recommended structure is:

- `StatusPanel`
  - `StatusCardsBox` (`UHorizontalBox`)
    - `HealthCard`
      - `HealthCardBox` (`UVerticalBox`)
    - `SprintCard`
      - `SprintCardBox` (`UVerticalBox`)
    - `RangedCard`
      - `RangedCardBox` (`UVerticalBox`)

Each card will contain:

- one `USizeBox` + `UProgressBar`
- one `UTextBlock`

## Data Flow

No gameplay or HUD refresh behavior changes are needed.

- `RefreshWidgetData()` continues to populate the same progress bars and labels
- crosshair visibility and offset behavior remain unchanged
- player stats and combat cooldown reads remain unchanged

## Testing

Update HUD layout automation coverage to verify:

- the status panel is bottom-centered instead of top-right
- the status panel contains a horizontal card row
- the card row contains exactly three cards
- each card keeps its gauge and label children

Retain existing checks for:

- widget existence
- crosshair structure
- general readability sizing

## Risks

Primary risk is layout regression in the existing HUD automation tests because those tests currently assume the old top-right stacked layout.

There is low gameplay risk because the change is presentation-only.
