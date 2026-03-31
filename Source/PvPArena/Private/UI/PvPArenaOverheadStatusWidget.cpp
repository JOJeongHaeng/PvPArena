#include "UI/PvPArenaOverheadStatusWidget.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/PlayerState.h"
#include "Game/PvPArenaPlayerState.h"
#include "Player/PvPArenaCharacter.h"
#include "PvPArena.h"

void UPvPArenaOverheadStatusWidget::NativeConstruct()
{
    Super::NativeConstruct();
    RefreshFromOwningCharacter();
}

void UPvPArenaOverheadStatusWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    RefreshFromOwningCharacter();
}

void UPvPArenaOverheadStatusWidget::BuildDisplayState(const APvPArenaCharacter* Character, FString& OutNickname, float& OutHealthPercent)
{
    if (!Character)
    {
        OutNickname = TEXT("Player");
        OutHealthPercent = 1.0f;
        return;
    }

    const APlayerState* PlayerState = Character->GetPlayerState();
    const APvPArenaPlayerState* PvPPlayerState = Cast<APvPArenaPlayerState>(PlayerState);
    if (PvPPlayerState && !PvPPlayerState->GetDisplayNickname().IsEmpty())
    {
        OutNickname = PvPPlayerState->GetDisplayNickname();
    }
    else if (PlayerState && PlayerState->GetPlayerId() > 0)
    {
        OutNickname = FString::Printf(TEXT("Player%d"), PlayerState->GetPlayerId());
    }
    else
    {
        OutNickname = FString(TEXT("Player"));
    }

    const float MaxHealth = FMath::Max(1.0f, Character->GetMaxHealth());
    OutHealthPercent = FMath::Clamp(Character->GetCurrentHealth() / MaxHealth, 0.0f, 1.0f);
}

void UPvPArenaOverheadStatusWidget::SetObservedCharacter(const APvPArenaCharacter* Character)
{
    ObservedCharacter = Character;
}

void UPvPArenaOverheadStatusWidget::RefreshFromCharacter(const APvPArenaCharacter* Character)
{
    if (Character)
    {
        ObservedCharacter = Character;
    }

    FString Nickname;
    float HealthPercent = 1.0f;
    BuildDisplayState(ObservedCharacter.Get(), Nickname, HealthPercent);

    if (NicknameText)
    {
        NicknameText->SetText(FText::FromString(Nickname));
    }

    if (HealthBar)
    {
        HealthBar->PercentDelegate.Unbind();
        HealthBar->SetPercent(HealthPercent);
        HealthBar->SynchronizeProperties();
    }
}

void UPvPArenaOverheadStatusWidget::RefreshFromOwningCharacter()
{
    const APvPArenaCharacter* Character = ObservedCharacter.Get();
    if (!Character)
    {
        const UWidgetComponent* WidgetComponent = GetTypedOuter<UWidgetComponent>();
        Character = WidgetComponent ? Cast<APvPArenaCharacter>(WidgetComponent->GetOwner()) : nullptr;
        if (Character)
        {
            ObservedCharacter = Character;
        }
    }

    RefreshFromCharacter(Character);
}
