#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PvPArenaOverheadStatusWidget.generated.h"

class UProgressBar;
class UTextBlock;
struct FGeometry;

UCLASS()
class PVPARENA_API UPvPArenaOverheadStatusWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    static void BuildDisplayState(const class APvPArenaCharacter* Character, FString& OutNickname, float& OutHealthPercent);
    void SetObservedCharacter(const class APvPArenaCharacter* Character);
    void RefreshFromCharacter(const class APvPArenaCharacter* Character);

private:
    void RefreshFromOwningCharacter();

    UPROPERTY(meta = (BindWidgetOptional), Transient)
    TObjectPtr<UTextBlock> NicknameText;

    UPROPERTY(meta = (BindWidgetOptional), Transient)
    TObjectPtr<UProgressBar> HealthBar;

    UPROPERTY(Transient)
    TWeakObjectPtr<const class APvPArenaCharacter> ObservedCharacter;
};
