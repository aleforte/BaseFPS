// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/BaseFPSMenuMode.h"

#include "BaseFPS.h"
#include "CommonSessionSubsystem.h"
#include "CommonUIExtensions.h"
#include "CommonActivatableWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Player/BaseFPSPlayerController.h"

class UCommonSessionSubsystem;

ABaseFPSMenuMode::ABaseFPSMenuMode()
{
	PlayerControllerClass = ABaseFPSPlayerController::StaticClass();
	DefaultPawnClass = nullptr;
	
	static ConstructorHelpers::FClassFinder<UCommonActivatableWidget> MainMenuClassFinder(TEXT("/Game/UI/Menus/W_MainMenu"));
	MainScreenClass = MainMenuClassFinder.Class;
}

void ABaseFPSMenuMode::BeginPlay()
{
	Super::BeginPlay();

	if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this))
	{
		// Always reset sessions
		UCommonSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UCommonSessionSubsystem>();
		if (ensure(SessionSubsystem))
		{
			SessionSubsystem->CleanUpSessions();
		}		
	}
}

void ABaseFPSMenuMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	UCommonUIExtensions::PushContentToLayer_ForPlayer(NewPlayer->GetLocalPlayer(), FrontendTags::TAG_UI_LAYER_MENU, MainScreenClass);
}
