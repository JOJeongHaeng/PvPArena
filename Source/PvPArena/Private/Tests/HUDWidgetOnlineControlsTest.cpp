#include "Misc/AutomationTest.h"
#include "Game/PvPArenaPlayerController.h"
#include "UI/PvPArenaHUDWidget.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FHUDWidgetOnlineControlsTest,
    "PvPArena.UI.HUDWidgetOnlineControls",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHUDWidgetOnlineControlsTest::RunTest(const FString& Parameters)
{
    const UClass* WidgetClass = UPvPArenaHUDWidget::StaticClass();
    UPvPArenaHUDWidget* HUDWidget = NewObject<UPvPArenaHUDWidget>();
    TestNotNull(TEXT("HUD widget should be instantiable for travel command verification"), HUDWidget);

    const FObjectProperty* ConnectionStatusTextProperty =
        FindFProperty<FObjectProperty>(WidgetClass, TEXT("ConnectionStatusText"));
    TestNotNull(TEXT("HUD widget should expose a connection status text block"), ConnectionStatusTextProperty);

    const FObjectProperty* JoinAddressTextBoxProperty =
        FindFProperty<FObjectProperty>(WidgetClass, TEXT("JoinAddressTextBox"));
    TestNotNull(TEXT("HUD widget should expose a join address text box"), JoinAddressTextBoxProperty);

    const FObjectProperty* LobbyModeStatusTextProperty =
        FindFProperty<FObjectProperty>(WidgetClass, TEXT("LobbyModeStatusText"));
    TestNotNull(TEXT("HUD widget should expose a lobby mode status text block"), LobbyModeStatusTextProperty);

    const FObjectProperty* LobbyLeftTeamListTextProperty =
        FindFProperty<FObjectProperty>(WidgetClass, TEXT("LobbyLeftTeamListText"));
    TestNotNull(TEXT("HUD widget should expose a left-team player list"), LobbyLeftTeamListTextProperty);

    const FObjectProperty* LobbyRightTeamListTextProperty =
        FindFProperty<FObjectProperty>(WidgetClass, TEXT("LobbyRightTeamListText"));
    TestNotNull(TEXT("HUD widget should expose a right-team player list"), LobbyRightTeamListTextProperty);

    UFunction* HostMatchFunction = WidgetClass->FindFunctionByName(TEXT("HandleHostMatchButtonClicked"));
    TestNotNull(TEXT("HUD widget should expose a host-match button handler"), HostMatchFunction);

    UFunction* LobbyStartFunction = WidgetClass->FindFunctionByName(TEXT("HandleLobbyReadyButtonClicked"));
    TestNotNull(TEXT("HUD widget should expose a lobby start button handler"), LobbyStartFunction);

    UFunction* JoinByIpFunction = WidgetClass->FindFunctionByName(TEXT("HandleJoinByIpButtonClicked"));
    TestNotNull(TEXT("HUD widget should expose a join-by-ip button handler"), JoinByIpFunction);

    UFunction* FreeForAllModeFunction = WidgetClass->FindFunctionByName(TEXT("HandleLobbyFreeForAllModeButtonClicked"));
    TestNotNull(TEXT("HUD widget should expose a free-for-all mode button handler"), FreeForAllModeFunction);

    UFunction* TeamVersusModeFunction = WidgetClass->FindFunctionByName(TEXT("HandleLobbyTeamVersusModeButtonClicked"));
    TestNotNull(TEXT("HUD widget should expose a team-versus mode button handler"), TeamVersusModeFunction);

    UFunction* LeftTeamFunction = WidgetClass->FindFunctionByName(TEXT("HandleLobbyLeftTeamButtonClicked"));
    TestNotNull(TEXT("HUD widget should expose a left-team selection handler"), LeftTeamFunction);

    UFunction* RightTeamFunction = WidgetClass->FindFunctionByName(TEXT("HandleLobbyRightTeamButtonClicked"));
    TestNotNull(TEXT("HUD widget should expose a right-team selection handler"), RightTeamFunction);

    UFunction* BuildHostTravelCommandFunction = WidgetClass->FindFunctionByName(TEXT("BuildHostTravelCommand"));
    TestNotNull(TEXT("HUD widget should expose a host travel command builder"), BuildHostTravelCommandFunction);

    UFunction* BuildJoinTravelCommandFunction = WidgetClass->FindFunctionByName(TEXT("BuildJoinTravelCommand"));
    TestNotNull(TEXT("HUD widget should expose a join travel command builder"), BuildJoinTravelCommandFunction);

    UFunction* BuildHostTravelMapNameFunction = WidgetClass->FindFunctionByName(TEXT("BuildHostTravelMapName"));
    TestNotNull(TEXT("HUD widget should expose a host travel map-name builder"), BuildHostTravelMapNameFunction);

    UFunction* BuildHostTravelOptionsFunction = WidgetClass->FindFunctionByName(TEXT("BuildHostTravelOptions"));
    TestNotNull(TEXT("HUD widget should expose host travel options for OpenLevel"), BuildHostTravelOptionsFunction);

    UFunction* BuildJoinTravelAddressFunction = WidgetClass->FindFunctionByName(TEXT("BuildJoinTravelAddress"));
    TestNotNull(TEXT("HUD widget should expose a join travel address builder"), BuildJoinTravelAddressFunction);

    const UClass* PlayerControllerClass = APvPArenaPlayerController::StaticClass();
    UFunction* RequestLobbyMatchStartFunction = PlayerControllerClass->FindFunctionByName(TEXT("RequestLobbyMatchStart"));
    TestNotNull(TEXT("Player controller should expose a host-only lobby start request"), RequestLobbyMatchStartFunction);

    UFunction* RequestLobbyModeChangeFunction = PlayerControllerClass->FindFunctionByName(TEXT("RequestLobbyMatchModeChange"));
    TestNotNull(TEXT("Player controller should expose a host-only lobby mode change request"), RequestLobbyModeChangeFunction);

    UFunction* RequestLobbyTeamSelectionFunction = PlayerControllerClass->FindFunctionByName(TEXT("RequestLobbyTeamSelection"));
    TestNotNull(TEXT("Player controller should expose a lobby team selection request"), RequestLobbyTeamSelectionFunction);

    FString HostTravelCommand;
    if (HUDWidget && BuildHostTravelCommandFunction)
    {
        struct FBuildHostTravelCommandParams
        {
            FString ReturnValue;
        };

        FBuildHostTravelCommandParams Params;
        HUDWidget->ProcessEvent(BuildHostTravelCommandFunction, &Params);
        HostTravelCommand = Params.ReturnValue;
    }

    FString HostTravelMapName;
    if (HUDWidget && BuildHostTravelMapNameFunction)
    {
        struct FBuildHostTravelMapNameParams
        {
            FString ReturnValue;
        };

        FBuildHostTravelMapNameParams Params;
        HUDWidget->ProcessEvent(BuildHostTravelMapNameFunction, &Params);
        HostTravelMapName = Params.ReturnValue;
    }

    FString HostTravelOptions;
    if (HUDWidget && BuildHostTravelOptionsFunction)
    {
        struct FBuildHostTravelOptionsParams
        {
            FString ReturnValue;
        };

        FBuildHostTravelOptionsParams Params;
        HUDWidget->ProcessEvent(BuildHostTravelOptionsFunction, &Params);
        HostTravelOptions = Params.ReturnValue;
    }

    FString JoinTravelAddress;
    if (HUDWidget && BuildJoinTravelAddressFunction)
    {
        struct FBuildJoinTravelAddressParams
        {
            FString JoinAddress;
            FString ReturnValue;
        };

        FBuildJoinTravelAddressParams Params;
        Params.JoinAddress = TEXT(" 127.0.0.1 ");
        HUDWidget->ProcessEvent(BuildJoinTravelAddressFunction, &Params);
        JoinTravelAddress = Params.ReturnValue;
    }

    TestEqual(
        TEXT("Host travel command should open the packaged default arena map as a listen server"),
        HostTravelCommand,
        FString(TEXT("open /Game/PvPArena/Maps/PvPArena_map?listen")));

    TestEqual(
        TEXT("Host travel map name should target the packaged default arena map"),
        HostTravelMapName,
        FString(TEXT("/Game/PvPArena/Maps/PvPArena_map")));

    TestEqual(
        TEXT("Host travel options should enable listen-server hosting"),
        HostTravelOptions,
        FString(TEXT("listen")));

    TestEqual(
        TEXT("Join travel address should trim whitespace and default the port"),
        JoinTravelAddress,
        FString(TEXT("127.0.0.1:7777")));

    return HUDWidget
        && ConnectionStatusTextProperty
        && JoinAddressTextBoxProperty
        && LobbyModeStatusTextProperty
        && LobbyLeftTeamListTextProperty
        && LobbyRightTeamListTextProperty
        && HostMatchFunction
        && LobbyStartFunction
        && JoinByIpFunction
        && FreeForAllModeFunction
        && TeamVersusModeFunction
        && LeftTeamFunction
        && RightTeamFunction
        && BuildHostTravelCommandFunction
        && BuildJoinTravelCommandFunction
        && BuildHostTravelMapNameFunction
        && BuildHostTravelOptionsFunction
        && BuildJoinTravelAddressFunction
        && RequestLobbyMatchStartFunction
        && RequestLobbyModeChangeFunction
        && RequestLobbyTeamSelectionFunction
        && HostTravelCommand == TEXT("open /Game/PvPArena/Maps/PvPArena_map?listen")
        && HostTravelMapName == TEXT("/Game/PvPArena/Maps/PvPArena_map")
        && HostTravelOptions == TEXT("listen")
        && JoinTravelAddress == TEXT("127.0.0.1:7777");
}
