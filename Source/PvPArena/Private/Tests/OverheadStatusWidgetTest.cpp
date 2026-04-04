#include "Misc/AutomationTest.h"
#include "Blueprint/WidgetTree.h"
#include "Components/WidgetComponent.h"
#include "Engine/World.h"
#include "Game/PvPArenaPlayerState.h"
#include "Player/PvPArenaCharacter.h"
#include "Tests/AutomationEditorCommon.h"
#include "UI/PvPArenaOverheadStatusWidget.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FOverheadStatusWidgetTest,
    "PvPArena.UI.OverheadStatusWidget",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FOverheadStatusWidgetTest::RunTest(const FString& Parameters)
{
    UPvPArenaOverheadStatusWidget* Widget = NewObject<UPvPArenaOverheadStatusWidget>();
    TestNotNull(TEXT("Overhead widget should be created"), Widget);

    if (!Widget)
    {
        return false;
    }

    Widget->TakeWidget();
    TestTrue(
        TEXT("Native overhead widget should not auto-build a layout tree when a blueprint child provides the visuals"),
        Widget->WidgetTree && Widget->WidgetTree->RootWidget == nullptr);

    UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
    TestNotNull(TEXT("Automation test should create an editor world"), TestWorld);

    APvPArenaCharacter* Character = TestWorld
        ? TestWorld->SpawnActor<APvPArenaCharacter>(APvPArenaCharacter::StaticClass())
        : nullptr;
    APvPArenaPlayerState* PlayerState = NewObject<APvPArenaPlayerState>();
    TestNotNull(TEXT("Character should be created"), Character);
    TestNotNull(TEXT("PlayerState should be created"), PlayerState);

    if (!Character || !PlayerState)
    {
        return false;
    }

    const UWidgetComponent* OverheadWidgetComponent = Character->GetOverheadStatusWidgetComponent();
    TestNotNull(TEXT("Character should expose an overhead widget component"), OverheadWidgetComponent);
    TestTrue(
        TEXT("Overhead widget should attach to the character mesh so it follows the head during movement and animation"),
        OverheadWidgetComponent && OverheadWidgetComponent->GetAttachParent() == Character->GetMesh());
    TestEqual(
        TEXT("Overhead widget should attach only to the explicit overheadsocket"),
        OverheadWidgetComponent ? OverheadWidgetComponent->GetAttachSocketName() : NAME_None,
        FName(TEXT("overheadsocket")));
    TestEqual(
        TEXT("Overhead widget should render in world space so listen servers can also see remote player widgets"),
        OverheadWidgetComponent ? OverheadWidgetComponent->GetWidgetSpace() : EWidgetSpace::World,
        EWidgetSpace::World);
    TestTrue(
        TEXT("Overhead widget should keep absolute rotation so character turning does not spin the widget"),
        OverheadWidgetComponent && OverheadWidgetComponent->IsUsingAbsoluteRotation());
    TestEqual(
        TEXT("Overhead widget should not impose a code-side relative location offset"),
        OverheadWidgetComponent ? OverheadWidgetComponent->GetRelativeLocation() : FVector::ZeroVector,
        FVector::ZeroVector);
    TestFalse(
        TEXT("Overhead widget should leave draw-at-desired-size disabled state to blueprint/editor tuning"),
        OverheadWidgetComponent && OverheadWidgetComponent->GetDrawAtDesiredSize());
    if (OverheadWidgetComponent)
    {
        const_cast<UWidgetComponent*>(OverheadWidgetComponent)->InitWidget();
    }
    TestNotNull(
        TEXT("Overhead widget component should be able to instantiate its runtime widget"),
        OverheadWidgetComponent ? OverheadWidgetComponent->GetUserWidgetObject() : nullptr);
    TestTrue(
        TEXT("Overhead widget runtime instance should derive from PvPArenaOverheadStatusWidget"),
        OverheadWidgetComponent
            && OverheadWidgetComponent->GetUserWidgetObject()
            && OverheadWidgetComponent->GetUserWidgetObject()->IsA<UPvPArenaOverheadStatusWidget>());

    PlayerState->SetDisplayNickname(TEXT("Player2"));
    Character->SetPlayerState(PlayerState);
    Character->ApplyServerDamage(50.0f, nullptr);

    FString NicknameLabel;
    float HealthPercent = 0.0f;
    UPvPArenaOverheadStatusWidget::BuildDisplayState(Character, NicknameLabel, HealthPercent);

    TestEqual(TEXT("Nickname should come from player state"), NicknameLabel, FString(TEXT("Player2")));
    TestEqual(TEXT("Health percent should match current health"), HealthPercent, 0.5f);

    PlayerState->SetDisplayNickname(TEXT(""));
    PlayerState->SetPlayerId(2);
    UPvPArenaOverheadStatusWidget::BuildDisplayState(Character, NicknameLabel, HealthPercent);
    TestEqual(
        TEXT("Empty replicated nicknames should fall back to a stable player-id label instead of a generic placeholder"),
        NicknameLabel,
        FString(TEXT("Player2")));

    APvPArenaPlayerState* LocalPlayerState = NewObject<APvPArenaPlayerState>();
    TestNotNull(TEXT("Local player state should be created"), LocalPlayerState);
    if (!LocalPlayerState)
    {
        return false;
    }

    LocalPlayerState->SetDisplayNickname(TEXT("LocalHero"));
    LocalPlayerState->SetLobbyMatchMode(EPvPALobbyMatchMode::FreeForAll);
    PlayerState->SetLobbyMatchMode(EPvPALobbyMatchMode::FreeForAll);
    TestEqual(
        TEXT("Free-for-all should classify every other player as an enemy"),
        UPvPArenaOverheadStatusWidget::ResolveRelationship(PlayerState, LocalPlayerState),
        EPvPArenaOverheadRelationship::Enemy);
    TestEqual(
        TEXT("The same player state should classify as self"),
        UPvPArenaOverheadStatusWidget::ResolveRelationship(LocalPlayerState, LocalPlayerState),
        EPvPArenaOverheadRelationship::Self);

    LocalPlayerState->SetLobbyMatchMode(EPvPALobbyMatchMode::TeamVersus);
    LocalPlayerState->SetLobbyTeam(EPvPALobbyTeam::Left);
    PlayerState->SetLobbyMatchMode(EPvPALobbyMatchMode::TeamVersus);
    PlayerState->SetLobbyTeam(EPvPALobbyTeam::Left);
    TestEqual(
        TEXT("Team-versus should classify matching non-empty teams as allies"),
        UPvPArenaOverheadStatusWidget::ResolveRelationship(PlayerState, LocalPlayerState),
        EPvPArenaOverheadRelationship::Ally);

    PlayerState->SetLobbyTeam(EPvPALobbyTeam::Right);
    TestEqual(
        TEXT("Team-versus should classify the opposing team as enemies"),
        UPvPArenaOverheadStatusWidget::ResolveRelationship(PlayerState, LocalPlayerState),
        EPvPArenaOverheadRelationship::Enemy);
    TestEqual(
        TEXT("Self badge should stay hidden to reduce clutter"),
        UPvPArenaOverheadStatusWidget::BuildRelationshipBadge(EPvPArenaOverheadRelationship::Self),
        FString());
    TestEqual(
        TEXT("Ally badge should be explicit"),
        UPvPArenaOverheadStatusWidget::BuildRelationshipBadge(EPvPArenaOverheadRelationship::Ally),
        FString(TEXT("ALLY")));
    TestEqual(
        TEXT("Enemy badge should be explicit"),
        UPvPArenaOverheadStatusWidget::BuildRelationshipBadge(EPvPArenaOverheadRelationship::Enemy),
        FString(TEXT("ENEMY")));

    const FLinearColor EnemyColor = UPvPArenaOverheadStatusWidget::BuildRelationshipColor(EPvPArenaOverheadRelationship::Enemy, false);
    const FLinearColor HighlightEnemyColor = UPvPArenaOverheadStatusWidget::BuildRelationshipColor(EPvPArenaOverheadRelationship::Enemy, true);
    TestTrue(
        TEXT("Highlighted enemies should brighten the overhead nickname color"),
        HighlightEnemyColor.R >= EnemyColor.R
            && HighlightEnemyColor.G >= EnemyColor.G
            && HighlightEnemyColor.B >= EnemyColor.B);
    const FLinearColor SelfHealthColor = UPvPArenaOverheadStatusWidget::BuildHealthBarFillColor(EPvPArenaOverheadRelationship::Self, false);
    const FLinearColor AllyHealthColor = UPvPArenaOverheadStatusWidget::BuildHealthBarFillColor(EPvPArenaOverheadRelationship::Ally, false);
    const FLinearColor EnemyHealthColor = UPvPArenaOverheadStatusWidget::BuildHealthBarFillColor(EPvPArenaOverheadRelationship::Enemy, false);
    const FLinearColor HighlightEnemyHealthColor = UPvPArenaOverheadStatusWidget::BuildHealthBarFillColor(EPvPArenaOverheadRelationship::Enemy, true);
    TestTrue(
        TEXT("Self health bar should stay in a darker saturated green-blue range"),
        SelfHealthColor.G >= 0.55f && SelfHealthColor.B >= 0.45f && SelfHealthColor.R <= 0.20f);
    TestTrue(
        TEXT("Ally health bar should stay in a darker saturated blue range"),
        AllyHealthColor.B >= 0.75f && AllyHealthColor.R <= 0.15f && AllyHealthColor.G <= 0.45f);
    TestTrue(
        TEXT("Enemy health bar should stay in a darker saturated red range"),
        EnemyHealthColor.R >= 0.75f && EnemyHealthColor.G <= 0.22f && EnemyHealthColor.B <= 0.20f);
    TestTrue(
        TEXT("Highlighted enemy health bars should stay vivid instead of washing out"),
        HighlightEnemyHealthColor.R >= EnemyHealthColor.R
            && HighlightEnemyHealthColor.G <= 0.30f
            && HighlightEnemyHealthColor.B <= 0.28f);

    TestTrue(
        TEXT("Overhead widget should stay visible when a local view controller exists and the target is unobstructed in front of the camera"),
        APvPArenaCharacter::BuildOverheadWidgetVisibleState(true, false, true, false, false));
    TestFalse(
        TEXT("Overhead widget should hide when a blocking obstacle occludes the target from the local camera"),
        APvPArenaCharacter::BuildOverheadWidgetVisibleState(true, false, true, true, false));
    TestTrue(
        TEXT("Overhead widget should remain visible for the owning character even when the trace hits the character body"),
        APvPArenaCharacter::BuildOverheadWidgetVisibleState(true, true, true, true, true));

    return true;
}
