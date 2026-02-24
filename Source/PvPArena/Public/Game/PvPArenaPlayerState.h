#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "PvPArenaPlayerState.generated.h"

UCLASS()
class PVPARENA_API APvPArenaPlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    int32 GetKills() const { return Kills; }
    int32 GetDeaths() const { return Deaths; }

    void AddKill();
    void AddDeath();

protected:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    UPROPERTY(Replicated, VisibleAnywhere, Category = "Score")
    int32 Kills = 0;

    UPROPERTY(Replicated, VisibleAnywhere, Category = "Score")
    int32 Deaths = 0;
};
