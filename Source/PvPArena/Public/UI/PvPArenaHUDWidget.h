#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Game/PvPArenaPlayerState.h"
#include "Types/SlateEnums.h"
#include "PvPArenaHUDWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UButton;
class UEditableTextBox;
class UAudioComponent;
class UOverlay;
class UBorder;
class USizeBox;
class UHorizontalBox;
class UVerticalBox;
class USlider;
class USoundBase;
enum class ESlateVisibility : uint8;
struct FGeometry;
class UPvPCombatComponent;

UCLASS()
class PVPARENA_API UPvPArenaHUDWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UPvPArenaHUDWidget(const FObjectInitializer& ObjectInitializer);
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    static void BuildHealthDisplayState(const class APvPArenaCharacter* Character, float& OutHealthPercent, FString& OutHealthLabel);
    static void BuildSprintDisplayState(const class APvPArenaCharacter* Character, float& OutSprintPercent, FString& OutSprintLabel);
    static void BuildRangedCooldownDisplayState(const UPvPCombatComponent* CombatComponent, float NowSeconds, float& OutCooldownPercent, FString& OutCooldownLabel);
    static ESlateVisibility BuildRangedCrosshairVisibilityState(
        const class APvPArenaCharacter* Character,
        const class APvPArenaGameState* GameState = nullptr);
    static float BuildRangedCrosshairVerticalOffsetState(
        const class APvPArenaCharacter* Character,
        const class APvPArenaGameState* GameState = nullptr);
    static TArray<FString> BuildLobbyParticipantLabels(const class APvPArenaGameState* GameState);
    static FString BuildLobbyNicknameTextBoxValue(const FString& DraftNickname, const FString& ReplicatedNickname);
    static FString BuildBackgroundMusicAssetPathForMatchPhase(uint8 MatchPhaseValue);
    static FString BuildMatchScoreSummary(
        const class APvPArenaPlayerState* LocalPlayerState,
        const class APvPArenaGameState* GameState);
    static FString BuildRoundResultLabel(
        const class APvPArenaPlayerState* LocalPlayerState,
        const class APvPArenaGameState* GameState);
    static FString BuildMatchResultLabel(
        const class APvPArenaPlayerState* LocalPlayerState,
        const class APvPArenaGameState* GameState);

    UFUNCTION()
    void ToggleSettingsMenu();

private:
    UFUNCTION()
    void HandleLobbyReadyButtonClicked();

    UFUNCTION()
    void HandleLobbyFreeForAllModeButtonClicked();

    UFUNCTION()
    void HandleLobbyTeamVersusModeButtonClicked();

    UFUNCTION()
    void HandleLobbyLeftTeamButtonClicked();

    UFUNCTION()
    void HandleLobbyRightTeamButtonClicked();

    bool ShouldShowLobbyStartButton(const class APlayerController* PlayerController, const class APvPArenaGameState* GameState) const;

    UFUNCTION()
    void HandleHostMatchButtonClicked();

    UFUNCTION()
    void HandleJoinByIpButtonClicked();

    UFUNCTION()
    void HandleLobbyNicknameTextCommitted(const FText& NewText, ETextCommit::Type CommitMethod);

    UFUNCTION()
    FString BuildHostTravelCommand() const;

    UFUNCTION()
    FString BuildJoinTravelCommand(const FString& JoinAddress) const;

    UFUNCTION()
    void HandleSettingsResumeButtonClicked();

    UFUNCTION()
    void HandleSettingsQuitButtonClicked();

    UFUNCTION()
    void HandleSettingsWindowModeButtonClicked();

    UFUNCTION()
    void HandleSettingsResolutionButtonClicked();

    UFUNCTION()
    void HandleSettingsVSyncButtonClicked();

    UFUNCTION()
    void HandleMasterVolumeSliderChanged(float NewValue);

    UFUNCTION()
    void HandleBgmVolumeSliderChanged(float NewValue);

    UFUNCTION()
    void HandleSfxVolumeSliderChanged(float NewValue);

    UFUNCTION()
    void ApplyDisplaySettings();

    UFUNCTION()
    void ApplyAudioSettings();

    void BuildWidgetTree();
    void RefreshWidgetData();
    void RefreshLobbyParticipantList(const class APvPArenaGameState* GameState);
    void RefreshCrosshairVisibility();
    void RefreshBackgroundMusic(const class APvPArenaGameState* GameState);
    void ApplyLobbyInputMode(class APlayerController* PlayerController, bool bEnableLobbyInput);
    void SetConnectionStatus(const FString& NewStatus);
    bool ExecuteTravelCommand(const FString& TravelCommand, const FString& PendingStatus);
    FString BuildLobbyTeamListText(const class APvPArenaGameState* GameState, uint8 LobbyTeamValue) const;
    static FString LobbyMatchModeToString(uint8 LobbyMatchModeValue);
    static FString LobbyTeamToString(uint8 LobbyTeamValue);
    FString GetLobbyStatusText(const class APlayerController* PlayerController, const class APvPArenaGameState* GameState) const;
    FString GetRoundResultText(const class APlayerController* PlayerController, const class APvPArenaGameState* GameState) const;
    FString GetMatchResultText(const class APlayerController* PlayerController, const class APvPArenaGameState* GameState) const;
    FString GetMatchSummaryText(const class APlayerController* PlayerController, const class APvPArenaGameState* GameState) const;
    void RefreshSettingsMenuState();
    void ApplyResolutionIndex(int32 NewResolutionIndex);
    void ApplyWindowModeIndex(int32 NewWindowModeIndex);
    static FString BuildWindowModeLabel(int32 WindowModeIndex);
    static FString BuildResolutionLabel(const FIntPoint& Resolution);
    static TArray<FIntPoint> BuildSupportedResolutions();
    static FString LobbyTeamToDisplayName(EPvPALobbyTeam LobbyTeamValue);
    static int32 BuildTeamRoundWins(const class APvPArenaGameState* GameState, EPvPALobbyTeam LobbyTeam);
    static FString MatchPhaseToString(uint8 MatchPhaseValue);
    static FString RoundStateToString(uint8 RoundStateValue);

    UPROPERTY(Transient)
    TObjectPtr<UOverlay> RootOverlay;

    UPROPERTY(Transient)
    TObjectPtr<UBorder> StatusPanel;

    UPROPERTY(Transient)
    TObjectPtr<UBorder> CountdownPanel;

    UPROPERTY(Transient)
    TObjectPtr<UBorder> InfoPanel;

    UPROPERTY(Transient)
    TObjectPtr<UBorder> AnnouncementPanel;

    UPROPERTY(Transient)
    TObjectPtr<UBorder> LobbyPanel;

    UPROPERTY(Transient)
    TObjectPtr<UBorder> LobbyControlsPanel;

    UPROPERTY(Transient)
    TObjectPtr<UBorder> MatchResultPanel;

    UPROPERTY(Transient)
    TObjectPtr<UBorder> SettingsPanel;

    UPROPERTY(Transient)
    TObjectPtr<UBorder> SpectatorHelpPanel;

    UPROPERTY(Transient)
    TObjectPtr<UVerticalBox> StatusBox;

    UPROPERTY(Transient)
    TObjectPtr<UHorizontalBox> StatusCardsBox;

    UPROPERTY(Transient)
    TObjectPtr<UVerticalBox> CountdownBox;

    UPROPERTY(Transient)
    TObjectPtr<UVerticalBox> InfoBox;

    UPROPERTY(Transient)
    TObjectPtr<UVerticalBox> AnnouncementBox;

    UPROPERTY(Transient)
    TObjectPtr<UVerticalBox> LobbyBox;

    UPROPERTY(Transient)
    TObjectPtr<UVerticalBox> LobbyControlsBox;

    UPROPERTY(Transient)
    TObjectPtr<UHorizontalBox> LobbyControlsCardsBox;

    UPROPERTY(Transient)
    TObjectPtr<UVerticalBox> MatchResultBox;

    UPROPERTY(Transient)
    TObjectPtr<UVerticalBox> SettingsBox;

    UPROPERTY(Transient)
    TObjectPtr<UVerticalBox> SpectatorHelpBox;

    UPROPERTY(Transient)
    TObjectPtr<UBorder> LobbyKeyboardCard;

    UPROPERTY(Transient)
    TObjectPtr<UBorder> LobbyMouseCard;

    UPROPERTY(Transient)
    TObjectPtr<UBorder> HealthCard;

    UPROPERTY(Transient)
    TObjectPtr<UBorder> SprintCard;

    UPROPERTY(Transient)
    TObjectPtr<UBorder> RangedCard;

    UPROPERTY(Transient)
    TObjectPtr<UVerticalBox> LobbyKeyboardCardBox;

    UPROPERTY(Transient)
    TObjectPtr<UVerticalBox> LobbyMouseCardBox;

    UPROPERTY(Transient)
    TObjectPtr<UVerticalBox> HealthCardBox;

    UPROPERTY(Transient)
    TObjectPtr<UVerticalBox> SprintCardBox;

    UPROPERTY(Transient)
    TObjectPtr<UVerticalBox> RangedCardBox;

    UPROPERTY(Transient)
    TObjectPtr<USizeBox> HealthBarSizeBox;

    UPROPERTY(Transient)
    TObjectPtr<UProgressBar> HealthBar;

    UPROPERTY(Transient)
    TObjectPtr<USizeBox> SprintBarSizeBox;

    UPROPERTY(Transient)
    TObjectPtr<UProgressBar> SprintBar;

    UPROPERTY(Transient)
    TObjectPtr<USizeBox> RangedCooldownBarSizeBox;

    UPROPERTY(Transient)
    TObjectPtr<UProgressBar> RangedCooldownBar;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> HealthText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> SprintText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> RangedCooldownText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> MatchScoreText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> TimerText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> RoundStateText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> ResultText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> NextRoundText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> LobbyTitleText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> LobbyStatusText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> LobbyModeStatusText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> LobbyTeamStatusText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> LobbyLeftTeamListText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> LobbyRightTeamListText;

    UPROPERTY(Transient)
    TObjectPtr<UButton> LobbyReadyButton;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> LobbyReadyButtonText;

    UPROPERTY(Transient)
    TObjectPtr<UButton> LobbyFreeForAllModeButton;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> LobbyFreeForAllModeButtonText;

    UPROPERTY(Transient)
    TObjectPtr<UButton> LobbyTeamVersusModeButton;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> LobbyTeamVersusModeButtonText;

    UPROPERTY(Transient)
    TObjectPtr<UButton> LobbyLeftTeamButton;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> LobbyLeftTeamButtonText;

    UPROPERTY(Transient)
    TObjectPtr<UButton> LobbyRightTeamButton;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> LobbyRightTeamButtonText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> LobbyControlsTitleText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> LobbyControlsKeyboardTitleText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> LobbyControlsKeyboardMoveText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> LobbyControlsKeyboardSprintText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> LobbyControlsMouseTitleText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> LobbyControlsMouseMeleeText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> LobbyControlsMouseRangedText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> MatchResultTitleText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> MatchResultSummaryText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> ConnectionStatusText;

    UPROPERTY(Transient)
    TObjectPtr<UEditableTextBox> LobbyNicknameTextBox;

    UPROPERTY(Transient)
    TObjectPtr<UVerticalBox> LobbyPlayerListBox;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> SettingsTitleText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> SettingsDisplayModeLabelText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> SettingsResolutionLabelText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> SettingsAudioLabelText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> SettingsMasterVolumeText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> SettingsBgmVolumeText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> SettingsSfxVolumeText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> SettingsWindowModeButtonText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> SettingsResolutionButtonText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> SettingsVSyncButtonText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> SettingsResumeButtonText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> SettingsQuitButtonText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> SpectatorHelpTitleText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> SpectatorHelpMoveText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> SpectatorHelpLookText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> SpectatorHelpRiseText;

    UPROPERTY(Transient)
    TObjectPtr<UEditableTextBox> JoinAddressTextBox;

    UPROPERTY(Transient)
    TObjectPtr<UButton> HostMatchButton;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> HostMatchButtonText;

    UPROPERTY(Transient)
    TObjectPtr<UButton> JoinByIpButton;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> JoinByIpButtonText;

    UPROPERTY(Transient)
    TObjectPtr<USlider> SettingsMasterVolumeSlider;

    UPROPERTY(Transient)
    TObjectPtr<USlider> SettingsBgmVolumeSlider;

    UPROPERTY(Transient)
    TObjectPtr<USlider> SettingsSfxVolumeSlider;

    UPROPERTY(Transient)
    TObjectPtr<UButton> SettingsWindowModeButton;

    UPROPERTY(Transient)
    TObjectPtr<UButton> SettingsResolutionButton;

    UPROPERTY(Transient)
    TObjectPtr<UButton> SettingsVSyncButton;

    UPROPERTY(Transient)
    TObjectPtr<UButton> SettingsResumeButton;

    UPROPERTY(Transient)
    TObjectPtr<UButton> SettingsQuitButton;

    UPROPERTY(Transient)
    TObjectPtr<UOverlay> RangedCrosshairOverlay;

    UPROPERTY(Transient)
    TObjectPtr<USizeBox> RangedCrosshairHorizontalBox;

    UPROPERTY(Transient)
    TObjectPtr<USizeBox> RangedCrosshairVerticalBox;

    UPROPERTY(Transient)
    TObjectPtr<UBorder> RangedCrosshairHorizontalLine;

    UPROPERTY(Transient)
    TObjectPtr<UBorder> RangedCrosshairVerticalLine;

    UPROPERTY(EditDefaultsOnly, Category = "Audio")
    TObjectPtr<USoundBase> NonCombatBackgroundMusic;

    UPROPERTY(EditDefaultsOnly, Category = "Audio")
    TObjectPtr<USoundBase> GameplayBackgroundMusic;

    UPROPERTY(Transient)
    TObjectPtr<UAudioComponent> BackgroundMusicAudioComponent;

    UPROPERTY(Transient)
    TObjectPtr<USoundBase> CurrentBackgroundMusic;

    FTimerHandle RefreshTimerHandle;
    bool bLobbyInputModeActive = false;
    bool bSettingsMenuOpen = false;
    bool bUpdatingLobbyNicknameText = false;
    float MasterVolume = 1.0f;
    float BackgroundMusicVolume = 1.0f;
    float SfxVolume = 1.0f;
    int32 SelectedResolutionIndex = 0;
    int32 SelectedWindowModeIndex = 1;
    bool bVSyncEnabled = false;
};
