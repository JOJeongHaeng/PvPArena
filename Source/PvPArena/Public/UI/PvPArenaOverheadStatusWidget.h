#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PvPArenaOverheadStatusWidget.generated.h"

class UProgressBar;
class UTextBlock;
struct FGeometry;
class APvPArenaPlayerState;

UENUM()
enum class EPvPArenaOverheadRelationship : uint8
{
    Self,
    Ally,
    Enemy
};

UCLASS()
class PVPARENA_API UPvPArenaOverheadStatusWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    static void BuildDisplayState(const class APvPArenaCharacter* Character, FString& OutNickname, float& OutHealthPercent);
    static EPvPArenaOverheadRelationship ResolveRelationship(
        const APvPArenaPlayerState* ObservedPlayerState,
        const APvPArenaPlayerState* LocalPlayerState);
    static FString BuildRelationshipBadge(EPvPArenaOverheadRelationship Relationship);
    static FLinearColor BuildRelationshipColor(EPvPArenaOverheadRelationship Relationship, bool bIsHighlighted);
    static FLinearColor BuildHealthBarFillColor(EPvPArenaOverheadRelationship Relationship, bool bIsHighlighted);
    void SetObservedCharacter(const class APvPArenaCharacter* Character);
    void RefreshFromCharacter(const class APvPArenaCharacter* Character);

private:
    static bool IsCrosshairTargetedByLocalPlayer(const APvPArenaCharacter* ObservedCharacter);
    void RefreshFromOwningCharacter();

    UPROPERTY(meta = (BindWidgetOptional), Transient)
    TObjectPtr<UTextBlock> NicknameText;

    UPROPERTY(meta = (BindWidgetOptional), Transient)
    TObjectPtr<UProgressBar> HealthBar;

    UPROPERTY(Transient)
    TWeakObjectPtr<const class APvPArenaCharacter> ObservedCharacter;
};
