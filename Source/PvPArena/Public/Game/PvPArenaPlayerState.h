#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "PvPArenaPlayerState.generated.h"

UENUM(BlueprintType)
enum class EPvPALobbyMatchMode : uint8
{
    FreeForAll,
    TeamVersus
};

UENUM(BlueprintType)
enum class EPvPALobbyTeam : uint8
{
    None,
    Left,
    Right
};

UCLASS()
class PVPARENA_API APvPArenaPlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    EPvPALobbyMatchMode GetLobbyMatchMode() const { return LobbyMatchMode; }
    EPvPALobbyTeam GetLobbyTeam() const { return LobbyTeam; }
    int32 GetRoundKills() const { return RoundKills; }
    int32 GetRoundDeaths() const { return RoundDeaths; }
    int32 GetMatchKills() const { return MatchKills; }
    int32 GetMatchDeaths() const { return MatchDeaths; }
    int32 GetRoundWins() const { return RoundWins; }
    bool IsReadyForLobbyStart() const { return bReadyForLobbyStart; }
    int32 GetKills() const { return GetRoundKills(); }
    int32 GetDeaths() const { return GetRoundDeaths(); }
    const FString& GetDisplayNickname() const { return DisplayNickname; }

    static FString BuildNormalizedDisplayNickname(const FString& RawNickname);

    void AddKill();
    void AddDeath();
    void AddRoundWin();
    void SetReadyForLobbyStart(bool bNewReady);
    void SetDisplayNickname(const FString& NewNickname);
    void SetLobbyMatchMode(EPvPALobbyMatchMode NewLobbyMatchMode);
    void SetLobbyTeam(EPvPALobbyTeam NewLobbyTeam);
    void ResetLobbyStateForModeChange(EPvPALobbyMatchMode NewLobbyMatchMode);
    void ResetRoundStats();
    void ResetMatchStats();

protected:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    UPROPERTY(Replicated, VisibleAnywhere, Category = "Score")
    int32 RoundKills = 0;

    UPROPERTY(Replicated, VisibleAnywhere, Category = "Score")
    int32 RoundDeaths = 0;

    UPROPERTY(Replicated, VisibleAnywhere, Category = "Score")
    int32 MatchKills = 0;

    UPROPERTY(Replicated, VisibleAnywhere, Category = "Score")
    int32 MatchDeaths = 0;

    UPROPERTY(Replicated, VisibleAnywhere, Category = "Score")
    int32 RoundWins = 0;

    UPROPERTY(Replicated, VisibleAnywhere, Category = "Match")
    bool bReadyForLobbyStart = false;

    UPROPERTY(Replicated, VisibleAnywhere, Category = "Identity")
    FString DisplayNickname;

    UPROPERTY(Replicated, VisibleAnywhere, Category = "Match")
    EPvPALobbyMatchMode LobbyMatchMode = EPvPALobbyMatchMode::FreeForAll;

    UPROPERTY(Replicated, VisibleAnywhere, Category = "Match")
    EPvPALobbyTeam LobbyTeam = EPvPALobbyTeam::None;
};
