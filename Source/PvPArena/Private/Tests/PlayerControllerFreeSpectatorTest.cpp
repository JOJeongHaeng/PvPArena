#include "Misc/AutomationTest.h"
#include "Game/PvPArenaPlayerController.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPlayerControllerFreeSpectatorTest,
    "PvPArena.PlayerController.FreeSpectator",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPlayerControllerFreeSpectatorTest::RunTest(const FString& Parameters)
{
    APvPArenaPlayerController* PlayerController = NewObject<APvPArenaPlayerController>();
    TestNotNull(TEXT("PlayerController should be created"), PlayerController);

    if (!PlayerController)
    {
        return false;
    }

    PlayerController->SetIgnoreMoveInput(true);
    PlayerController->SetIgnoreLookInput(true);

    TestTrue(TEXT("Move input should start ignored for the test"), PlayerController->IsMoveInputIgnored());
    TestTrue(TEXT("Look input should start ignored for the test"), PlayerController->IsLookInputIgnored());

    PlayerController->ClientEnterFreeSpectatorMode_Implementation();

    TestFalse(TEXT("Free spectator mode should restore move input"), PlayerController->IsMoveInputIgnored());
    TestFalse(TEXT("Free spectator mode should restore look input"), PlayerController->IsLookInputIgnored());
    TestEqual(
        TEXT("Free spectator move speed should be tuned down for easier control"),
        static_cast<double>(APvPArenaPlayerController::FreeSpectatorMoveSpeed),
        1200.0);
    TestEqual(
        TEXT("Free spectator look sensitivity should be slightly increased"),
        static_cast<double>(APvPArenaPlayerController::FreeSpectatorLookSensitivity),
        1.25);

    const FRotator RotatedRight = APvPArenaPlayerController::BuildFreeSpectatorControlRotation(FRotator::ZeroRotator, 15.0f, 0.0f);
    TestEqual(TEXT("Mouse X should rotate yaw to the right"), static_cast<double>(RotatedRight.Yaw), 18.75);

    const FRotator RotatedUp = APvPArenaPlayerController::BuildFreeSpectatorControlRotation(FRotator::ZeroRotator, 0.0f, -20.0f);
    TestEqual(TEXT("Mouse up should decrease pitch"), static_cast<double>(RotatedUp.Pitch), -25.0);

    const FRotator ClampedPitch = APvPArenaPlayerController::BuildFreeSpectatorControlRotation(FRotator(-80.0f, 0.0f, 0.0f), 0.0f, -20.0f);
    TestEqual(TEXT("Free spectator pitch should clamp to avoid flipping"), static_cast<double>(ClampedPitch.Pitch), -89.0);

    PlayerController->SetIgnoreMoveInput(true);
    PlayerController->SetIgnoreLookInput(true);
    PlayerController->PrepareForRoundRestart();

    TestFalse(TEXT("Round restart preparation should restore move input"), PlayerController->IsMoveInputIgnored());
    TestFalse(TEXT("Round restart preparation should restore look input"), PlayerController->IsLookInputIgnored());

    PlayerController->SetIgnoreMoveInput(true);
    PlayerController->SetIgnoreLookInput(true);
    PlayerController->ClientPrepareForRoundRestart_Implementation();

    TestFalse(TEXT("Client round restart preparation should restore move input"), PlayerController->IsMoveInputIgnored());
    TestFalse(TEXT("Client round restart preparation should restore look input"), PlayerController->IsLookInputIgnored());
    return true;
}
