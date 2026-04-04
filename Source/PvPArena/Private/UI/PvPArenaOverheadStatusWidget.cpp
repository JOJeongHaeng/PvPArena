#include "UI/PvPArenaOverheadStatusWidget.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/WidgetComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"
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
    else if (PlayerState && !PlayerState->GetPlayerName().IsEmpty())
    {
        OutNickname = PlayerState->GetPlayerName();
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

EPvPArenaOverheadRelationship UPvPArenaOverheadStatusWidget::ResolveRelationship(
    const APvPArenaPlayerState* ObservedPlayerState,
    const APvPArenaPlayerState* LocalPlayerState)
{
    if (!ObservedPlayerState)
    {
        return EPvPArenaOverheadRelationship::Enemy;
    }

    if (ObservedPlayerState == LocalPlayerState)
    {
        return EPvPArenaOverheadRelationship::Self;
    }

    if (!LocalPlayerState)
    {
        return EPvPArenaOverheadRelationship::Enemy;
    }

    if (ObservedPlayerState->GetLobbyMatchMode() == EPvPALobbyMatchMode::TeamVersus
        && LocalPlayerState->GetLobbyMatchMode() == EPvPALobbyMatchMode::TeamVersus
        && ObservedPlayerState->GetLobbyTeam() != EPvPALobbyTeam::None
        && ObservedPlayerState->GetLobbyTeam() == LocalPlayerState->GetLobbyTeam())
    {
        return EPvPArenaOverheadRelationship::Ally;
    }

    return EPvPArenaOverheadRelationship::Enemy;
}

FString UPvPArenaOverheadStatusWidget::BuildRelationshipBadge(EPvPArenaOverheadRelationship Relationship)
{
    switch (Relationship)
    {
    case EPvPArenaOverheadRelationship::Self:
        return FString();
    case EPvPArenaOverheadRelationship::Ally:
        return TEXT("ALLY");
    default:
        return TEXT("ENEMY");
    }
}

FLinearColor UPvPArenaOverheadStatusWidget::BuildRelationshipColor(
    EPvPArenaOverheadRelationship Relationship,
    bool bIsHighlighted)
{
    const FLinearColor BaseColor = [&]()
    {
        switch (Relationship)
        {
        case EPvPArenaOverheadRelationship::Self:
            return FLinearColor(0.96f, 0.98f, 1.0f, 1.0f);
        case EPvPArenaOverheadRelationship::Ally:
            return FLinearColor(0.36f, 0.72f, 1.0f, 1.0f);
        default:
            return FLinearColor(1.0f, 0.34f, 0.34f, 1.0f);
        }
    }();

    return bIsHighlighted
        ? FLinearColor(
            FMath::Min(BaseColor.R + 0.18f, 1.0f),
            FMath::Min(BaseColor.G + 0.18f, 1.0f),
            FMath::Min(BaseColor.B + 0.18f, 1.0f),
            1.0f)
        : BaseColor;
}

FLinearColor UPvPArenaOverheadStatusWidget::BuildHealthBarFillColor(
    EPvPArenaOverheadRelationship Relationship,
    bool bIsHighlighted)
{
    const FLinearColor BaseColor = [&]()
    {
        switch (Relationship)
        {
        case EPvPArenaOverheadRelationship::Self:
            return FLinearColor(0.10f, 0.72f, 0.56f, 0.98f);
        case EPvPArenaOverheadRelationship::Ally:
            return FLinearColor(0.06f, 0.32f, 0.92f, 0.98f);
        default:
            return FLinearColor(0.88f, 0.14f, 0.12f, 0.98f);
        }
    }();

    return bIsHighlighted
        ? FLinearColor(
            FMath::Min(BaseColor.R + 0.08f, 1.0f),
            FMath::Min(BaseColor.G + 0.06f, 1.0f),
            FMath::Min(BaseColor.B + 0.06f, 1.0f),
            1.0f)
        : BaseColor;
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

    const APvPArenaCharacter* LocalCharacter = nullptr;
    if (const UWorld* World = GetWorld())
    {
        if (const APlayerController* LocalPlayerController = World->GetFirstPlayerController())
        {
            LocalCharacter = Cast<APvPArenaCharacter>(LocalPlayerController->GetPawn());
        }
    }

    const APvPArenaPlayerState* ObservedPlayerState = ObservedCharacter.IsValid()
        ? Cast<APvPArenaPlayerState>(ObservedCharacter->GetPlayerState())
        : nullptr;
    const APvPArenaPlayerState* LocalPlayerState = LocalCharacter
        ? Cast<APvPArenaPlayerState>(LocalCharacter->GetPlayerState())
        : nullptr;
    const EPvPArenaOverheadRelationship Relationship = ResolveRelationship(ObservedPlayerState, LocalPlayerState);
    const bool bIsHighlighted = IsCrosshairTargetedByLocalPlayer(ObservedCharacter.Get());
    const FString Badge = BuildRelationshipBadge(Relationship);
    const FString DecoratedNickname = Badge.IsEmpty()
        ? Nickname
        : FString::Printf(TEXT("[%s] %s"), *Badge, *Nickname);

    if (NicknameText)
    {
        NicknameText->SetText(FText::FromString(DecoratedNickname));
        NicknameText->SetColorAndOpacity(FSlateColor(BuildRelationshipColor(Relationship, bIsHighlighted)));
    }

    if (HealthBar)
    {
        HealthBar->PercentDelegate.Unbind();
        HealthBar->SetPercent(HealthPercent);
        HealthBar->SetFillColorAndOpacity(BuildHealthBarFillColor(Relationship, bIsHighlighted));
        HealthBar->SynchronizeProperties();
    }

    SetRenderScale(bIsHighlighted ? FVector2D(1.1f, 1.1f) : FVector2D(1.0f, 1.0f));
}

bool UPvPArenaOverheadStatusWidget::IsCrosshairTargetedByLocalPlayer(const APvPArenaCharacter* ObservedCharacter)
{
    if (!ObservedCharacter)
    {
        return false;
    }

    const UWorld* World = ObservedCharacter->GetWorld();
    const APlayerController* LocalPlayerController = World ? World->GetFirstPlayerController() : nullptr;
    const APvPArenaCharacter* LocalCharacter = LocalPlayerController
        ? Cast<APvPArenaCharacter>(LocalPlayerController->GetPawn())
        : nullptr;
    if (!LocalPlayerController || !LocalCharacter)
    {
        return false;
    }

    FVector AimPoint = FVector::ZeroVector;
    if (!LocalCharacter->ResolveRangedCrosshairAimPoint(AimPoint))
    {
        return false;
    }

    FVector ViewLocation = FVector::ZeroVector;
    FRotator ViewRotation = FRotator::ZeroRotator;
    LocalPlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);

    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(PvPArenaOverheadCrosshairTargetTrace), false, LocalCharacter);
    QueryParams.AddIgnoredActor(LocalCharacter);

    FHitResult HitResult;
    if (!World->LineTraceSingleByChannel(HitResult, ViewLocation, AimPoint, ECC_Visibility, QueryParams))
    {
        return false;
    }

    return HitResult.GetActor() == ObservedCharacter;
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
