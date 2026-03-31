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
