#include "UI/PvPArenaHUDWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/AudioComponent.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Components/EditableTextBox.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "GameFramework/GameUserSettings.h"
#include "Engine/World.h"
#include "Game/PvPArenaGameState.h"
#include "Game/PvPArenaPlayerController.h"
#include "Game/PvPArenaPlayerState.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/PlayerState.h"
#include "Combat/PvPCombatComponent.h"
#include "Player/PvPArenaCharacter.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"
#include "Widgets/SWidget.h"

namespace
{
const TCHAR* NonCombatBackgroundMusicPath = TEXT("/Game/PvPArena/Audio/Starter_Music_Cue.Starter_Music_Cue");
const TCHAR* GameplayBackgroundMusicPath = TEXT("/Game/PvPArena/Audio/Starter_Background_Cue.Starter_Background_Cue");
const TCHAR* HostTravelMapPath = TEXT("/Game/PvPArena/Maps/PvPArena_TestMap?listen");
const TCHAR* DefaultJoinAddressHint = TEXT("123.45.67.89:7777");
const FIntPoint DefaultWindowedResolution(1280, 720);
constexpr float DefaultRangedCrosshairVerticalOffset = -24.0f;
}

UPvPArenaHUDWidget::UPvPArenaHUDWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    static ConstructorHelpers::FObjectFinder<USoundBase> NonCombatMusicFinder(NonCombatBackgroundMusicPath);
    if (NonCombatMusicFinder.Succeeded())
    {
        NonCombatBackgroundMusic = NonCombatMusicFinder.Object;
    }

    static ConstructorHelpers::FObjectFinder<USoundBase> GameplayMusicFinder(GameplayBackgroundMusicPath);
    if (GameplayMusicFinder.Succeeded())
    {
        GameplayBackgroundMusic = GameplayMusicFinder.Object;
    }
}

TSharedRef<SWidget> UPvPArenaHUDWidget::RebuildWidget()
{
    BuildWidgetTree();
    return Super::RebuildWidget();
}

void UPvPArenaHUDWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (!BackgroundMusicAudioComponent && GetWorld())
    {
        BackgroundMusicAudioComponent = NewObject<UAudioComponent>(this, TEXT("BackgroundMusicAudioComponent"));
        if (BackgroundMusicAudioComponent)
        {
            BackgroundMusicAudioComponent->bAutoActivate = false;
            BackgroundMusicAudioComponent->bIsUISound = true;
            BackgroundMusicAudioComponent->RegisterComponentWithWorld(GetWorld());
        }
    }

    BuildWidgetTree();
    if (UGameUserSettings* GameUserSettings = UGameUserSettings::GetGameUserSettings())
    {
        const TArray<FIntPoint> SupportedResolutions = BuildSupportedResolutions();
        const FIntPoint CurrentResolution = GameUserSettings->GetScreenResolution();
        const int32 ResolutionIndex = SupportedResolutions.IndexOfByKey(CurrentResolution);
        SelectedResolutionIndex = ResolutionIndex != INDEX_NONE ? ResolutionIndex : 0;
        SelectedWindowModeIndex = static_cast<int32>(GameUserSettings->GetFullscreenMode());
        bVSyncEnabled = GameUserSettings->IsVSyncEnabled();
    }

    MasterVolume = 1.0f;
    BackgroundMusicVolume = 1.0f;
    SfxVolume = 1.0f;
    RefreshSettingsMenuState();
    ApplyAudioSettings();
    RefreshWidgetData();
    RefreshCrosshairVisibility();
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(RefreshTimerHandle, this, &UPvPArenaHUDWidget::RefreshWidgetData, 0.15f, true);
    }
}

void UPvPArenaHUDWidget::NativeDestruct()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(RefreshTimerHandle);
    }

    if (BackgroundMusicAudioComponent)
    {
        BackgroundMusicAudioComponent->Stop();
        BackgroundMusicAudioComponent->UnregisterComponent();
        BackgroundMusicAudioComponent = nullptr;
    }

    CurrentBackgroundMusic = nullptr;

    Super::NativeDestruct();
}

void UPvPArenaHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    RefreshCrosshairVisibility();
}

void UPvPArenaHUDWidget::BuildWidgetTree()
{
    if (!WidgetTree)
    {
        return;
    }

    if (RootOverlay)
    {
        return;
    }

    RootOverlay = Cast<UOverlay>(WidgetTree->RootWidget);
    if (!RootOverlay)
    {
        RootOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("RootOverlay"));
        WidgetTree->RootWidget = RootOverlay;
    }

    StatusPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("StatusPanel"));
    CountdownPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("CountdownPanel"));
    InfoPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("InfoPanel"));
    AnnouncementPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("AnnouncementPanel"));
    LobbyPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("LobbyPanel"));
    LobbyControlsPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("LobbyControlsPanel"));
    MatchResultPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("MatchResultPanel"));
    SettingsPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("SettingsPanel"));
    SpectatorHelpPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("SpectatorHelpPanel"));
    StatusBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("StatusBox"));
    StatusCardsBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("StatusCardsBox"));
    CountdownBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("CountdownBox"));
    InfoBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("InfoBox"));
    AnnouncementBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("AnnouncementBox"));
    LobbyBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("LobbyBox"));
    LobbyControlsBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("LobbyControlsBox"));
    LobbyControlsCardsBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("LobbyControlsCardsBox"));
    MatchResultBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("MatchResultBox"));
    SettingsBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("SettingsBox"));
    SpectatorHelpBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("SpectatorHelpBox"));
    LobbyKeyboardCard = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("LobbyKeyboardCard"));
    LobbyMouseCard = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("LobbyMouseCard"));
    HealthCard = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("HealthCard"));
    SprintCard = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("SprintCard"));
    RangedCard = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("RangedCard"));
    LobbyKeyboardCardBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("LobbyKeyboardCardBox"));
    LobbyMouseCardBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("LobbyMouseCardBox"));
    HealthCardBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("HealthCardBox"));
    SprintCardBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("SprintCardBox"));
    RangedCardBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("RangedCardBox"));
    HealthBarSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("HealthBarSizeBox"));
    HealthBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("HealthBar"));
    SprintBarSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("SprintBarSizeBox"));
    SprintBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("SprintBar"));
    RangedCooldownBarSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("RangedCooldownBarSizeBox"));
    RangedCooldownBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("RangedCooldownBar"));
    HealthText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("HealthText"));
    SprintText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SprintText"));
    RangedCooldownText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RangedCooldownText"));
    MatchScoreText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("MatchScoreText"));
    TimerText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TimerText"));
    RoundStateText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RoundStateText"));
    ResultText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ResultText"));
    NextRoundText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("NextRoundText"));
    LobbyTitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LobbyTitleText"));
    LobbyStatusText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LobbyStatusText"));
    LobbyModeStatusText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LobbyModeStatusText"));
    LobbyTeamStatusText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LobbyTeamStatusText"));
    LobbyLeftTeamListText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LobbyLeftTeamListText"));
    LobbyRightTeamListText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LobbyRightTeamListText"));
    LobbyReadyButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("LobbyReadyButton"));
    LobbyReadyButtonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LobbyReadyButtonText"));
    LobbyFreeForAllModeButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("LobbyFreeForAllModeButton"));
    LobbyFreeForAllModeButtonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LobbyFreeForAllModeButtonText"));
    LobbyTeamVersusModeButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("LobbyTeamVersusModeButton"));
    LobbyTeamVersusModeButtonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LobbyTeamVersusModeButtonText"));
    LobbyLeftTeamButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("LobbyLeftTeamButton"));
    LobbyLeftTeamButtonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LobbyLeftTeamButtonText"));
    LobbyRightTeamButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("LobbyRightTeamButton"));
    LobbyRightTeamButtonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LobbyRightTeamButtonText"));
    LobbyControlsTitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LobbyControlsTitleText"));
    LobbyControlsKeyboardTitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LobbyControlsKeyboardTitleText"));
    LobbyControlsKeyboardMoveText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LobbyControlsKeyboardMoveText"));
    LobbyControlsKeyboardSprintText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LobbyControlsKeyboardSprintText"));
    LobbyControlsMouseTitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LobbyControlsMouseTitleText"));
    LobbyControlsMouseMeleeText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LobbyControlsMouseMeleeText"));
    LobbyControlsMouseRangedText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LobbyControlsMouseRangedText"));
    MatchResultTitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("MatchResultTitleText"));
    MatchResultSummaryText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("MatchResultSummaryText"));
    ConnectionStatusText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ConnectionStatusText"));
    LobbyNicknameTextBox = WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), TEXT("LobbyNicknameTextBox"));
    LobbyPlayerListBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("LobbyPlayerListBox"));
    SettingsTitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SettingsTitleText"));
    SettingsDisplayModeLabelText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SettingsDisplayModeLabelText"));
    SettingsResolutionLabelText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SettingsResolutionLabelText"));
    SettingsAudioLabelText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SettingsAudioLabelText"));
    SettingsMasterVolumeText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SettingsMasterVolumeText"));
    SettingsBgmVolumeText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SettingsBgmVolumeText"));
    SettingsSfxVolumeText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SettingsSfxVolumeText"));
    SettingsWindowModeButtonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SettingsWindowModeButtonText"));
    SettingsResolutionButtonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SettingsResolutionButtonText"));
    SettingsVSyncButtonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SettingsVSyncButtonText"));
    SettingsResumeButtonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SettingsResumeButtonText"));
    SettingsQuitButtonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SettingsQuitButtonText"));
    SpectatorHelpTitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SpectatorHelpTitleText"));
    SpectatorHelpMoveText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SpectatorHelpMoveText"));
    SpectatorHelpLookText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SpectatorHelpLookText"));
    SpectatorHelpRiseText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SpectatorHelpRiseText"));
    JoinAddressTextBox = WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), TEXT("JoinAddressTextBox"));
    HostMatchButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("HostMatchButton"));
    HostMatchButtonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("HostMatchButtonText"));
    JoinByIpButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("JoinByIpButton"));
    JoinByIpButtonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("JoinByIpButtonText"));
    SettingsMasterVolumeSlider = WidgetTree->ConstructWidget<USlider>(USlider::StaticClass(), TEXT("SettingsMasterVolumeSlider"));
    SettingsBgmVolumeSlider = WidgetTree->ConstructWidget<USlider>(USlider::StaticClass(), TEXT("SettingsBgmVolumeSlider"));
    SettingsSfxVolumeSlider = WidgetTree->ConstructWidget<USlider>(USlider::StaticClass(), TEXT("SettingsSfxVolumeSlider"));
    SettingsWindowModeButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("SettingsWindowModeButton"));
    SettingsResolutionButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("SettingsResolutionButton"));
    SettingsVSyncButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("SettingsVSyncButton"));
    SettingsResumeButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("SettingsResumeButton"));
    SettingsQuitButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("SettingsQuitButton"));
    RangedCrosshairOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("RangedCrosshairOverlay"));
    RangedCrosshairHorizontalBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("RangedCrosshairHorizontalBox"));
    RangedCrosshairVerticalBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("RangedCrosshairVerticalBox"));
    RangedCrosshairHorizontalLine = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("RangedCrosshairHorizontalLine"));
    RangedCrosshairVerticalLine = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("RangedCrosshairVerticalLine"));

    if (!RootOverlay || !StatusPanel || !CountdownPanel || !InfoPanel || !AnnouncementPanel || !LobbyPanel || !LobbyControlsPanel || !MatchResultPanel || !SettingsPanel || !SpectatorHelpPanel || !StatusCardsBox || !CountdownBox || !InfoBox || !AnnouncementBox || !LobbyBox || !LobbyControlsBox || !LobbyControlsCardsBox || !MatchResultBox || !SettingsBox || !SpectatorHelpBox || !LobbyKeyboardCard || !LobbyMouseCard || !HealthCard || !SprintCard || !RangedCard || !LobbyKeyboardCardBox || !LobbyMouseCardBox || !HealthCardBox || !SprintCardBox || !RangedCardBox || !HealthBarSizeBox || !HealthBar || !SprintBarSizeBox || !SprintBar || !RangedCooldownBarSizeBox || !RangedCooldownBar || !HealthText || !SprintText || !RangedCooldownText || !MatchScoreText || !TimerText || !RoundStateText || !ResultText || !NextRoundText || !LobbyTitleText || !LobbyStatusText || !LobbyModeStatusText || !LobbyTeamStatusText || !LobbyLeftTeamListText || !LobbyRightTeamListText || !LobbyReadyButton || !LobbyReadyButtonText || !LobbyFreeForAllModeButton || !LobbyFreeForAllModeButtonText || !LobbyTeamVersusModeButton || !LobbyTeamVersusModeButtonText || !LobbyLeftTeamButton || !LobbyLeftTeamButtonText || !LobbyRightTeamButton || !LobbyRightTeamButtonText || !LobbyControlsTitleText || !LobbyControlsKeyboardTitleText || !LobbyControlsKeyboardMoveText || !LobbyControlsKeyboardSprintText || !LobbyControlsMouseTitleText || !LobbyControlsMouseMeleeText || !LobbyControlsMouseRangedText || !MatchResultTitleText || !MatchResultSummaryText || !ConnectionStatusText || !LobbyNicknameTextBox || !LobbyPlayerListBox || !SettingsTitleText || !SettingsDisplayModeLabelText || !SettingsResolutionLabelText || !SettingsAudioLabelText || !SettingsMasterVolumeText || !SettingsBgmVolumeText || !SettingsSfxVolumeText || !SettingsWindowModeButtonText || !SettingsResolutionButtonText || !SettingsVSyncButtonText || !SettingsResumeButtonText || !SettingsQuitButtonText || !SpectatorHelpTitleText || !SpectatorHelpMoveText || !SpectatorHelpLookText || !SpectatorHelpRiseText || !JoinAddressTextBox || !HostMatchButton || !HostMatchButtonText || !JoinByIpButton || !JoinByIpButtonText || !SettingsMasterVolumeSlider || !SettingsBgmVolumeSlider || !SettingsSfxVolumeSlider || !SettingsWindowModeButton || !SettingsResolutionButton || !SettingsVSyncButton || !SettingsResumeButton || !SettingsQuitButton || !RangedCrosshairOverlay || !RangedCrosshairHorizontalBox || !RangedCrosshairVerticalBox || !RangedCrosshairHorizontalLine || !RangedCrosshairVerticalLine)
    {
        return;
    }

    StatusPanel->SetContent(StatusCardsBox);
    StatusPanel->SetPadding(FMargin(20.0f, 14.0f, 20.0f, 14.0f));
    StatusPanel->SetBrushColor(FLinearColor(0.03f, 0.05f, 0.08f, 0.78f));

    CountdownPanel->SetContent(CountdownBox);
    CountdownPanel->SetPadding(FMargin(20.0f, 10.0f, 20.0f, 10.0f));
    CountdownPanel->SetBrushColor(FLinearColor(0.03f, 0.05f, 0.08f, 0.78f));

    InfoPanel->SetContent(InfoBox);
    InfoPanel->SetPadding(FMargin(18.0f, 16.0f, 18.0f, 16.0f));
    InfoPanel->SetBrushColor(FLinearColor(0.03f, 0.05f, 0.08f, 0.78f));

    AnnouncementPanel->SetContent(AnnouncementBox);
    AnnouncementPanel->SetPadding(FMargin(28.0f, 20.0f, 28.0f, 20.0f));
    AnnouncementPanel->SetBrushColor(FLinearColor(0.02f, 0.03f, 0.06f, 0.82f));
    AnnouncementPanel->SetVisibility(ESlateVisibility::Collapsed);

    LobbyPanel->SetContent(LobbyBox);
    LobbyPanel->SetPadding(FMargin(34.0f, 26.0f, 34.0f, 26.0f));
    LobbyPanel->SetBrushColor(FLinearColor(0.02f, 0.05f, 0.08f, 0.84f));
    LobbyPanel->SetVisibility(ESlateVisibility::Collapsed);

    LobbyControlsPanel->SetContent(LobbyControlsBox);
    LobbyControlsPanel->SetPadding(FMargin(28.0f, 20.0f, 28.0f, 20.0f));
    LobbyControlsPanel->SetBrushColor(FLinearColor(0.02f, 0.05f, 0.08f, 0.76f));
    LobbyControlsPanel->SetVisibility(ESlateVisibility::Collapsed);

    MatchResultPanel->SetContent(MatchResultBox);
    MatchResultPanel->SetPadding(FMargin(34.0f, 26.0f, 34.0f, 26.0f));
    MatchResultPanel->SetBrushColor(FLinearColor(0.08f, 0.04f, 0.02f, 0.86f));
    MatchResultPanel->SetVisibility(ESlateVisibility::Collapsed);

    SettingsPanel->SetContent(SettingsBox);
    SettingsPanel->SetPadding(FMargin(30.0f, 24.0f, 30.0f, 24.0f));
    SettingsPanel->SetBrushColor(FLinearColor(0.01f, 0.03f, 0.06f, 0.92f));
    SettingsPanel->SetVisibility(ESlateVisibility::Collapsed);

    SpectatorHelpPanel->SetContent(SpectatorHelpBox);
    SpectatorHelpPanel->SetPadding(FMargin(18.0f, 14.0f, 18.0f, 14.0f));
    SpectatorHelpPanel->SetBrushColor(FLinearColor(0.02f, 0.05f, 0.08f, 0.82f));
    SpectatorHelpPanel->SetVisibility(ESlateVisibility::Collapsed);

    UOverlaySlot* StatusBoxSlot = RootOverlay->AddChildToOverlay(StatusPanel);
    UOverlaySlot* CountdownBoxSlot = RootOverlay->AddChildToOverlay(CountdownPanel);
    UOverlaySlot* InfoBoxSlot = RootOverlay->AddChildToOverlay(InfoPanel);
    UOverlaySlot* AnnouncementBoxSlot = RootOverlay->AddChildToOverlay(AnnouncementPanel);
    UOverlaySlot* LobbyBoxSlot = RootOverlay->AddChildToOverlay(LobbyPanel);
    UOverlaySlot* LobbyControlsBoxSlot = RootOverlay->AddChildToOverlay(LobbyControlsPanel);
    UOverlaySlot* MatchResultBoxSlot = RootOverlay->AddChildToOverlay(MatchResultPanel);
    UOverlaySlot* SettingsBoxSlot = RootOverlay->AddChildToOverlay(SettingsPanel);
    UOverlaySlot* SpectatorHelpBoxSlot = RootOverlay->AddChildToOverlay(SpectatorHelpPanel);
    UOverlaySlot* RangedCrosshairSlot = RootOverlay->AddChildToOverlay(RangedCrosshairOverlay);

    if (StatusBoxSlot)
    {
        StatusBoxSlot->SetHorizontalAlignment(HAlign_Center);
        StatusBoxSlot->SetVerticalAlignment(VAlign_Bottom);
        StatusBoxSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 44.0f));
    }

    if (CountdownBoxSlot)
    {
        CountdownBoxSlot->SetHorizontalAlignment(HAlign_Center);
        CountdownBoxSlot->SetVerticalAlignment(VAlign_Top);
        CountdownBoxSlot->SetPadding(FMargin(0.0f, 28.0f, 0.0f, 0.0f));
    }

    if (InfoBoxSlot)
    {
        InfoBoxSlot->SetHorizontalAlignment(HAlign_Left);
        InfoBoxSlot->SetVerticalAlignment(VAlign_Top);
        InfoBoxSlot->SetPadding(FMargin(40.0f, 40.0f, 0.0f, 0.0f));
    }

    if (AnnouncementBoxSlot)
    {
        AnnouncementBoxSlot->SetHorizontalAlignment(HAlign_Center);
        AnnouncementBoxSlot->SetVerticalAlignment(VAlign_Center);
        AnnouncementBoxSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 80.0f));
    }

    if (LobbyBoxSlot)
    {
        LobbyBoxSlot->SetHorizontalAlignment(HAlign_Center);
        LobbyBoxSlot->SetVerticalAlignment(VAlign_Center);
        LobbyBoxSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 180.0f));
    }

    if (LobbyControlsBoxSlot)
    {
        LobbyControlsBoxSlot->SetHorizontalAlignment(HAlign_Center);
        LobbyControlsBoxSlot->SetVerticalAlignment(VAlign_Bottom);
        LobbyControlsBoxSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 36.0f));
    }

    if (MatchResultBoxSlot)
    {
        MatchResultBoxSlot->SetHorizontalAlignment(HAlign_Center);
        MatchResultBoxSlot->SetVerticalAlignment(VAlign_Center);
    }

    if (SettingsBoxSlot)
    {
        SettingsBoxSlot->SetHorizontalAlignment(HAlign_Center);
        SettingsBoxSlot->SetVerticalAlignment(VAlign_Center);
    }

    if (SpectatorHelpBoxSlot)
    {
        SpectatorHelpBoxSlot->SetHorizontalAlignment(HAlign_Right);
        SpectatorHelpBoxSlot->SetVerticalAlignment(VAlign_Bottom);
        SpectatorHelpBoxSlot->SetPadding(FMargin(0.0f, 0.0f, 24.0f, 120.0f));
    }

    if (RangedCrosshairSlot)
    {
        RangedCrosshairSlot->SetHorizontalAlignment(HAlign_Center);
        RangedCrosshairSlot->SetVerticalAlignment(VAlign_Center);
    }

    RangedCrosshairHorizontalBox->SetWidthOverride(18.0f);
    RangedCrosshairHorizontalBox->SetHeightOverride(2.0f);
    RangedCrosshairHorizontalBox->SetContent(RangedCrosshairHorizontalLine);
    RangedCrosshairVerticalBox->SetWidthOverride(2.0f);
    RangedCrosshairVerticalBox->SetHeightOverride(18.0f);
    RangedCrosshairVerticalBox->SetContent(RangedCrosshairVerticalLine);

    HealthCard->SetContent(HealthCardBox);
    HealthCard->SetPadding(FMargin(16.0f, 12.0f, 16.0f, 12.0f));
    HealthCard->SetBrushColor(FLinearColor(0.03f, 0.05f, 0.08f, 0.84f));
    SprintCard->SetContent(SprintCardBox);
    SprintCard->SetPadding(FMargin(16.0f, 12.0f, 16.0f, 12.0f));
    SprintCard->SetBrushColor(FLinearColor(0.03f, 0.05f, 0.08f, 0.84f));
    RangedCard->SetContent(RangedCardBox);
    RangedCard->SetPadding(FMargin(16.0f, 12.0f, 16.0f, 12.0f));
    RangedCard->SetBrushColor(FLinearColor(0.03f, 0.05f, 0.08f, 0.84f));

    UOverlaySlot* HorizontalCrosshairSlot = RangedCrosshairOverlay->AddChildToOverlay(RangedCrosshairHorizontalBox);
    UOverlaySlot* VerticalCrosshairSlot = RangedCrosshairOverlay->AddChildToOverlay(RangedCrosshairVerticalBox);
    UHorizontalBoxSlot* HealthCardSlot = StatusCardsBox->AddChildToHorizontalBox(HealthCard);
    UHorizontalBoxSlot* SprintCardSlot = StatusCardsBox->AddChildToHorizontalBox(SprintCard);
    UHorizontalBoxSlot* RangedCardSlot = StatusCardsBox->AddChildToHorizontalBox(RangedCard);
    if (HorizontalCrosshairSlot)
    {
        HorizontalCrosshairSlot->SetHorizontalAlignment(HAlign_Center);
        HorizontalCrosshairSlot->SetVerticalAlignment(VAlign_Center);
        HorizontalCrosshairSlot->SetPadding(FMargin(-9.0f, 0.0f, -9.0f, 0.0f));
    }

    if (VerticalCrosshairSlot)
    {
        VerticalCrosshairSlot->SetHorizontalAlignment(HAlign_Center);
        VerticalCrosshairSlot->SetVerticalAlignment(VAlign_Center);
        VerticalCrosshairSlot->SetPadding(FMargin(0.0f, -9.0f, 0.0f, -9.0f));
    }

    if (HealthCardSlot)
    {
        HealthCardSlot->SetPadding(FMargin(0.0f, 0.0f, 10.0f, 0.0f));
    }

    if (SprintCardSlot)
    {
        SprintCardSlot->SetPadding(FMargin(10.0f, 0.0f, 10.0f, 0.0f));
    }

    if (RangedCardSlot)
    {
        RangedCardSlot->SetPadding(FMargin(10.0f, 0.0f, 0.0f, 0.0f));
    }

    HealthBarSizeBox->SetWidthOverride(280.0f);
    HealthBarSizeBox->SetHeightOverride(22.0f);
    HealthBarSizeBox->SetContent(HealthBar);

    SprintBarSizeBox->SetWidthOverride(280.0f);
    SprintBarSizeBox->SetHeightOverride(16.0f);
    SprintBarSizeBox->SetContent(SprintBar);

    RangedCooldownBarSizeBox->SetWidthOverride(280.0f);
    RangedCooldownBarSizeBox->SetHeightOverride(16.0f);
    RangedCooldownBarSizeBox->SetContent(RangedCooldownBar);

    HealthBar->SetPercent(1.0f);
    HealthBar->SetFillColorAndOpacity(FLinearColor(0.9f, 0.2f, 0.2f, 1.0f));
    SprintBar->SetPercent(1.0f);
    SprintBar->SetFillColorAndOpacity(FLinearColor(0.95f, 0.82f, 0.15f, 1.0f));
    RangedCooldownBar->SetPercent(1.0f);
    RangedCooldownBar->SetFillColorAndOpacity(FLinearColor(0.2f, 0.75f, 1.0f, 1.0f));
    UVerticalBoxSlot* HealthBarSlot = HealthCardBox->AddChildToVerticalBox(HealthBarSizeBox);
    UVerticalBoxSlot* HealthTextSlot = HealthCardBox->AddChildToVerticalBox(HealthText);
    UVerticalBoxSlot* SprintBarSlot = SprintCardBox->AddChildToVerticalBox(SprintBarSizeBox);
    UVerticalBoxSlot* SprintTextSlot = SprintCardBox->AddChildToVerticalBox(SprintText);
    UVerticalBoxSlot* RangedCooldownBarSlot = RangedCardBox->AddChildToVerticalBox(RangedCooldownBarSizeBox);
    UVerticalBoxSlot* RangedCooldownTextSlot = RangedCardBox->AddChildToVerticalBox(RangedCooldownText);
    UVerticalBoxSlot* TimerTextSlot = CountdownBox->AddChildToVerticalBox(TimerText);
    UVerticalBoxSlot* MatchScoreTextSlot = InfoBox->AddChildToVerticalBox(MatchScoreText);
    UVerticalBoxSlot* RoundStateTextSlot = InfoBox->AddChildToVerticalBox(RoundStateText);
    UVerticalBoxSlot* ResultTextSlot = AnnouncementBox->AddChildToVerticalBox(ResultText);
    UVerticalBoxSlot* NextRoundTextSlot = AnnouncementBox->AddChildToVerticalBox(NextRoundText);
    UVerticalBoxSlot* LobbyTitleTextSlot = LobbyBox->AddChildToVerticalBox(LobbyTitleText);
    UVerticalBoxSlot* LobbyStatusTextSlot = LobbyBox->AddChildToVerticalBox(LobbyStatusText);
    UHorizontalBox* LobbyMainColumnsBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("LobbyMainColumnsBox"));
    UVerticalBox* LobbyLeftColumnBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("LobbyLeftColumnBox"));
    UVerticalBox* LobbyCenterColumnBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("LobbyCenterColumnBox"));
    UVerticalBox* LobbyRightColumnBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("LobbyRightColumnBox"));
    if (!LobbyMainColumnsBox || !LobbyLeftColumnBox || !LobbyCenterColumnBox || !LobbyRightColumnBox)
    {
        return;
    }
    UVerticalBoxSlot* LobbyMainColumnsSlot = LobbyBox->AddChildToVerticalBox(LobbyMainColumnsBox);
    UHorizontalBoxSlot* LobbyLeftColumnSlot = LobbyMainColumnsBox->AddChildToHorizontalBox(LobbyLeftColumnBox);
    UHorizontalBoxSlot* LobbyCenterColumnSlot = LobbyMainColumnsBox->AddChildToHorizontalBox(LobbyCenterColumnBox);
    UHorizontalBoxSlot* LobbyRightColumnSlot = LobbyMainColumnsBox->AddChildToHorizontalBox(LobbyRightColumnBox);
    UVerticalBoxSlot* LobbyLeftTeamListTextSlot = LobbyLeftColumnBox->AddChildToVerticalBox(LobbyLeftTeamListText);
    UVerticalBoxSlot* LobbyModeStatusTextSlot = LobbyCenterColumnBox->AddChildToVerticalBox(LobbyModeStatusText);
    UVerticalBoxSlot* LobbyTeamStatusTextSlot = LobbyCenterColumnBox->AddChildToVerticalBox(LobbyTeamStatusText);
    UVerticalBoxSlot* LobbyNicknameTextBoxSlot = LobbyCenterColumnBox->AddChildToVerticalBox(LobbyNicknameTextBox);
    UVerticalBoxSlot* LobbyPlayerListBoxSlot = LobbyCenterColumnBox->AddChildToVerticalBox(LobbyPlayerListBox);
    UVerticalBoxSlot* ConnectionStatusTextSlot = LobbyCenterColumnBox->AddChildToVerticalBox(ConnectionStatusText);
    UVerticalBoxSlot* JoinAddressTextBoxSlot = LobbyCenterColumnBox->AddChildToVerticalBox(JoinAddressTextBox);
    UVerticalBoxSlot* HostMatchButtonSlot = LobbyCenterColumnBox->AddChildToVerticalBox(HostMatchButton);
    UVerticalBoxSlot* JoinByIpButtonSlot = LobbyCenterColumnBox->AddChildToVerticalBox(JoinByIpButton);
    UVerticalBoxSlot* LobbyRightTeamListTextSlot = LobbyRightColumnBox->AddChildToVerticalBox(LobbyRightTeamListText);
    UVerticalBoxSlot* LobbyControlsTitleTextSlot = LobbyControlsBox->AddChildToVerticalBox(LobbyControlsTitleText);
    UVerticalBoxSlot* LobbyControlsCardsBoxSlot = LobbyControlsBox->AddChildToVerticalBox(LobbyControlsCardsBox);
    UHorizontalBoxSlot* LobbyKeyboardCardSlot = LobbyControlsCardsBox->AddChildToHorizontalBox(LobbyKeyboardCard);
    UHorizontalBoxSlot* LobbyMouseCardSlot = LobbyControlsCardsBox->AddChildToHorizontalBox(LobbyMouseCard);
    UVerticalBoxSlot* LobbyControlsKeyboardTitleTextSlot = LobbyKeyboardCardBox->AddChildToVerticalBox(LobbyControlsKeyboardTitleText);
    UVerticalBoxSlot* LobbyControlsKeyboardMoveTextSlot = LobbyKeyboardCardBox->AddChildToVerticalBox(LobbyControlsKeyboardMoveText);
    UVerticalBoxSlot* LobbyControlsKeyboardSprintTextSlot = LobbyKeyboardCardBox->AddChildToVerticalBox(LobbyControlsKeyboardSprintText);
    UVerticalBoxSlot* LobbyControlsMouseTitleTextSlot = LobbyMouseCardBox->AddChildToVerticalBox(LobbyControlsMouseTitleText);
    UVerticalBoxSlot* LobbyControlsMouseMeleeTextSlot = LobbyMouseCardBox->AddChildToVerticalBox(LobbyControlsMouseMeleeText);
    UVerticalBoxSlot* LobbyControlsMouseRangedTextSlot = LobbyMouseCardBox->AddChildToVerticalBox(LobbyControlsMouseRangedText);
    UVerticalBoxSlot* MatchResultTitleTextSlot = MatchResultBox->AddChildToVerticalBox(MatchResultTitleText);
    UVerticalBoxSlot* MatchResultSummaryTextSlot = MatchResultBox->AddChildToVerticalBox(MatchResultSummaryText);
    UVerticalBoxSlot* SettingsTitleTextSlot = SettingsBox->AddChildToVerticalBox(SettingsTitleText);
    UVerticalBoxSlot* SpectatorHelpTitleTextSlot = SpectatorHelpBox->AddChildToVerticalBox(SpectatorHelpTitleText);
    UVerticalBoxSlot* SpectatorHelpMoveTextSlot = SpectatorHelpBox->AddChildToVerticalBox(SpectatorHelpMoveText);
    UVerticalBoxSlot* SpectatorHelpLookTextSlot = SpectatorHelpBox->AddChildToVerticalBox(SpectatorHelpLookText);
    UVerticalBoxSlot* SpectatorHelpRiseTextSlot = SpectatorHelpBox->AddChildToVerticalBox(SpectatorHelpRiseText);
    UVerticalBoxSlot* SettingsAudioLabelTextSlot = SettingsBox->AddChildToVerticalBox(SettingsAudioLabelText);
    UVerticalBoxSlot* SettingsMasterVolumeTextSlot = SettingsBox->AddChildToVerticalBox(SettingsMasterVolumeText);
    UVerticalBoxSlot* SettingsMasterVolumeSliderSlot = SettingsBox->AddChildToVerticalBox(SettingsMasterVolumeSlider);
    UVerticalBoxSlot* SettingsBgmVolumeTextSlot = SettingsBox->AddChildToVerticalBox(SettingsBgmVolumeText);
    UVerticalBoxSlot* SettingsBgmVolumeSliderSlot = SettingsBox->AddChildToVerticalBox(SettingsBgmVolumeSlider);
    UVerticalBoxSlot* SettingsSfxVolumeTextSlot = SettingsBox->AddChildToVerticalBox(SettingsSfxVolumeText);
    UVerticalBoxSlot* SettingsSfxVolumeSliderSlot = SettingsBox->AddChildToVerticalBox(SettingsSfxVolumeSlider);
    UVerticalBoxSlot* SettingsDisplayModeLabelTextSlot = SettingsBox->AddChildToVerticalBox(SettingsDisplayModeLabelText);
    UVerticalBoxSlot* SettingsWindowModeButtonSlot = SettingsBox->AddChildToVerticalBox(SettingsWindowModeButton);
    UVerticalBoxSlot* SettingsResolutionLabelTextSlot = SettingsBox->AddChildToVerticalBox(SettingsResolutionLabelText);
    UVerticalBoxSlot* SettingsResolutionButtonSlot = SettingsBox->AddChildToVerticalBox(SettingsResolutionButton);
    UVerticalBoxSlot* SettingsVSyncButtonSlot = SettingsBox->AddChildToVerticalBox(SettingsVSyncButton);
    UVerticalBoxSlot* SettingsResumeButtonSlot = SettingsBox->AddChildToVerticalBox(SettingsResumeButton);
    UVerticalBoxSlot* SettingsQuitButtonSlot = SettingsBox->AddChildToVerticalBox(SettingsQuitButton);

    if (HealthBarSlot)
    {
        HealthBarSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 10.0f));
    }

    if (HealthTextSlot)
    {
        HealthTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
    }
    HealthText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
    HealthText->SetFont(FSlateFontInfo(HealthText->GetFont().FontObject, 20, HealthText->GetFont().TypefaceFontName));
    HealthText->SetShadowOffset(FVector2D(1.0f, 1.0f));

    if (SprintBarSlot)
    {
        SprintBarSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
    }

    if (SprintTextSlot)
    {
        SprintTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
    }
    SprintText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.88f, 0.35f, 1.0f)));
    SprintText->SetFont(FSlateFontInfo(SprintText->GetFont().FontObject, 18, SprintText->GetFont().TypefaceFontName));
    SprintText->SetShadowOffset(FVector2D(1.0f, 1.0f));

    if (RangedCooldownBarSlot)
    {
        RangedCooldownBarSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
    }

    if (RangedCooldownTextSlot)
    {
        RangedCooldownTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
    }
    RangedCooldownText->SetColorAndOpacity(FSlateColor(FLinearColor(0.55f, 0.9f, 1.0f, 1.0f)));
    RangedCooldownText->SetFont(FSlateFontInfo(RangedCooldownText->GetFont().FontObject, 18, RangedCooldownText->GetFont().TypefaceFontName));
    RangedCooldownText->SetShadowOffset(FVector2D(1.0f, 1.0f));

    if (MatchScoreTextSlot)
    {
        MatchScoreTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 6.0f));
    }
    MatchScoreText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.85f, 0.2f, 1.0f)));
    MatchScoreText->SetFont(FSlateFontInfo(MatchScoreText->GetFont().FontObject, 18, MatchScoreText->GetFont().TypefaceFontName));
    MatchScoreText->SetShadowOffset(FVector2D(1.0f, 1.0f));

    if (TimerTextSlot)
    {
        TimerTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 6.0f));
    }
    TimerText->SetColorAndOpacity(FSlateColor(FLinearColor(0.5f, 1.0f, 0.55f, 1.0f)));
    TimerText->SetFont(FSlateFontInfo(TimerText->GetFont().FontObject, 18, TimerText->GetFont().TypefaceFontName));
    TimerText->SetShadowOffset(FVector2D(1.0f, 1.0f));

    if (RoundStateTextSlot)
    {
        RoundStateTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 0.0f));
    }
    RoundStateText->SetColorAndOpacity(FSlateColor(FLinearColor(0.45f, 0.9f, 1.0f, 1.0f)));
    RoundStateText->SetFont(FSlateFontInfo(RoundStateText->GetFont().FontObject, 18, RoundStateText->GetFont().TypefaceFontName));
    RoundStateText->SetShadowOffset(FVector2D(1.0f, 1.0f));

    if (ResultTextSlot)
    {
        ResultTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 10.0f));
    }
    ResultText->SetJustification(ETextJustify::Center);
    ResultText->SetColorAndOpacity(FSlateColor(FLinearColor::Yellow));
    ResultText->SetFont(FSlateFontInfo(ResultText->GetFont().FontObject, 42, ResultText->GetFont().TypefaceFontName));
    ResultText->SetShadowOffset(FVector2D(2.0f, 2.0f));
    ResultText->SetVisibility(ESlateVisibility::Collapsed);

    if (NextRoundTextSlot)
    {
        NextRoundTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 0.0f));
    }
    NextRoundText->SetJustification(ETextJustify::Center);
    NextRoundText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.6f, 0.0f, 1.0f)));
    NextRoundText->SetFont(FSlateFontInfo(NextRoundText->GetFont().FontObject, 28, NextRoundText->GetFont().TypefaceFontName));
    NextRoundText->SetShadowOffset(FVector2D(1.0f, 1.0f));
    NextRoundText->SetVisibility(ESlateVisibility::Collapsed);

    if (LobbyTitleTextSlot)
    {
        LobbyTitleTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 10.0f));
    }
    LobbyTitleText->SetJustification(ETextJustify::Center);
    LobbyTitleText->SetColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.96f, 1.0f, 1.0f)));
    LobbyTitleText->SetFont(FSlateFontInfo(LobbyTitleText->GetFont().FontObject, 38, LobbyTitleText->GetFont().TypefaceFontName));
    LobbyTitleText->SetShadowOffset(FVector2D(2.0f, 2.0f));
    LobbyTitleText->SetText(FText::FromString(TEXT("Lobby")));

    if (LobbyStatusTextSlot)
    {
        LobbyStatusTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 12.0f));
    }
    LobbyStatusText->SetJustification(ETextJustify::Center);
    LobbyStatusText->SetColorAndOpacity(FSlateColor(FLinearColor(0.65f, 0.92f, 1.0f, 1.0f)));
    LobbyStatusText->SetFont(FSlateFontInfo(LobbyStatusText->GetFont().FontObject, 24, LobbyStatusText->GetFont().TypefaceFontName));
    LobbyStatusText->SetShadowOffset(FVector2D(1.0f, 1.0f));

    LobbyReadyButton->SetContent(LobbyReadyButtonText);
    LobbyReadyButton->SetBackgroundColor(FLinearColor(0.15f, 0.55f, 0.82f, 1.0f));
    LobbyReadyButton->OnClicked.AddDynamic(this, &UPvPArenaHUDWidget::HandleLobbyReadyButtonClicked);
    UVerticalBoxSlot* LobbyReadyButtonSlot = LobbyCenterColumnBox->AddChildToVerticalBox(LobbyReadyButton);
    if (LobbyReadyButtonSlot)
    {
        LobbyReadyButtonSlot->SetPadding(FMargin(0.0f, 18.0f, 0.0f, 0.0f));
        LobbyReadyButtonSlot->SetHorizontalAlignment(HAlign_Center);
    }

    LobbyReadyButtonText->SetJustification(ETextJustify::Center);
    LobbyReadyButtonText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
    LobbyReadyButtonText->SetFont(FSlateFontInfo(LobbyReadyButtonText->GetFont().FontObject, 22, LobbyReadyButtonText->GetFont().TypefaceFontName));
    LobbyReadyButtonText->SetText(FText::FromString(TEXT("Start Match")));

    if (LobbyModeStatusTextSlot)
    {
        LobbyModeStatusTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
    }
    LobbyModeStatusText->SetJustification(ETextJustify::Center);
    LobbyModeStatusText->SetColorAndOpacity(FSlateColor(FLinearColor(0.95f, 0.85f, 0.4f, 1.0f)));
    LobbyModeStatusText->SetFont(FSlateFontInfo(LobbyModeStatusText->GetFont().FontObject, 18, LobbyModeStatusText->GetFont().TypefaceFontName));

    if (LobbyTeamStatusTextSlot)
    {
        LobbyTeamStatusTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
    }
    LobbyTeamStatusText->SetJustification(ETextJustify::Center);
    LobbyTeamStatusText->SetColorAndOpacity(FSlateColor(FLinearColor(0.8f, 0.94f, 1.0f, 1.0f)));
    LobbyTeamStatusText->SetFont(FSlateFontInfo(LobbyTeamStatusText->GetFont().FontObject, 18, LobbyTeamStatusText->GetFont().TypefaceFontName));

    if (LobbyLeftTeamListTextSlot)
    {
        LobbyLeftTeamListTextSlot->SetPadding(FMargin(0.0f, 6.0f, 0.0f, 4.0f));
    }
    if (LobbyMainColumnsSlot)
    {
        LobbyMainColumnsSlot->SetPadding(FMargin(0.0f, 12.0f, 0.0f, 12.0f));
        LobbyMainColumnsSlot->SetHorizontalAlignment(HAlign_Fill);
    }

    if (LobbyLeftColumnSlot)
    {
        LobbyLeftColumnSlot->SetSize(ESlateSizeRule::Fill);
        LobbyLeftColumnSlot->SetPadding(FMargin(0.0f, 16.0f, 20.0f, 0.0f));
        LobbyLeftColumnSlot->SetHorizontalAlignment(HAlign_Left);
        LobbyLeftColumnSlot->SetVerticalAlignment(VAlign_Top);
    }

    if (LobbyCenterColumnSlot)
    {
        LobbyCenterColumnSlot->SetSize(ESlateSizeRule::Fill);
        LobbyCenterColumnSlot->SetPadding(FMargin(12.0f, 0.0f, 12.0f, 0.0f));
        LobbyCenterColumnSlot->SetHorizontalAlignment(HAlign_Center);
        LobbyCenterColumnSlot->SetVerticalAlignment(VAlign_Top);
    }

    if (LobbyRightColumnSlot)
    {
        LobbyRightColumnSlot->SetSize(ESlateSizeRule::Fill);
        LobbyRightColumnSlot->SetPadding(FMargin(20.0f, 16.0f, 0.0f, 0.0f));
        LobbyRightColumnSlot->SetHorizontalAlignment(HAlign_Right);
        LobbyRightColumnSlot->SetVerticalAlignment(VAlign_Top);
    }

    LobbyLeftTeamListText->SetJustification(ETextJustify::Left);
    LobbyLeftTeamListText->SetColorAndOpacity(FSlateColor(FLinearColor(0.55f, 0.85f, 1.0f, 1.0f)));
    LobbyLeftTeamListText->SetFont(FSlateFontInfo(LobbyLeftTeamListText->GetFont().FontObject, 20, LobbyLeftTeamListText->GetFont().TypefaceFontName));

    if (LobbyRightTeamListTextSlot)
    {
        LobbyRightTeamListTextSlot->SetPadding(FMargin(0.0f, 6.0f, 0.0f, 12.0f));
    }
    LobbyRightTeamListText->SetJustification(ETextJustify::Right);
    LobbyRightTeamListText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.78f, 0.56f, 1.0f)));
    LobbyRightTeamListText->SetFont(FSlateFontInfo(LobbyRightTeamListText->GetFont().FontObject, 20, LobbyRightTeamListText->GetFont().TypefaceFontName));

    LobbyFreeForAllModeButton->SetContent(LobbyFreeForAllModeButtonText);
    LobbyFreeForAllModeButton->SetBackgroundColor(FLinearColor(0.1f, 0.42f, 0.78f, 1.0f));
    LobbyFreeForAllModeButton->OnClicked.AddDynamic(this, &UPvPArenaHUDWidget::HandleLobbyFreeForAllModeButtonClicked);
    if (UVerticalBoxSlot* LobbyFreeForAllModeButtonSlot = LobbyCenterColumnBox->AddChildToVerticalBox(LobbyFreeForAllModeButton))
    {
        LobbyFreeForAllModeButtonSlot->SetPadding(FMargin(0.0f, 4.0f, 0.0f, 0.0f));
        LobbyFreeForAllModeButtonSlot->SetHorizontalAlignment(HAlign_Center);
    }
    LobbyFreeForAllModeButtonText->SetJustification(ETextJustify::Center);
    LobbyFreeForAllModeButtonText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
    LobbyFreeForAllModeButtonText->SetFont(FSlateFontInfo(LobbyFreeForAllModeButtonText->GetFont().FontObject, 18, LobbyFreeForAllModeButtonText->GetFont().TypefaceFontName));
    LobbyFreeForAllModeButtonText->SetText(FText::FromString(TEXT("Free For All")));

    LobbyTeamVersusModeButton->SetContent(LobbyTeamVersusModeButtonText);
    LobbyTeamVersusModeButton->SetBackgroundColor(FLinearColor(0.15f, 0.55f, 0.38f, 1.0f));
    LobbyTeamVersusModeButton->OnClicked.AddDynamic(this, &UPvPArenaHUDWidget::HandleLobbyTeamVersusModeButtonClicked);
    if (UVerticalBoxSlot* LobbyTeamVersusModeButtonSlot = LobbyCenterColumnBox->AddChildToVerticalBox(LobbyTeamVersusModeButton))
    {
        LobbyTeamVersusModeButtonSlot->SetPadding(FMargin(0.0f, 4.0f, 0.0f, 0.0f));
        LobbyTeamVersusModeButtonSlot->SetHorizontalAlignment(HAlign_Center);
    }
    LobbyTeamVersusModeButtonText->SetJustification(ETextJustify::Center);
    LobbyTeamVersusModeButtonText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
    LobbyTeamVersusModeButtonText->SetFont(FSlateFontInfo(LobbyTeamVersusModeButtonText->GetFont().FontObject, 18, LobbyTeamVersusModeButtonText->GetFont().TypefaceFontName));
    LobbyTeamVersusModeButtonText->SetText(FText::FromString(TEXT("Team Versus")));

    LobbyLeftTeamButton->SetContent(LobbyLeftTeamButtonText);
    LobbyLeftTeamButton->SetBackgroundColor(FLinearColor(0.1f, 0.36f, 0.72f, 1.0f));
    LobbyLeftTeamButton->OnClicked.AddDynamic(this, &UPvPArenaHUDWidget::HandleLobbyLeftTeamButtonClicked);
    if (UVerticalBoxSlot* LobbyLeftTeamButtonSlot = LobbyLeftColumnBox->AddChildToVerticalBox(LobbyLeftTeamButton))
    {
        LobbyLeftTeamButtonSlot->SetPadding(FMargin(0.0f, 10.0f, 0.0f, 0.0f));
        LobbyLeftTeamButtonSlot->SetHorizontalAlignment(HAlign_Left);
    }
    LobbyLeftTeamButtonText->SetJustification(ETextJustify::Center);
    LobbyLeftTeamButtonText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
    LobbyLeftTeamButtonText->SetFont(FSlateFontInfo(LobbyLeftTeamButtonText->GetFont().FontObject, 18, LobbyLeftTeamButtonText->GetFont().TypefaceFontName));
    LobbyLeftTeamButtonText->SetText(FText::FromString(TEXT("Join Left Team")));

    LobbyRightTeamButton->SetContent(LobbyRightTeamButtonText);
    LobbyRightTeamButton->SetBackgroundColor(FLinearColor(0.76f, 0.38f, 0.16f, 1.0f));
    LobbyRightTeamButton->OnClicked.AddDynamic(this, &UPvPArenaHUDWidget::HandleLobbyRightTeamButtonClicked);
    if (UVerticalBoxSlot* LobbyRightTeamButtonSlot = LobbyRightColumnBox->AddChildToVerticalBox(LobbyRightTeamButton))
    {
        LobbyRightTeamButtonSlot->SetPadding(FMargin(0.0f, 10.0f, 0.0f, 0.0f));
        LobbyRightTeamButtonSlot->SetHorizontalAlignment(HAlign_Right);
    }
    LobbyRightTeamButtonText->SetJustification(ETextJustify::Center);
    LobbyRightTeamButtonText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
    LobbyRightTeamButtonText->SetFont(FSlateFontInfo(LobbyRightTeamButtonText->GetFont().FontObject, 18, LobbyRightTeamButtonText->GetFont().TypefaceFontName));
    LobbyRightTeamButtonText->SetText(FText::FromString(TEXT("Join Right Team")));

    if (LobbyControlsTitleTextSlot)
    {
        LobbyControlsTitleTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 12.0f));
        LobbyControlsTitleTextSlot->SetHorizontalAlignment(HAlign_Center);
    }

    if (LobbyControlsCardsBoxSlot)
    {
        LobbyControlsCardsBoxSlot->SetHorizontalAlignment(HAlign_Center);
    }
    LobbyControlsTitleText->SetJustification(ETextJustify::Center);
    LobbyControlsTitleText->SetColorAndOpacity(FSlateColor(FLinearColor(0.95f, 0.98f, 1.0f, 1.0f)));
    LobbyControlsTitleText->SetFont(FSlateFontInfo(LobbyControlsTitleText->GetFont().FontObject, 24, LobbyControlsTitleText->GetFont().TypefaceFontName));
    LobbyControlsTitleText->SetText(FText::FromString(TEXT("Controls")));

    if (ConnectionStatusTextSlot)
    {
        ConnectionStatusTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 10.0f));
        ConnectionStatusTextSlot->SetHorizontalAlignment(HAlign_Center);
    }
    ConnectionStatusText->SetJustification(ETextJustify::Center);
    ConnectionStatusText->SetColorAndOpacity(FSlateColor(FLinearColor(0.55f, 0.9f, 1.0f, 1.0f)));
    ConnectionStatusText->SetFont(FSlateFontInfo(ConnectionStatusText->GetFont().FontObject, 16, ConnectionStatusText->GetFont().TypefaceFontName));
    ConnectionStatusText->SetText(FText::FromString(TEXT("Network: Ready to host or join")));

    if (LobbyNicknameTextBoxSlot)
    {
        LobbyNicknameTextBoxSlot->SetPadding(FMargin(0.0f, 4.0f, 0.0f, 0.0f));
        LobbyNicknameTextBoxSlot->SetHorizontalAlignment(HAlign_Center);
    }
    LobbyNicknameTextBox->SetMinDesiredWidth(250.0f);
    LobbyNicknameTextBox->SetHintText(FText::FromString(TEXT("Nickname")));
    LobbyNicknameTextBox->SetText(FText::GetEmpty());
    LobbyNicknameTextBox->OnTextCommitted.AddDynamic(this, &UPvPArenaHUDWidget::HandleLobbyNicknameTextCommitted);

    if (LobbyPlayerListBoxSlot)
    {
        LobbyPlayerListBoxSlot->SetPadding(FMargin(0.0f, 10.0f, 0.0f, 4.0f));
        LobbyPlayerListBoxSlot->SetHorizontalAlignment(HAlign_Fill);
    }

    if (JoinAddressTextBoxSlot)
    {
        JoinAddressTextBoxSlot->SetPadding(FMargin(0.0f, 2.0f, 0.0f, 0.0f));
        JoinAddressTextBoxSlot->SetHorizontalAlignment(HAlign_Center);
    }
    JoinAddressTextBox->SetMinDesiredWidth(250.0f);
    JoinAddressTextBox->SetHintText(FText::FromString(DefaultJoinAddressHint));
    JoinAddressTextBox->SetText(FText::GetEmpty());

    LobbyKeyboardCard->SetContent(LobbyKeyboardCardBox);
    LobbyKeyboardCard->SetPadding(FMargin(18.0f, 14.0f, 18.0f, 14.0f));
    LobbyKeyboardCard->SetBrushColor(FLinearColor(0.07f, 0.12f, 0.18f, 0.92f));
    LobbyMouseCard->SetContent(LobbyMouseCardBox);
    LobbyMouseCard->SetPadding(FMargin(18.0f, 14.0f, 18.0f, 14.0f));
    LobbyMouseCard->SetBrushColor(FLinearColor(0.11f, 0.09f, 0.07f, 0.92f));

    if (LobbyKeyboardCardSlot)
    {
        LobbyKeyboardCardSlot->SetPadding(FMargin(0.0f, 0.0f, 10.0f, 0.0f));
    }

    if (LobbyMouseCardSlot)
    {
        LobbyMouseCardSlot->SetPadding(FMargin(10.0f, 0.0f, 0.0f, 0.0f));
    }

    if (LobbyControlsKeyboardTitleTextSlot)
    {
        LobbyControlsKeyboardTitleTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
    }
    LobbyControlsKeyboardTitleText->SetColorAndOpacity(FSlateColor(FLinearColor(0.96f, 0.98f, 1.0f, 1.0f)));
    LobbyControlsKeyboardTitleText->SetFont(FSlateFontInfo(LobbyControlsKeyboardTitleText->GetFont().FontObject, 18, LobbyControlsKeyboardTitleText->GetFont().TypefaceFontName));
    LobbyControlsKeyboardTitleText->SetText(FText::FromString(TEXT("Keyboard")));

    if (LobbyControlsKeyboardMoveTextSlot)
    {
        LobbyControlsKeyboardMoveTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 4.0f));
    }
    LobbyControlsKeyboardMoveText->SetColorAndOpacity(FSlateColor(FLinearColor(0.82f, 0.92f, 1.0f, 1.0f)));
    LobbyControlsKeyboardMoveText->SetFont(FSlateFontInfo(LobbyControlsKeyboardMoveText->GetFont().FontObject, 17, LobbyControlsKeyboardMoveText->GetFont().TypefaceFontName));
    LobbyControlsKeyboardMoveText->SetText(FText::FromString(TEXT("[ W ]\n[ A ] [ S ] [ D ]   Move")));

    if (LobbyControlsKeyboardSprintTextSlot)
    {
        LobbyControlsKeyboardSprintTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 0.0f));
    }
    LobbyControlsKeyboardSprintText->SetColorAndOpacity(FSlateColor(FLinearColor(0.96f, 0.86f, 0.35f, 1.0f)));
    LobbyControlsKeyboardSprintText->SetFont(FSlateFontInfo(LobbyControlsKeyboardSprintText->GetFont().FontObject, 17, LobbyControlsKeyboardSprintText->GetFont().TypefaceFontName));
    LobbyControlsKeyboardSprintText->SetText(FText::FromString(TEXT("[ Shift ]   Dash")));

    if (LobbyControlsMouseTitleTextSlot)
    {
        LobbyControlsMouseTitleTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
    }
    LobbyControlsMouseTitleText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.97f, 0.92f, 1.0f)));
    LobbyControlsMouseTitleText->SetFont(FSlateFontInfo(LobbyControlsMouseTitleText->GetFont().FontObject, 18, LobbyControlsMouseTitleText->GetFont().TypefaceFontName));
    LobbyControlsMouseTitleText->SetText(FText::FromString(TEXT("Mouse")));

    if (LobbyControlsMouseMeleeTextSlot)
    {
        LobbyControlsMouseMeleeTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 4.0f));
    }
    LobbyControlsMouseMeleeText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.9f, 0.82f, 1.0f)));
    LobbyControlsMouseMeleeText->SetFont(FSlateFontInfo(LobbyControlsMouseMeleeText->GetFont().FontObject, 17, LobbyControlsMouseMeleeText->GetFont().TypefaceFontName));
    LobbyControlsMouseMeleeText->SetText(FText::FromString(TEXT("[ LMB ]   Melee Attack")));

    if (LobbyControlsMouseRangedTextSlot)
    {
        LobbyControlsMouseRangedTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 0.0f));
    }
    LobbyControlsMouseRangedText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.82f, 0.62f, 1.0f)));
    LobbyControlsMouseRangedText->SetFont(FSlateFontInfo(LobbyControlsMouseRangedText->GetFont().FontObject, 17, LobbyControlsMouseRangedText->GetFont().TypefaceFontName));
    LobbyControlsMouseRangedText->SetText(FText::FromString(TEXT("[ RMB ]   Ranged Attack (Charge)")));

    if (MatchResultTitleTextSlot)
    {
        MatchResultTitleTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 10.0f));
    }
    MatchResultTitleText->SetJustification(ETextJustify::Center);
    MatchResultTitleText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.86f, 0.25f, 1.0f)));
    MatchResultTitleText->SetFont(FSlateFontInfo(MatchResultTitleText->GetFont().FontObject, 40, MatchResultTitleText->GetFont().TypefaceFontName));
    MatchResultTitleText->SetShadowOffset(FVector2D(2.0f, 2.0f));
    MatchResultTitleText->SetVisibility(ESlateVisibility::Collapsed);

    if (MatchResultSummaryTextSlot)
    {
        MatchResultSummaryTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 0.0f));
    }
    MatchResultSummaryText->SetJustification(ETextJustify::Center);
    MatchResultSummaryText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.92f, 0.85f, 1.0f)));
    MatchResultSummaryText->SetFont(FSlateFontInfo(MatchResultSummaryText->GetFont().FontObject, 24, MatchResultSummaryText->GetFont().TypefaceFontName));
    MatchResultSummaryText->SetShadowOffset(FVector2D(1.0f, 1.0f));
    MatchResultSummaryText->SetVisibility(ESlateVisibility::Collapsed);

    if (SpectatorHelpTitleTextSlot)
    {
        SpectatorHelpTitleTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
    }
    SpectatorHelpTitleText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.92f, 0.45f, 1.0f)));
    SpectatorHelpTitleText->SetFont(FSlateFontInfo(SpectatorHelpTitleText->GetFont().FontObject, 20, SpectatorHelpTitleText->GetFont().TypefaceFontName));
    SpectatorHelpTitleText->SetText(FText::FromString(TEXT("관전 조작")));

    if (SpectatorHelpMoveTextSlot)
    {
        SpectatorHelpMoveTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 4.0f));
    }
    SpectatorHelpMoveText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
    SpectatorHelpMoveText->SetFont(FSlateFontInfo(SpectatorHelpMoveText->GetFont().FontObject, 16, SpectatorHelpMoveText->GetFont().TypefaceFontName));
    SpectatorHelpMoveText->SetText(FText::FromString(TEXT("WASD: 이동")));

    if (SpectatorHelpLookTextSlot)
    {
        SpectatorHelpLookTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 4.0f));
    }
    SpectatorHelpLookText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
    SpectatorHelpLookText->SetFont(FSlateFontInfo(SpectatorHelpLookText->GetFont().FontObject, 16, SpectatorHelpLookText->GetFont().TypefaceFontName));
    SpectatorHelpLookText->SetText(FText::FromString(TEXT("마우스: 시점 회전")));

    SpectatorHelpRiseText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
    SpectatorHelpRiseText->SetFont(FSlateFontInfo(SpectatorHelpRiseText->GetFont().FontObject, 16, SpectatorHelpRiseText->GetFont().TypefaceFontName));
    SpectatorHelpRiseText->SetText(FText::FromString(TEXT("Q / E: 하강 / 상승")));

    HostMatchButton->SetContent(HostMatchButtonText);
    HostMatchButton->SetBackgroundColor(FLinearColor(0.18f, 0.52f, 0.34f, 1.0f));
    HostMatchButton->OnClicked.AddDynamic(this, &UPvPArenaHUDWidget::HandleHostMatchButtonClicked);
    if (HostMatchButtonSlot)
    {
        HostMatchButtonSlot->SetPadding(FMargin(0.0f, 6.0f, 0.0f, 0.0f));
        HostMatchButtonSlot->SetHorizontalAlignment(HAlign_Center);
    }
    HostMatchButtonText->SetJustification(ETextJustify::Center);
    HostMatchButtonText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
    HostMatchButtonText->SetFont(FSlateFontInfo(HostMatchButtonText->GetFont().FontObject, 18, HostMatchButtonText->GetFont().TypefaceFontName));
    HostMatchButtonText->SetText(FText::FromString(TEXT("Host Match")));

    JoinByIpButton->SetContent(JoinByIpButtonText);
    JoinByIpButton->SetBackgroundColor(FLinearColor(0.68f, 0.45f, 0.16f, 1.0f));
    JoinByIpButton->OnClicked.AddDynamic(this, &UPvPArenaHUDWidget::HandleJoinByIpButtonClicked);
    if (JoinByIpButtonSlot)
    {
        JoinByIpButtonSlot->SetPadding(FMargin(0.0f, 6.0f, 0.0f, 0.0f));
        JoinByIpButtonSlot->SetHorizontalAlignment(HAlign_Center);
    }
    JoinByIpButtonText->SetJustification(ETextJustify::Center);
    JoinByIpButtonText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
    JoinByIpButtonText->SetFont(FSlateFontInfo(JoinByIpButtonText->GetFont().FontObject, 18, JoinByIpButtonText->GetFont().TypefaceFontName));
    JoinByIpButtonText->SetText(FText::FromString(TEXT("Join By IP")));

    if (SettingsTitleTextSlot)
    {
        SettingsTitleTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 12.0f));
        SettingsTitleTextSlot->SetHorizontalAlignment(HAlign_Center);
    }
    SettingsTitleText->SetJustification(ETextJustify::Center);
    SettingsTitleText->SetColorAndOpacity(FSlateColor(FLinearColor(0.95f, 0.98f, 1.0f, 1.0f)));
    SettingsTitleText->SetFont(FSlateFontInfo(SettingsTitleText->GetFont().FontObject, 30, SettingsTitleText->GetFont().TypefaceFontName));
    SettingsTitleText->SetText(FText::FromString(TEXT("Settings")));

    auto ConfigureSettingsLabel = [](UTextBlock* TextBlock, const FString& Text, int32 FontSize)
    {
        if (!TextBlock)
        {
            return;
        }

        TextBlock->SetColorAndOpacity(FSlateColor(FLinearColor(0.84f, 0.92f, 1.0f, 1.0f)));
        TextBlock->SetFont(FSlateFontInfo(TextBlock->GetFont().FontObject, FontSize, TextBlock->GetFont().TypefaceFontName));
        TextBlock->SetText(FText::FromString(Text));
    };

    if (SettingsAudioLabelTextSlot)
    {
        SettingsAudioLabelTextSlot->SetPadding(FMargin(0.0f, 4.0f, 0.0f, 8.0f));
    }
    ConfigureSettingsLabel(SettingsAudioLabelText, TEXT("Audio"), 20);

    if (SettingsMasterVolumeTextSlot)
    {
        SettingsMasterVolumeTextSlot->SetPadding(FMargin(0.0f, 2.0f, 0.0f, 2.0f));
    }
    ConfigureSettingsLabel(SettingsMasterVolumeText, TEXT("Master Volume"), 16);

    if (SettingsBgmVolumeTextSlot)
    {
        SettingsBgmVolumeTextSlot->SetPadding(FMargin(0.0f, 8.0f, 0.0f, 2.0f));
    }
    ConfigureSettingsLabel(SettingsBgmVolumeText, TEXT("BGM Volume"), 16);

    if (SettingsSfxVolumeTextSlot)
    {
        SettingsSfxVolumeTextSlot->SetPadding(FMargin(0.0f, 8.0f, 0.0f, 2.0f));
    }
    ConfigureSettingsLabel(SettingsSfxVolumeText, TEXT("SFX Volume"), 16);

    if (SettingsDisplayModeLabelTextSlot)
    {
        SettingsDisplayModeLabelTextSlot->SetPadding(FMargin(0.0f, 16.0f, 0.0f, 6.0f));
    }
    ConfigureSettingsLabel(SettingsDisplayModeLabelText, TEXT("Window Mode"), 20);

    if (SettingsResolutionLabelTextSlot)
    {
        SettingsResolutionLabelTextSlot->SetPadding(FMargin(0.0f, 12.0f, 0.0f, 6.0f));
    }
    ConfigureSettingsLabel(SettingsResolutionLabelText, TEXT("Resolution"), 20);

    SettingsMasterVolumeSlider->SetMinValue(0.0f);
    SettingsMasterVolumeSlider->SetMaxValue(1.0f);
    SettingsMasterVolumeSlider->SetStepSize(0.05f);
    SettingsMasterVolumeSlider->OnValueChanged.AddDynamic(this, &UPvPArenaHUDWidget::HandleMasterVolumeSliderChanged);

    SettingsBgmVolumeSlider->SetMinValue(0.0f);
    SettingsBgmVolumeSlider->SetMaxValue(1.0f);
    SettingsBgmVolumeSlider->SetStepSize(0.05f);
    SettingsBgmVolumeSlider->OnValueChanged.AddDynamic(this, &UPvPArenaHUDWidget::HandleBgmVolumeSliderChanged);

    SettingsSfxVolumeSlider->SetMinValue(0.0f);
    SettingsSfxVolumeSlider->SetMaxValue(1.0f);
    SettingsSfxVolumeSlider->SetStepSize(0.05f);
    SettingsSfxVolumeSlider->OnValueChanged.AddDynamic(this, &UPvPArenaHUDWidget::HandleSfxVolumeSliderChanged);

    auto ConfigureSettingsButton = [](UButton* Button, UTextBlock* Label, const FLinearColor& Color, const FString& Text)
    {
        if (!Button || !Label)
        {
            return;
        }

        Button->SetContent(Label);
        Button->SetBackgroundColor(Color);
        Label->SetJustification(ETextJustify::Center);
        Label->SetColorAndOpacity(FSlateColor(FLinearColor::White));
        Label->SetFont(FSlateFontInfo(Label->GetFont().FontObject, 16, Label->GetFont().TypefaceFontName));
        Label->SetText(FText::FromString(Text));
    };

    ConfigureSettingsButton(SettingsWindowModeButton, SettingsWindowModeButtonText, FLinearColor(0.22f, 0.37f, 0.58f, 1.0f), TEXT("Window Mode"));
    ConfigureSettingsButton(SettingsResolutionButton, SettingsResolutionButtonText, FLinearColor(0.28f, 0.43f, 0.28f, 1.0f), TEXT("Resolution"));
    ConfigureSettingsButton(SettingsVSyncButton, SettingsVSyncButtonText, FLinearColor(0.45f, 0.36f, 0.18f, 1.0f), TEXT("VSync"));
    ConfigureSettingsButton(SettingsResumeButton, SettingsResumeButtonText, FLinearColor(0.18f, 0.52f, 0.34f, 1.0f), TEXT("Resume"));
    ConfigureSettingsButton(SettingsQuitButton, SettingsQuitButtonText, FLinearColor(0.62f, 0.18f, 0.18f, 1.0f), TEXT("Quit To Desktop"));

    SettingsWindowModeButton->OnClicked.AddDynamic(this, &UPvPArenaHUDWidget::HandleSettingsWindowModeButtonClicked);
    SettingsResolutionButton->OnClicked.AddDynamic(this, &UPvPArenaHUDWidget::HandleSettingsResolutionButtonClicked);
    SettingsVSyncButton->OnClicked.AddDynamic(this, &UPvPArenaHUDWidget::HandleSettingsVSyncButtonClicked);
    SettingsResumeButton->OnClicked.AddDynamic(this, &UPvPArenaHUDWidget::HandleSettingsResumeButtonClicked);
    SettingsQuitButton->OnClicked.AddDynamic(this, &UPvPArenaHUDWidget::HandleSettingsQuitButtonClicked);

    if (SettingsWindowModeButtonSlot)
    {
        SettingsWindowModeButtonSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 6.0f));
        SettingsWindowModeButtonSlot->SetHorizontalAlignment(HAlign_Fill);
    }

    if (SettingsResolutionButtonSlot)
    {
        SettingsResolutionButtonSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 6.0f));
        SettingsResolutionButtonSlot->SetHorizontalAlignment(HAlign_Fill);
    }

    if (SettingsVSyncButtonSlot)
    {
        SettingsVSyncButtonSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 12.0f));
        SettingsVSyncButtonSlot->SetHorizontalAlignment(HAlign_Fill);
    }

    if (SettingsResumeButtonSlot)
    {
        SettingsResumeButtonSlot->SetPadding(FMargin(0.0f, 8.0f, 0.0f, 6.0f));
        SettingsResumeButtonSlot->SetHorizontalAlignment(HAlign_Fill);
    }

    if (SettingsQuitButtonSlot)
    {
        SettingsQuitButtonSlot->SetHorizontalAlignment(HAlign_Fill);
    }

    RangedCrosshairHorizontalLine->SetBrushColor(FLinearColor(0.95f, 0.98f, 1.0f, 0.95f));
    RangedCrosshairVerticalLine->SetBrushColor(FLinearColor(0.95f, 0.98f, 1.0f, 0.95f));
    RangedCrosshairOverlay->SetVisibility(ESlateVisibility::Collapsed);

    RefreshSettingsMenuState();
}

void UPvPArenaHUDWidget::BuildHealthDisplayState(const APvPArenaCharacter* Character, float& OutHealthPercent, FString& OutHealthLabel)
{
    if (!Character)
    {
        OutHealthPercent = 0.0f;
        OutHealthLabel = TEXT("HP: --");
        return;
    }

    const float MaxHp = FMath::Max(1.0f, Character->GetMaxHealth());
    const float CurrentHp = Character->GetCurrentHealth();
    OutHealthPercent = CurrentHp / MaxHp;
    OutHealthLabel = FString::Printf(TEXT("HP: %.0f / %.0f"), CurrentHp, MaxHp);
}

void UPvPArenaHUDWidget::BuildSprintDisplayState(const APvPArenaCharacter* Character, float& OutSprintPercent, FString& OutSprintLabel)
{
    if (!Character)
    {
        OutSprintPercent = 0.0f;
        OutSprintLabel = TEXT("Dash: --");
        return;
    }

    OutSprintPercent = Character->GetSprintEnergyAlpha();
    OutSprintLabel = FString::Printf(
        TEXT("Dash: %.1fs %s"),
        Character->GetCurrentSprintEnergySeconds(),
        Character->IsSprinting() ? TEXT("(Active)") : TEXT("(Charging)"));
}

void UPvPArenaHUDWidget::BuildRangedCooldownDisplayState(const UPvPCombatComponent* CombatComponent, float NowSeconds, float& OutCooldownPercent, FString& OutCooldownLabel)
{
    if (!CombatComponent)
    {
        OutCooldownPercent = 0.0f;
        OutCooldownLabel = TEXT("Ranged: --");
        return;
    }

    const float RemainingCooldown = CombatComponent->GetRemainingRangedCooldown(NowSeconds);
    OutCooldownPercent = CombatComponent->GetRangedCooldownAlpha(NowSeconds);
    OutCooldownLabel = RemainingCooldown > 0.0f
        ? FString::Printf(TEXT("Ranged: %.1fs"), RemainingCooldown)
        : FString(TEXT("Ranged: Ready"));
}

ESlateVisibility UPvPArenaHUDWidget::BuildRangedCrosshairVisibilityState(
    const APvPArenaCharacter* Character,
    const APvPArenaGameState* GameState)
{
    return Character
        && GameState
        && GameState->GetMatchPhase() == EPvPAMatchPhase::Playing
        ? ESlateVisibility::Visible
        : ESlateVisibility::Collapsed;
}

float UPvPArenaHUDWidget::BuildRangedCrosshairVerticalOffsetState(
    const APvPArenaCharacter* Character,
    const APvPArenaGameState* GameState)
{
    if (BuildRangedCrosshairVisibilityState(Character, GameState) != ESlateVisibility::Visible)
    {
        return 0.0f;
    }

    return Character && Character->IsRangedChargeInputHeld()
        ? 0.0f
        : DefaultRangedCrosshairVerticalOffset;
}

FString UPvPArenaHUDWidget::BuildBackgroundMusicAssetPathForMatchPhase(uint8 MatchPhaseValue)
{
    switch (static_cast<EPvPAMatchPhase>(MatchPhaseValue))
    {
    case EPvPAMatchPhase::Playing:
        return FString(GameplayBackgroundMusicPath);

    case EPvPAMatchPhase::Lobby:
    case EPvPAMatchPhase::MatchEnd:
    default:
        return FString(NonCombatBackgroundMusicPath);
    }
}

TArray<FString> UPvPArenaHUDWidget::BuildLobbyParticipantLabels(const APvPArenaGameState* GameState)
{
    TArray<FString> ParticipantLabels;
    if (!GameState)
    {
        return ParticipantLabels;
    }

    for (APlayerState* PlayerState : GameState->PlayerArray)
    {
        const APvPArenaPlayerState* PvPPlayerState = Cast<APvPArenaPlayerState>(PlayerState);
        if (!PvPPlayerState)
        {
            continue;
        }

        ParticipantLabels.Add(
            PvPPlayerState->GetDisplayNickname().IsEmpty()
                ? FString(TEXT("Player"))
                : PvPPlayerState->GetDisplayNickname());
    }

    return ParticipantLabels;
}

int32 UPvPArenaHUDWidget::BuildTeamRoundWins(const APvPArenaGameState* GameState, EPvPALobbyTeam LobbyTeam)
{
    if (!GameState)
    {
        return 0;
    }

    int32 TeamRoundWins = 0;
    for (APlayerState* PlayerState : GameState->PlayerArray)
    {
        const APvPArenaPlayerState* PvPPlayerState = Cast<APvPArenaPlayerState>(PlayerState);
        if (PvPPlayerState && PvPPlayerState->GetLobbyTeam() == LobbyTeam)
        {
            TeamRoundWins = FMath::Max(TeamRoundWins, PvPPlayerState->GetRoundWins());
        }
    }

    return TeamRoundWins;
}

FString UPvPArenaHUDWidget::LobbyTeamToDisplayName(EPvPALobbyTeam LobbyTeamValue)
{
    switch (LobbyTeamValue)
    {
    case EPvPALobbyTeam::Left:
        return TEXT("Blue");
    case EPvPALobbyTeam::Right:
        return TEXT("Red");
    default:
        return TEXT("None");
    }
}

FString UPvPArenaHUDWidget::BuildMatchScoreSummary(
    const APvPArenaPlayerState* LocalPlayerState,
    const APvPArenaGameState* GameState)
{
    if (!LocalPlayerState)
    {
        return FString();
    }

    if (LocalPlayerState->GetLobbyMatchMode() == EPvPALobbyMatchMode::TeamVersus)
    {
        const int32 RoundWinsToWin = GameState ? GameState->GetRoundWinsToWin() : 3;
        return FString::Printf(
            TEXT("Match Score: Blue %d / %d | Red %d / %d"),
            BuildTeamRoundWins(GameState, EPvPALobbyTeam::Left),
            RoundWinsToWin,
            BuildTeamRoundWins(GameState, EPvPALobbyTeam::Right),
            RoundWinsToWin);
    }

    return FString::Printf(
        TEXT("Match Score: %d / %d | K / D: %d / %d"),
        LocalPlayerState->GetMatchKills(),
        GameState ? GameState->GetScoreLimit() : 5,
        LocalPlayerState->GetMatchKills(),
        LocalPlayerState->GetMatchDeaths());
}

FString UPvPArenaHUDWidget::BuildRoundResultLabel(
    const APvPArenaPlayerState* LocalPlayerState,
    const APvPArenaGameState* GameState)
{
    if (!LocalPlayerState || !GameState)
    {
        return TEXT("Unknown");
    }

    if (LocalPlayerState->GetLobbyMatchMode() == EPvPALobbyMatchMode::TeamVersus)
    {
        const APvPArenaPlayerState* RoundWinner = GameState->GetRoundWinner();
        if (!RoundWinner)
        {
            return TEXT("Draw");
        }

        return RoundWinner->GetLobbyTeam() == LocalPlayerState->GetLobbyTeam()
            ? TEXT("Victory")
            : TEXT("Defeat");
    }

    const int32 LocalKills = LocalPlayerState->GetRoundKills();
    int32 HighestOpponentKills = TNumericLimits<int32>::Min();
    bool bFoundOpponent = false;

    for (APlayerState* PlayerState : GameState->PlayerArray)
    {
        const APvPArenaPlayerState* PvPState = Cast<APvPArenaPlayerState>(PlayerState);
        if (!PvPState || PvPState == LocalPlayerState)
        {
            continue;
        }

        HighestOpponentKills = FMath::Max(HighestOpponentKills, PvPState->GetRoundKills());
        bFoundOpponent = true;
    }

    if (!bFoundOpponent)
    {
        return TEXT("Pending");
    }

    if (LocalKills > HighestOpponentKills)
    {
        return TEXT("Victory");
    }

    if (LocalKills < HighestOpponentKills)
    {
        return TEXT("Defeat");
    }

    return TEXT("Draw");
}

FString UPvPArenaHUDWidget::BuildMatchResultLabel(
    const APvPArenaPlayerState* LocalPlayerState,
    const APvPArenaGameState* GameState)
{
    if (!LocalPlayerState || !GameState)
    {
        return TEXT("Match Complete");
    }

    const APvPArenaPlayerState* MatchWinner = GameState->GetMatchWinner();
    if (!MatchWinner)
    {
        return TEXT("Match Complete");
    }

    if (LocalPlayerState->GetLobbyMatchMode() == EPvPALobbyMatchMode::TeamVersus)
    {
        return MatchWinner->GetLobbyTeam() == LocalPlayerState->GetLobbyTeam()
            ? TEXT("Final Victory")
            : TEXT("Final Defeat");
    }

    return MatchWinner == LocalPlayerState ? TEXT("Final Victory") : TEXT("Final Defeat");
}

FString UPvPArenaHUDWidget::BuildLobbyNicknameTextBoxValue(const FString& DraftNickname, const FString& ReplicatedNickname)
{
    const FString NormalizedDraft = APvPArenaPlayerState::BuildNormalizedDisplayNickname(DraftNickname);
    if (!NormalizedDraft.IsEmpty())
    {
        return NormalizedDraft;
    }

    return APvPArenaPlayerState::BuildNormalizedDisplayNickname(ReplicatedNickname);
}

void UPvPArenaHUDWidget::RefreshBackgroundMusic(const APvPArenaGameState* GameState)
{
    if (!BackgroundMusicAudioComponent)
    {
        return;
    }

    BackgroundMusicAudioComponent->SetVolumeMultiplier(MasterVolume * BackgroundMusicVolume);

    const USoundBase* DesiredMusic = GameState && GameState->GetMatchPhase() == EPvPAMatchPhase::Playing
        ? GameplayBackgroundMusic
        : NonCombatBackgroundMusic;

    if (!DesiredMusic)
    {
        BackgroundMusicAudioComponent->Stop();
        CurrentBackgroundMusic = nullptr;
        return;
    }

    if (CurrentBackgroundMusic == DesiredMusic && BackgroundMusicAudioComponent->IsPlaying())
    {
        return;
    }

    BackgroundMusicAudioComponent->Stop();
    BackgroundMusicAudioComponent->SetSound(const_cast<USoundBase*>(DesiredMusic));
    BackgroundMusicAudioComponent->Play(0.0f);
    CurrentBackgroundMusic = const_cast<USoundBase*>(DesiredMusic);
}

void UPvPArenaHUDWidget::RefreshWidgetData()
{
    APlayerController* PlayerController = GetOwningPlayer();
    if (!PlayerController)
    {
        return;
    }

    float HealthPercent = 0.0f;
    FString HealthLabel;
    const APvPArenaCharacter* Character = Cast<APvPArenaCharacter>(PlayerController->GetPawn());
    BuildHealthDisplayState(Character, HealthPercent, HealthLabel);

    float SprintPercent = 0.0f;
    FString SprintLabel;
    BuildSprintDisplayState(Character, SprintPercent, SprintLabel);

    float RangedCooldownPercent = 0.0f;
    FString RangedCooldownLabel;
    const APvPArenaGameState* PvPGameState = GetWorld() ? GetWorld()->GetGameState<APvPArenaGameState>() : nullptr;
    const float CooldownNowSeconds = PvPGameState ? PvPGameState->GetServerWorldTimeSeconds() : (GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f);
    BuildRangedCooldownDisplayState(
        Character ? Character->GetCombatComponent() : nullptr,
        CooldownNowSeconds,
        RangedCooldownPercent,
        RangedCooldownLabel);

    ApplyAudioSettings();
    RefreshCrosshairVisibility();
    RefreshBackgroundMusic(PvPGameState);

    if (HealthBar)
    {
        HealthBar->SetPercent(HealthPercent);
    }

    if (HealthText)
    {
        HealthText->SetText(FText::FromString(HealthLabel));
    }

    if (SprintBar)
    {
        SprintBar->SetPercent(SprintPercent);
    }

    if (SprintText)
    {
        SprintText->SetText(FText::FromString(SprintLabel));
    }

    if (RangedCooldownBar)
    {
        RangedCooldownBar->SetPercent(RangedCooldownPercent);
    }

    if (RangedCooldownText)
    {
        RangedCooldownText->SetText(FText::FromString(RangedCooldownLabel));
    }

    const APvPArenaPlayerState* LocalPlayerState = PlayerController->GetPlayerState<APvPArenaPlayerState>();
    if (LocalPlayerState)
    {
        if (MatchScoreText)
        {
            MatchScoreText->SetVisibility(ESlateVisibility::Visible);
            MatchScoreText->SetText(FText::FromString(BuildMatchScoreSummary(LocalPlayerState, PvPGameState)));
        }
    }

    if (PvPGameState)
    {
        const bool bIsRoundEnd = PvPGameState->GetRoundState() == EPvPARoundState::RoundEnd
            && PvPGameState->GetMatchPhase() == EPvPAMatchPhase::Playing;
        const bool bIsLobby = PvPGameState->GetMatchPhase() == EPvPAMatchPhase::Lobby;
        const bool bIsMatchEnd = PvPGameState->GetMatchPhase() == EPvPAMatchPhase::MatchEnd;
        const bool bIsSpectating = PlayerController->GetSpectatorPawn() != nullptr && !bIsLobby && !bIsMatchEnd;

        ApplyLobbyInputMode(PlayerController, bIsLobby || bSettingsMenuOpen);

        if (AnnouncementPanel)
        {
            AnnouncementPanel->SetVisibility(bIsRoundEnd ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
        }

        if (LobbyPanel)
        {
            LobbyPanel->SetVisibility(bIsLobby ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
        }

        if (LobbyControlsPanel)
        {
            LobbyControlsPanel->SetVisibility(bIsLobby ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
        }

        if (MatchResultPanel)
        {
            MatchResultPanel->SetVisibility(bIsMatchEnd ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
        }

        if (SpectatorHelpPanel)
        {
            SpectatorHelpPanel->SetVisibility(bIsSpectating ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
        }

        if (TimerText)
        {
            if (bIsLobby)
            {
                TimerText->SetText(FText::FromString(TEXT("Time: Waiting")));
            }
            else if (bIsMatchEnd)
            {
                TimerText->SetText(FText::FromString(FString::Printf(TEXT("Lobby In: %d"), PvPGameState->GetRemainingMatchEndTimeSeconds())));
            }
            else
            {
                TimerText->SetText(FText::FromString(FString::Printf(TEXT("Time: %d"), PvPGameState->GetRemainingRoundTimeSeconds())));
            }
        }

        if (RoundStateText)
        {
            RoundStateText->SetText(FText::FromString(
                FString::Printf(
                    TEXT("Phase: %s | Round: %s"),
                    *MatchPhaseToString(static_cast<uint8>(PvPGameState->GetMatchPhase())),
                    *RoundStateToString(static_cast<uint8>(PvPGameState->GetRoundState())))));
        }

        if (ResultText)
        {
            if (bIsRoundEnd)
            {
                ResultText->SetVisibility(ESlateVisibility::Visible);
                ResultText->SetText(FText::FromString(GetRoundResultText(PlayerController, PvPGameState)));
            }
            else
            {
                ResultText->SetVisibility(ESlateVisibility::Collapsed);
                ResultText->SetText(FText::GetEmpty());
            }
        }

        if (NextRoundText)
        {
            if (bIsRoundEnd)
            {
                NextRoundText->SetVisibility(ESlateVisibility::Visible);
                NextRoundText->SetText(FText::FromString(
                    FString::Printf(TEXT("Next Round In: %d"), PvPGameState->GetRemainingRoundEndTimeSeconds())));
            }
            else
            {
                NextRoundText->SetVisibility(ESlateVisibility::Collapsed);
                NextRoundText->SetText(FText::GetEmpty());
            }
        }

        if (LobbyStatusText)
        {
            LobbyStatusText->SetText(FText::FromString(bIsLobby ? GetLobbyStatusText(PlayerController, PvPGameState) : FString()));
        }

        const EPvPALobbyMatchMode LobbyMatchMode = LocalPlayerState ? LocalPlayerState->GetLobbyMatchMode() : EPvPALobbyMatchMode::FreeForAll;
        const bool bIsTeamVersus = LobbyMatchMode == EPvPALobbyMatchMode::TeamVersus;

        if (LobbyNicknameTextBox && LocalPlayerState && !LobbyNicknameTextBox->HasKeyboardFocus())
        {
            const FString DesiredNickname = BuildLobbyNicknameTextBoxValue(FString(), LocalPlayerState->GetDisplayNickname());
            const FString CurrentNickname = LobbyNicknameTextBox->GetText().ToString();
            if (!DesiredNickname.Equals(CurrentNickname, ESearchCase::CaseSensitive))
            {
                bUpdatingLobbyNicknameText = true;
                LobbyNicknameTextBox->SetText(FText::FromString(DesiredNickname));
                bUpdatingLobbyNicknameText = false;
            }
        }

        RefreshLobbyParticipantList(PvPGameState);

        if (LobbyModeStatusText)
        {
            LobbyModeStatusText->SetText(FText::FromString(
                bIsLobby ? FString::Printf(TEXT("Mode: %s"), *LobbyMatchModeToString(static_cast<uint8>(LobbyMatchMode))) : FString()));
        }

        if (LobbyTeamStatusText)
        {
            LobbyTeamStatusText->SetText(FText::FromString(
                bIsLobby
                    ? FString::Printf(TEXT("Your Team: %s"), LocalPlayerState ? *LobbyTeamToString(static_cast<uint8>(LocalPlayerState->GetLobbyTeam())) : TEXT("None"))
                    : FString()));
        }

        if (LobbyLeftTeamListText)
        {
            LobbyLeftTeamListText->SetVisibility(bIsLobby && bIsTeamVersus ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
            LobbyLeftTeamListText->SetText(FText::FromString(bIsLobby ? BuildLobbyTeamListText(PvPGameState, static_cast<uint8>(EPvPALobbyTeam::Left)) : FString()));
        }

        if (LobbyRightTeamListText)
        {
            LobbyRightTeamListText->SetVisibility(bIsLobby && bIsTeamVersus ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
            LobbyRightTeamListText->SetText(FText::FromString(bIsLobby ? BuildLobbyTeamListText(PvPGameState, static_cast<uint8>(EPvPALobbyTeam::Right)) : FString()));
        }

        if (LobbyReadyButton)
        {
            LobbyReadyButton->SetVisibility(
                bIsLobby && ShouldShowLobbyStartButton(PlayerController, PvPGameState)
                    ? ESlateVisibility::Visible
                    : ESlateVisibility::Collapsed);
        }

        if (LobbyReadyButtonText)
        {
            LobbyReadyButtonText->SetText(FText::FromString(TEXT("Start Match")));
        }

        if (LobbyFreeForAllModeButton)
        {
            LobbyFreeForAllModeButton->SetVisibility(bIsLobby && PlayerController && PlayerController->HasAuthority() ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
        }

        if (LobbyTeamVersusModeButton)
        {
            LobbyTeamVersusModeButton->SetVisibility(bIsLobby && PlayerController && PlayerController->HasAuthority() ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
        }

        if (LobbyLeftTeamButton)
        {
            LobbyLeftTeamButton->SetVisibility(bIsLobby && bIsTeamVersus ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
        }

        if (LobbyRightTeamButton)
        {
            LobbyRightTeamButton->SetVisibility(bIsLobby && bIsTeamVersus ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
        }

        if (MatchResultTitleText)
        {
            if (bIsMatchEnd)
            {
                MatchResultTitleText->SetVisibility(ESlateVisibility::Visible);
                MatchResultTitleText->SetText(FText::FromString(GetMatchResultText(PlayerController, PvPGameState)));
            }
            else
            {
                MatchResultTitleText->SetVisibility(ESlateVisibility::Collapsed);
                MatchResultTitleText->SetText(FText::GetEmpty());
            }
        }

        if (MatchResultSummaryText)
        {
            if (bIsMatchEnd)
            {
                MatchResultSummaryText->SetVisibility(ESlateVisibility::Visible);
                MatchResultSummaryText->SetText(FText::FromString(GetMatchSummaryText(PlayerController, PvPGameState)));
            }
            else
            {
                MatchResultSummaryText->SetVisibility(ESlateVisibility::Collapsed);
                MatchResultSummaryText->SetText(FText::GetEmpty());
            }
        }
    }
}

void UPvPArenaHUDWidget::ApplyLobbyInputMode(APlayerController* PlayerController, bool bEnableLobbyInput)
{
    if (!PlayerController || bLobbyInputModeActive == bEnableLobbyInput)
    {
        return;
    }

    if (bEnableLobbyInput)
    {
        FInputModeGameAndUI InputMode;
        InputMode.SetWidgetToFocus(TakeWidget());
        InputMode.SetHideCursorDuringCapture(false);
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        PlayerController->SetInputMode(InputMode);
        PlayerController->bShowMouseCursor = true;
    }
    else
    {
        FInputModeGameOnly InputMode;
        PlayerController->SetInputMode(InputMode);
        PlayerController->bShowMouseCursor = false;
    }

    bLobbyInputModeActive = bEnableLobbyInput;
}

void UPvPArenaHUDWidget::HandleLobbyReadyButtonClicked()
{
    APvPArenaPlayerController* PvPPlayerController = Cast<APvPArenaPlayerController>(GetOwningPlayer());
    if (!PvPPlayerController)
    {
        return;
    }

    PvPPlayerController->RequestLobbyMatchStart();
}

void UPvPArenaHUDWidget::HandleLobbyFreeForAllModeButtonClicked()
{
    if (APvPArenaPlayerController* PvPPlayerController = Cast<APvPArenaPlayerController>(GetOwningPlayer()))
    {
        PvPPlayerController->RequestLobbyMatchModeChange(EPvPALobbyMatchMode::FreeForAll);
    }
}

void UPvPArenaHUDWidget::HandleLobbyTeamVersusModeButtonClicked()
{
    if (APvPArenaPlayerController* PvPPlayerController = Cast<APvPArenaPlayerController>(GetOwningPlayer()))
    {
        PvPPlayerController->RequestLobbyMatchModeChange(EPvPALobbyMatchMode::TeamVersus);
    }
}

void UPvPArenaHUDWidget::HandleLobbyLeftTeamButtonClicked()
{
    if (APvPArenaPlayerController* PvPPlayerController = Cast<APvPArenaPlayerController>(GetOwningPlayer()))
    {
        PvPPlayerController->RequestLobbyTeamSelection(EPvPALobbyTeam::Left);
    }
}

void UPvPArenaHUDWidget::HandleLobbyRightTeamButtonClicked()
{
    if (APvPArenaPlayerController* PvPPlayerController = Cast<APvPArenaPlayerController>(GetOwningPlayer()))
    {
        PvPPlayerController->RequestLobbyTeamSelection(EPvPALobbyTeam::Right);
    }
}

void UPvPArenaHUDWidget::HandleHostMatchButtonClicked()
{
    ExecuteTravelCommand(BuildHostTravelCommand(), TEXT("Network: Hosting match on port 7777"));
}

void UPvPArenaHUDWidget::HandleJoinByIpButtonClicked()
{
    const FString JoinAddress = JoinAddressTextBox ? JoinAddressTextBox->GetText().ToString() : FString();
    const FString TravelCommand = BuildJoinTravelCommand(JoinAddress);
    if (TravelCommand.IsEmpty())
    {
        SetConnectionStatus(TEXT("Network: Enter a host IP address"));
        return;
    }

    const int32 AddressStartIndex = TravelCommand.Find(TEXT(" "));
    const FString PendingAddress = AddressStartIndex != INDEX_NONE
        ? TravelCommand.Mid(AddressStartIndex + 1)
        : TravelCommand;
    ExecuteTravelCommand(TravelCommand, FString::Printf(TEXT("Network: Joining %s"), *PendingAddress));
}

void UPvPArenaHUDWidget::HandleLobbyNicknameTextCommitted(const FText& NewText, ETextCommit::Type CommitMethod)
{
    if (bUpdatingLobbyNicknameText || CommitMethod == ETextCommit::Default)
    {
        return;
    }

    APvPArenaPlayerController* PvPPlayerController = Cast<APvPArenaPlayerController>(GetOwningPlayer());
    if (!PvPPlayerController)
    {
        return;
    }

    PvPPlayerController->SubmitLobbyNickname(NewText.ToString());
}

FString UPvPArenaHUDWidget::BuildHostTravelCommand() const
{
    return FString::Printf(TEXT("open %s"), HostTravelMapPath);
}

FString UPvPArenaHUDWidget::BuildJoinTravelCommand(const FString& JoinAddress) const
{
    FString CleanAddress = JoinAddress;
    CleanAddress.TrimStartAndEndInline();
    if (CleanAddress.IsEmpty())
    {
        return FString();
    }

    if (CleanAddress.StartsWith(TEXT("open ")))
    {
        return CleanAddress;
    }

    if (!CleanAddress.Contains(TEXT(":")))
    {
        CleanAddress += TEXT(":7777");
    }

    return FString::Printf(TEXT("open %s"), *CleanAddress);
}

void UPvPArenaHUDWidget::ToggleSettingsMenu()
{
    bSettingsMenuOpen = !bSettingsMenuOpen;
    RefreshSettingsMenuState();

    APlayerController* PlayerController = GetOwningPlayer();
    const APvPArenaGameState* PvPGameState = GetWorld() ? GetWorld()->GetGameState<APvPArenaGameState>() : nullptr;
    const bool bIsLobby = PvPGameState && PvPGameState->GetMatchPhase() == EPvPAMatchPhase::Lobby;
    ApplyLobbyInputMode(PlayerController, bSettingsMenuOpen || bIsLobby);
}

void UPvPArenaHUDWidget::HandleSettingsResumeButtonClicked()
{
    if (!bSettingsMenuOpen)
    {
        return;
    }

    ToggleSettingsMenu();
}

void UPvPArenaHUDWidget::HandleSettingsQuitButtonClicked()
{
    APlayerController* PlayerController = GetOwningPlayer();
    if (!PlayerController)
    {
        return;
    }

    UKismetSystemLibrary::QuitGame(this, PlayerController, EQuitPreference::Quit, false);
}

void UPvPArenaHUDWidget::HandleSettingsWindowModeButtonClicked()
{
    ApplyWindowModeIndex((SelectedWindowModeIndex + 1) % 3);
    ApplyDisplaySettings();
}

void UPvPArenaHUDWidget::HandleSettingsResolutionButtonClicked()
{
    const TArray<FIntPoint> SupportedResolutions = BuildSupportedResolutions();
    if (SupportedResolutions.IsEmpty())
    {
        return;
    }

    ApplyResolutionIndex((SelectedResolutionIndex + 1) % SupportedResolutions.Num());
    ApplyDisplaySettings();
}

void UPvPArenaHUDWidget::HandleSettingsVSyncButtonClicked()
{
    bVSyncEnabled = !bVSyncEnabled;
    ApplyDisplaySettings();
}

void UPvPArenaHUDWidget::HandleMasterVolumeSliderChanged(float NewValue)
{
    MasterVolume = FMath::Clamp(NewValue, 0.0f, 1.0f);
    ApplyAudioSettings();
}

void UPvPArenaHUDWidget::HandleBgmVolumeSliderChanged(float NewValue)
{
    BackgroundMusicVolume = FMath::Clamp(NewValue, 0.0f, 1.0f);
    ApplyAudioSettings();
}

void UPvPArenaHUDWidget::HandleSfxVolumeSliderChanged(float NewValue)
{
    SfxVolume = FMath::Clamp(NewValue, 0.0f, 1.0f);
    ApplyAudioSettings();
}

void UPvPArenaHUDWidget::ApplyDisplaySettings()
{
    UGameUserSettings* GameUserSettings = UGameUserSettings::GetGameUserSettings();
    const TArray<FIntPoint> SupportedResolutions = BuildSupportedResolutions();
    if (!GameUserSettings || !SupportedResolutions.IsValidIndex(SelectedResolutionIndex))
    {
        return;
    }

    GameUserSettings->SetScreenResolution(SupportedResolutions[SelectedResolutionIndex]);
    GameUserSettings->SetFullscreenMode(static_cast<EWindowMode::Type>(SelectedWindowModeIndex));
    GameUserSettings->SetVSyncEnabled(bVSyncEnabled);
    GameUserSettings->ApplySettings(false);
    GameUserSettings->SaveSettings();
    RefreshSettingsMenuState();
}

void UPvPArenaHUDWidget::ApplyAudioSettings()
{
    if (BackgroundMusicAudioComponent)
    {
        BackgroundMusicAudioComponent->SetVolumeMultiplier(MasterVolume * BackgroundMusicVolume);
    }

    const APlayerController* PlayerController = GetOwningPlayer();
    APvPArenaCharacter* Character = PlayerController ? Cast<APvPArenaCharacter>(PlayerController->GetPawn()) : nullptr;
    if (Character)
    {
        Character->ApplyAudioSettings(MasterVolume, SfxVolume);
    }

    RefreshSettingsMenuState();
}

void UPvPArenaHUDWidget::RefreshSettingsMenuState()
{
    if (SettingsPanel)
    {
        SettingsPanel->SetVisibility(bSettingsMenuOpen ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }

    if (SettingsMasterVolumeSlider && !FMath::IsNearlyEqual(SettingsMasterVolumeSlider->GetValue(), MasterVolume))
    {
        SettingsMasterVolumeSlider->SetValue(MasterVolume);
    }

    if (SettingsBgmVolumeSlider && !FMath::IsNearlyEqual(SettingsBgmVolumeSlider->GetValue(), BackgroundMusicVolume))
    {
        SettingsBgmVolumeSlider->SetValue(BackgroundMusicVolume);
    }

    if (SettingsSfxVolumeSlider && !FMath::IsNearlyEqual(SettingsSfxVolumeSlider->GetValue(), SfxVolume))
    {
        SettingsSfxVolumeSlider->SetValue(SfxVolume);
    }

    if (SettingsMasterVolumeText)
    {
        SettingsMasterVolumeText->SetText(FText::FromString(FString::Printf(TEXT("Master Volume: %d%%"), FMath::RoundToInt(MasterVolume * 100.0f))));
    }

    if (SettingsBgmVolumeText)
    {
        SettingsBgmVolumeText->SetText(FText::FromString(FString::Printf(TEXT("BGM Volume: %d%%"), FMath::RoundToInt(BackgroundMusicVolume * 100.0f))));
    }

    if (SettingsSfxVolumeText)
    {
        SettingsSfxVolumeText->SetText(FText::FromString(FString::Printf(TEXT("SFX Volume: %d%%"), FMath::RoundToInt(SfxVolume * 100.0f))));
    }

    if (SettingsWindowModeButtonText)
    {
        SettingsWindowModeButtonText->SetText(FText::FromString(FString::Printf(TEXT("Window Mode: %s"), *BuildWindowModeLabel(SelectedWindowModeIndex))));
    }

    const TArray<FIntPoint> SupportedResolutions = BuildSupportedResolutions();
    if (SettingsResolutionButtonText && SupportedResolutions.IsValidIndex(SelectedResolutionIndex))
    {
        SettingsResolutionButtonText->SetText(FText::FromString(FString::Printf(TEXT("Resolution: %s"), *BuildResolutionLabel(SupportedResolutions[SelectedResolutionIndex]))));
    }

    if (SettingsVSyncButtonText)
    {
        SettingsVSyncButtonText->SetText(FText::FromString(FString::Printf(TEXT("VSync: %s"), bVSyncEnabled ? TEXT("On") : TEXT("Off"))));
    }
}

void UPvPArenaHUDWidget::ApplyResolutionIndex(int32 NewResolutionIndex)
{
    const TArray<FIntPoint> SupportedResolutions = BuildSupportedResolutions();
    if (!SupportedResolutions.IsValidIndex(NewResolutionIndex))
    {
        return;
    }

    SelectedResolutionIndex = NewResolutionIndex;
    RefreshSettingsMenuState();
}

void UPvPArenaHUDWidget::ApplyWindowModeIndex(int32 NewWindowModeIndex)
{
    SelectedWindowModeIndex = FMath::Clamp(NewWindowModeIndex, 0, 2);
    RefreshSettingsMenuState();
}

FString UPvPArenaHUDWidget::BuildWindowModeLabel(int32 WindowModeIndex)
{
    switch (static_cast<EWindowMode::Type>(WindowModeIndex))
    {
    case EWindowMode::Fullscreen:
        return TEXT("Fullscreen");
    case EWindowMode::WindowedFullscreen:
        return TEXT("Borderless");
    case EWindowMode::Windowed:
    default:
        return TEXT("Windowed");
    }
}

FString UPvPArenaHUDWidget::BuildResolutionLabel(const FIntPoint& Resolution)
{
    return FString::Printf(TEXT("%d x %d"), Resolution.X, Resolution.Y);
}

TArray<FIntPoint> UPvPArenaHUDWidget::BuildSupportedResolutions()
{
    TArray<FIntPoint> Resolutions;
    Resolutions.Add(DefaultWindowedResolution);
    Resolutions.Add(FIntPoint(1600, 900));
    Resolutions.Add(FIntPoint(1920, 1080));

    const FIntPoint DesktopResolution = UGameUserSettings::GetGameUserSettings()
        ? UGameUserSettings::GetGameUserSettings()->GetDesktopResolution()
        : FIntPoint::ZeroValue;
    if (DesktopResolution.X > 0 && DesktopResolution.Y > 0)
    {
        Resolutions.AddUnique(DesktopResolution);
    }

    return Resolutions;
}

void UPvPArenaHUDWidget::SetConnectionStatus(const FString& NewStatus)
{
    if (ConnectionStatusText)
    {
        ConnectionStatusText->SetText(FText::FromString(NewStatus));
    }
}

bool UPvPArenaHUDWidget::ExecuteTravelCommand(const FString& TravelCommand, const FString& PendingStatus)
{
    APlayerController* PlayerController = GetOwningPlayer();
    if (!PlayerController)
    {
        SetConnectionStatus(TEXT("Network: Player controller unavailable"));
        return false;
    }

    if (TravelCommand.IsEmpty())
    {
        SetConnectionStatus(TEXT("Network: Invalid travel command"));
        return false;
    }

    PlayerController->ConsoleCommand(TravelCommand, true);
    SetConnectionStatus(PendingStatus);
    return true;
}

void UPvPArenaHUDWidget::RefreshCrosshairVisibility()
{
    if (!RangedCrosshairOverlay)
    {
        return;
    }

    const APlayerController* PlayerController = GetOwningPlayer();
    const APvPArenaCharacter* Character = PlayerController ? Cast<APvPArenaCharacter>(PlayerController->GetPawn()) : nullptr;
    const APvPArenaGameState* GameState = GetWorld() ? GetWorld()->GetGameState<APvPArenaGameState>() : nullptr;
    RangedCrosshairOverlay->SetVisibility(BuildRangedCrosshairVisibilityState(Character, GameState));
    RangedCrosshairOverlay->SetRenderTranslation(FVector2D(
        0.0f,
        BuildRangedCrosshairVerticalOffsetState(Character, GameState)));
}

void UPvPArenaHUDWidget::RefreshLobbyParticipantList(const APvPArenaGameState* GameState)
{
    if (!LobbyPlayerListBox || !WidgetTree)
    {
        return;
    }

    LobbyPlayerListBox->ClearChildren();

    for (const FString& ParticipantLabel : BuildLobbyParticipantLabels(GameState))
    {
        UTextBlock* ParticipantText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
        if (!ParticipantText)
        {
            continue;
        }

        ParticipantText->SetText(FText::FromString(ParticipantLabel));
        ParticipantText->SetColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.96f, 1.0f, 1.0f)));
        ParticipantText->SetFont(FSlateFontInfo(ParticipantText->GetFont().FontObject, 16, ParticipantText->GetFont().TypefaceFontName));
        ParticipantText->SetShadowOffset(FVector2D(1.0f, 1.0f));

        UVerticalBoxSlot* ParticipantSlot = LobbyPlayerListBox->AddChildToVerticalBox(ParticipantText);
        if (ParticipantSlot)
        {
            ParticipantSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 4.0f));
        }
    }
}

FString UPvPArenaHUDWidget::GetRoundResultText(const APlayerController* PlayerController, const APvPArenaGameState* GameState) const
{
    return BuildRoundResultLabel(
        PlayerController ? PlayerController->GetPlayerState<APvPArenaPlayerState>() : nullptr,
        GameState);
}

bool UPvPArenaHUDWidget::ShouldShowLobbyStartButton(const APlayerController* PlayerController, const APvPArenaGameState* GameState) const
{
    if (!PlayerController || !GameState || GameState->GetMatchPhase() != EPvPAMatchPhase::Lobby)
    {
        return false;
    }

    int32 ConnectedPlayers = 0;
    int32 LeftTeamPlayers = 0;
    int32 RightTeamPlayers = 0;
    EPvPALobbyMatchMode LobbyMatchMode = EPvPALobbyMatchMode::FreeForAll;
    for (APlayerState* PlayerState : GameState->PlayerArray)
    {
        if (const APvPArenaPlayerState* PvPPlayerState = Cast<APvPArenaPlayerState>(PlayerState))
        {
            ++ConnectedPlayers;
            LobbyMatchMode = PvPPlayerState->GetLobbyMatchMode();
            LeftTeamPlayers += PvPPlayerState->GetLobbyTeam() == EPvPALobbyTeam::Left ? 1 : 0;
            RightTeamPlayers += PvPPlayerState->GetLobbyTeam() == EPvPALobbyTeam::Right ? 1 : 0;
        }
    }

    if (!PlayerController->HasAuthority() || ConnectedPlayers < 2)
    {
        return false;
    }

    return LobbyMatchMode == EPvPALobbyMatchMode::FreeForAll || (LeftTeamPlayers > 0 && RightTeamPlayers > 0);
}

FString UPvPArenaHUDWidget::BuildLobbyTeamListText(const APvPArenaGameState* GameState, uint8 LobbyTeamValue) const
{
    const EPvPALobbyTeam LobbyTeam = static_cast<EPvPALobbyTeam>(LobbyTeamValue);
    TArray<FString> TeamPlayers;

    if (GameState)
    {
        for (APlayerState* PlayerState : GameState->PlayerArray)
        {
            const APvPArenaPlayerState* PvPPlayerState = Cast<APvPArenaPlayerState>(PlayerState);
            if (!PvPPlayerState || PvPPlayerState->GetLobbyTeam() != LobbyTeam)
            {
                continue;
            }

            const FString PreferredLabel = !PvPPlayerState->GetDisplayNickname().IsEmpty()
                ? PvPPlayerState->GetDisplayNickname()
                : (!PvPPlayerState->GetPlayerName().IsEmpty() ? PvPPlayerState->GetPlayerName() : TEXT("Player"));
            TeamPlayers.Add(PreferredLabel);
        }
    }

    const FString TeamLabel = LobbyTeam == EPvPALobbyTeam::Left ? TEXT("Left Team") : TEXT("Right Team");
    return TeamPlayers.IsEmpty()
        ? FString::Printf(TEXT("%s\n- Empty -"), *TeamLabel)
        : FString::Printf(TEXT("%s\n%s"), *TeamLabel, *FString::Join(TeamPlayers, TEXT("\n")));
}

FString UPvPArenaHUDWidget::LobbyMatchModeToString(uint8 LobbyMatchModeValue)
{
    return static_cast<EPvPALobbyMatchMode>(LobbyMatchModeValue) == EPvPALobbyMatchMode::TeamVersus
        ? TEXT("Team Versus")
        : TEXT("Free For All");
}

FString UPvPArenaHUDWidget::LobbyTeamToString(uint8 LobbyTeamValue)
{
    switch (static_cast<EPvPALobbyTeam>(LobbyTeamValue))
    {
    case EPvPALobbyTeam::Left:
        return TEXT("Left");
    case EPvPALobbyTeam::Right:
        return TEXT("Right");
    default:
        return TEXT("None");
    }
}

FString UPvPArenaHUDWidget::GetLobbyStatusText(const APlayerController* PlayerController, const APvPArenaGameState* GameState) const
{
    int32 ConnectedPlayers = 0;
    int32 LeftTeamPlayers = 0;
    int32 RightTeamPlayers = 0;
    EPvPALobbyMatchMode LobbyMatchMode = EPvPALobbyMatchMode::FreeForAll;
    if (GameState)
    {
        for (APlayerState* PlayerState : GameState->PlayerArray)
        {
            const APvPArenaPlayerState* PvPPlayerState = Cast<APvPArenaPlayerState>(PlayerState);
            if (!PvPPlayerState)
            {
                continue;
            }

            ++ConnectedPlayers;
            LobbyMatchMode = PvPPlayerState->GetLobbyMatchMode();
            LeftTeamPlayers += PvPPlayerState->GetLobbyTeam() == EPvPALobbyTeam::Left ? 1 : 0;
            RightTeamPlayers += PvPPlayerState->GetLobbyTeam() == EPvPALobbyTeam::Right ? 1 : 0;
        }
    }

    if (ConnectedPlayers < 2)
    {
        if (LobbyMatchMode == EPvPALobbyMatchMode::TeamVersus)
        {
            return FString::Printf(
                TEXT("Players Connected: %d / %d\nMode: Team Versus\nNeed %d players connected to begin.\nFirst to %d round wins takes the match."),
                ConnectedPlayers,
                6,
                2,
                GameState ? GameState->GetRoundWinsToWin() : 3);
        }

        return FString::Printf(
            TEXT("Players Connected: %d / %d\nMode: Free For All\nNeed %d players connected to begin.\nFirst to %d kills or best score at time wins."),
            ConnectedPlayers,
            6,
            2,
            GameState ? GameState->GetScoreLimit() : 5);
    }

    if (LobbyMatchMode == EPvPALobbyMatchMode::TeamVersus && (LeftTeamPlayers == 0 || RightTeamPlayers == 0))
    {
        return FString::Printf(
            TEXT("Players Connected: %d / %d\nMode: Team Versus\nLeft: %d  Right: %d\nNeed at least 1 player on each team to begin."),
            ConnectedPlayers,
            6,
            LeftTeamPlayers,
            RightTeamPlayers);
    }

    if (PlayerController && PlayerController->HasAuthority())
    {
        return FString::Printf(
            TEXT("Players Connected: %d / %d\nMode: Team Versus\nPress Start Match when ready.\nFirst to %d round wins takes the match."),
            ConnectedPlayers,
            6,
            GameState ? GameState->GetRoundWinsToWin() : 3);
    }

    if (LobbyMatchMode == EPvPALobbyMatchMode::TeamVersus)
    {
        return FString::Printf(
            TEXT("Players Connected: %d / %d\nMode: Team Versus\nWaiting for the host to start the match.\nFirst to %d round wins takes the match."),
            ConnectedPlayers,
            6,
            GameState ? GameState->GetRoundWinsToWin() : 3);
    }

    return FString::Printf(
        TEXT("Players Connected: %d / %d\nMode: Free For All\nWaiting for the host to start the match.\nFirst to %d kills or best score at time wins."),
        ConnectedPlayers,
        6,
        GameState ? GameState->GetScoreLimit() : 5);
}

FString UPvPArenaHUDWidget::GetMatchResultText(const APlayerController* PlayerController, const APvPArenaGameState* GameState) const
{
    return BuildMatchResultLabel(
        PlayerController ? PlayerController->GetPlayerState<APvPArenaPlayerState>() : nullptr,
        GameState);
}

FString UPvPArenaHUDWidget::GetMatchSummaryText(const APlayerController* PlayerController, const APvPArenaGameState* GameState) const
{
    if (!PlayerController || !GameState)
    {
        return TEXT("Returning to lobby...");
    }

    const APvPArenaPlayerState* LocalPlayerState = PlayerController->GetPlayerState<APvPArenaPlayerState>();
    if (!LocalPlayerState)
    {
        return TEXT("Returning to lobby...");
    }

    int32 HighestOpponentRoundWins = 0;
    for (APlayerState* PlayerState : GameState->PlayerArray)
    {
        const APvPArenaPlayerState* PvPState = Cast<APvPArenaPlayerState>(PlayerState);
        if (!PvPState || PvPState == LocalPlayerState)
        {
            continue;
        }

        HighestOpponentRoundWins = FMath::Max(HighestOpponentRoundWins, PvPState->GetRoundWins());
    }

    return FString::Printf(
        TEXT("Final Rounds: %d - %d\nK / D: %d / %d\nReturning to lobby in: %d"),
        LocalPlayerState->GetRoundWins(),
        HighestOpponentRoundWins,
        LocalPlayerState->GetMatchKills(),
        LocalPlayerState->GetMatchDeaths(),
        GameState->GetRemainingMatchEndTimeSeconds());
}

FString UPvPArenaHUDWidget::RoundStateToString(uint8 RoundStateValue)
{
    switch (static_cast<EPvPARoundState>(RoundStateValue))
    {
    case EPvPARoundState::Playing:
        return TEXT("Playing");
    case EPvPARoundState::RoundEnd:
        return TEXT("RoundEnd");
    case EPvPARoundState::SuddenDeath:
        return TEXT("SuddenDeath");
    default:
        return TEXT("Unknown");
    }
}

FString UPvPArenaHUDWidget::MatchPhaseToString(uint8 MatchPhaseValue)
{
    switch (static_cast<EPvPAMatchPhase>(MatchPhaseValue))
    {
    case EPvPAMatchPhase::Lobby:
        return TEXT("Lobby");
    case EPvPAMatchPhase::Playing:
        return TEXT("Playing");
    case EPvPAMatchPhase::MatchEnd:
        return TEXT("MatchEnd");
    default:
        return TEXT("Unknown");
    }
}
