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
    RoundScoreText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RoundScoreText"));
    MatchScoreText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("MatchScoreText"));
    TimerText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TimerText"));
    RoundStateText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RoundStateText"));
    ResultText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ResultText"));
    NextRoundText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("NextRoundText"));
    LobbyTitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LobbyTitleText"));
    LobbyStatusText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LobbyStatusText"));
    LobbyReadyButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("LobbyReadyButton"));
    LobbyReadyButtonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LobbyReadyButtonText"));
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

    if (!RootOverlay || !StatusPanel || !CountdownPanel || !InfoPanel || !AnnouncementPanel || !LobbyPanel || !LobbyControlsPanel || !MatchResultPanel || !SettingsPanel || !StatusCardsBox || !CountdownBox || !InfoBox || !AnnouncementBox || !LobbyBox || !LobbyControlsBox || !LobbyControlsCardsBox || !MatchResultBox || !SettingsBox || !LobbyKeyboardCard || !LobbyMouseCard || !HealthCard || !SprintCard || !RangedCard || !LobbyKeyboardCardBox || !LobbyMouseCardBox || !HealthCardBox || !SprintCardBox || !RangedCardBox || !HealthBarSizeBox || !HealthBar || !SprintBarSizeBox || !SprintBar || !RangedCooldownBarSizeBox || !RangedCooldownBar || !HealthText || !SprintText || !RangedCooldownText || !RoundScoreText || !MatchScoreText || !TimerText || !RoundStateText || !ResultText || !NextRoundText || !LobbyTitleText || !LobbyStatusText || !LobbyReadyButton || !LobbyReadyButtonText || !LobbyControlsTitleText || !LobbyControlsKeyboardTitleText || !LobbyControlsKeyboardMoveText || !LobbyControlsKeyboardSprintText || !LobbyControlsMouseTitleText || !LobbyControlsMouseMeleeText || !LobbyControlsMouseRangedText || !MatchResultTitleText || !MatchResultSummaryText || !ConnectionStatusText || !LobbyNicknameTextBox || !LobbyPlayerListBox || !SettingsTitleText || !SettingsDisplayModeLabelText || !SettingsResolutionLabelText || !SettingsAudioLabelText || !SettingsMasterVolumeText || !SettingsBgmVolumeText || !SettingsSfxVolumeText || !SettingsWindowModeButtonText || !SettingsResolutionButtonText || !SettingsVSyncButtonText || !SettingsResumeButtonText || !SettingsQuitButtonText || !JoinAddressTextBox || !HostMatchButton || !HostMatchButtonText || !JoinByIpButton || !JoinByIpButtonText || !SettingsMasterVolumeSlider || !SettingsBgmVolumeSlider || !SettingsSfxVolumeSlider || !SettingsWindowModeButton || !SettingsResolutionButton || !SettingsVSyncButton || !SettingsResumeButton || !SettingsQuitButton || !RangedCrosshairOverlay || !RangedCrosshairHorizontalBox || !RangedCrosshairVerticalBox || !RangedCrosshairHorizontalLine || !RangedCrosshairVerticalLine)
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

    UOverlaySlot* StatusBoxSlot = RootOverlay->AddChildToOverlay(StatusPanel);
    UOverlaySlot* CountdownBoxSlot = RootOverlay->AddChildToOverlay(CountdownPanel);
    UOverlaySlot* InfoBoxSlot = RootOverlay->AddChildToOverlay(InfoPanel);
    UOverlaySlot* AnnouncementBoxSlot = RootOverlay->AddChildToOverlay(AnnouncementPanel);
    UOverlaySlot* LobbyBoxSlot = RootOverlay->AddChildToOverlay(LobbyPanel);
    UOverlaySlot* LobbyControlsBoxSlot = RootOverlay->AddChildToOverlay(LobbyControlsPanel);
    UOverlaySlot* MatchResultBoxSlot = RootOverlay->AddChildToOverlay(MatchResultPanel);
    UOverlaySlot* SettingsBoxSlot = RootOverlay->AddChildToOverlay(SettingsPanel);
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
    UVerticalBoxSlot* RoundScoreTextSlot = InfoBox->AddChildToVerticalBox(RoundScoreText);
    UVerticalBoxSlot* MatchScoreTextSlot = InfoBox->AddChildToVerticalBox(MatchScoreText);
    UVerticalBoxSlot* RoundStateTextSlot = InfoBox->AddChildToVerticalBox(RoundStateText);
    UVerticalBoxSlot* ResultTextSlot = AnnouncementBox->AddChildToVerticalBox(ResultText);
    UVerticalBoxSlot* NextRoundTextSlot = AnnouncementBox->AddChildToVerticalBox(NextRoundText);
    UVerticalBoxSlot* LobbyTitleTextSlot = LobbyBox->AddChildToVerticalBox(LobbyTitleText);
    UVerticalBoxSlot* LobbyStatusTextSlot = LobbyBox->AddChildToVerticalBox(LobbyStatusText);
    UVerticalBoxSlot* ConnectionStatusTextSlot = LobbyBox->AddChildToVerticalBox(ConnectionStatusText);
    UVerticalBoxSlot* LobbyNicknameTextBoxSlot = LobbyBox->AddChildToVerticalBox(LobbyNicknameTextBox);
    UVerticalBoxSlot* LobbyPlayerListBoxSlot = LobbyBox->AddChildToVerticalBox(LobbyPlayerListBox);
    UVerticalBoxSlot* JoinAddressTextBoxSlot = LobbyBox->AddChildToVerticalBox(JoinAddressTextBox);
    UVerticalBoxSlot* HostMatchButtonSlot = LobbyBox->AddChildToVerticalBox(HostMatchButton);
    UVerticalBoxSlot* JoinByIpButtonSlot = LobbyBox->AddChildToVerticalBox(JoinByIpButton);
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

    if (RoundScoreTextSlot)
    {
        RoundScoreTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 6.0f));
    }
    RoundScoreText->SetColorAndOpacity(FSlateColor(FLinearColor::Yellow));
    RoundScoreText->SetFont(FSlateFontInfo(RoundScoreText->GetFont().FontObject, 18, RoundScoreText->GetFont().TypefaceFontName));
    RoundScoreText->SetShadowOffset(FVector2D(1.0f, 1.0f));

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
    UVerticalBoxSlot* LobbyReadyButtonSlot = LobbyBox->AddChildToVerticalBox(LobbyReadyButton);
    if (LobbyReadyButtonSlot)
    {
        LobbyReadyButtonSlot->SetPadding(FMargin(0.0f, 18.0f, 0.0f, 0.0f));
        LobbyReadyButtonSlot->SetHorizontalAlignment(HAlign_Center);
    }

    LobbyReadyButtonText->SetJustification(ETextJustify::Center);
    LobbyReadyButtonText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
    LobbyReadyButtonText->SetFont(FSlateFontInfo(LobbyReadyButtonText->GetFont().FontObject, 22, LobbyReadyButtonText->GetFont().TypefaceFontName));
    LobbyReadyButtonText->SetText(FText::FromString(TEXT("Start Match")));

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
        if (RoundScoreText)
        {
            RoundScoreText->SetText(FText::FromString(
                FString::Printf(TEXT("Round K / D: %d / %d"), LocalPlayerState->GetRoundKills(), LocalPlayerState->GetRoundDeaths())));
        }

        if (MatchScoreText)
        {
            MatchScoreText->SetText(FText::FromString(
                FString::Printf(
                    TEXT("Rounds: %d / %d | Match K / D: %d / %d"),
                    LocalPlayerState->GetRoundWins(),
                    PvPGameState ? PvPGameState->GetRoundWinsToWin() : 2,
                    LocalPlayerState->GetMatchKills(),
                    LocalPlayerState->GetMatchDeaths())));
        }
    }

    if (PvPGameState)
    {
        const bool bIsRoundEnd = PvPGameState->GetRoundState() == EPvPARoundState::RoundEnd
            && PvPGameState->GetMatchPhase() == EPvPAMatchPhase::Playing;
        const bool bIsLobby = PvPGameState->GetMatchPhase() == EPvPAMatchPhase::Lobby;
        const bool bIsMatchEnd = PvPGameState->GetMatchPhase() == EPvPAMatchPhase::MatchEnd;

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
    if (!PlayerController || !GameState)
    {
        return TEXT("Unknown");
    }

    const APvPArenaPlayerState* LocalPlayerState = PlayerController->GetPlayerState<APvPArenaPlayerState>();
    if (!LocalPlayerState)
    {
        return TEXT("Unknown");
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

bool UPvPArenaHUDWidget::ShouldShowLobbyStartButton(const APlayerController* PlayerController, const APvPArenaGameState* GameState) const
{
    if (!PlayerController || !GameState || GameState->GetMatchPhase() != EPvPAMatchPhase::Lobby)
    {
        return false;
    }

    int32 ConnectedPlayers = 0;
    for (APlayerState* PlayerState : GameState->PlayerArray)
    {
        if (Cast<APvPArenaPlayerState>(PlayerState))
        {
            ++ConnectedPlayers;
        }
    }

    return PlayerController->HasAuthority() && ConnectedPlayers >= 2;
}

FString UPvPArenaHUDWidget::GetLobbyStatusText(const APlayerController* PlayerController, const APvPArenaGameState* GameState) const
{
    int32 ConnectedPlayers = 0;
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
        }
    }

    if (ConnectedPlayers < 2)
    {
        return FString::Printf(
            TEXT("Players Connected: %d / %d\nNeed %d players connected to begin.\nFirst to %d round wins takes the match."),
            ConnectedPlayers,
            ConnectedPlayers,
            2,
            GameState ? GameState->GetRoundWinsToWin() : 2);
    }

    if (PlayerController && PlayerController->HasAuthority())
    {
        return FString::Printf(
            TEXT("Players Connected: %d / %d\nAll players are in. Press Start Match when ready.\nFirst to %d round wins takes the match."),
            ConnectedPlayers,
            ConnectedPlayers,
            GameState ? GameState->GetRoundWinsToWin() : 2);
    }

    return FString::Printf(
        TEXT("Players Connected: %d / %d\nWaiting for the host to start the match.\nFirst to %d round wins takes the match."),
        ConnectedPlayers,
        ConnectedPlayers,
        GameState ? GameState->GetRoundWinsToWin() : 2);
}

FString UPvPArenaHUDWidget::GetMatchResultText(const APlayerController* PlayerController, const APvPArenaGameState* GameState) const
{
    if (!PlayerController || !GameState)
    {
        return TEXT("Match Complete");
    }

    const APvPArenaPlayerState* LocalPlayerState = PlayerController->GetPlayerState<APvPArenaPlayerState>();
    const APvPArenaPlayerState* MatchWinner = GameState->GetMatchWinner();
    if (!LocalPlayerState || !MatchWinner)
    {
        return TEXT("Match Complete");
    }

    return MatchWinner == LocalPlayerState ? TEXT("Final Victory") : TEXT("Final Defeat");
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
