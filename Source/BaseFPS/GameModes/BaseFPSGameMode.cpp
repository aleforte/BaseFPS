// Copyright Epic Games, Inc. All Rights Reserved.

#include "BaseFPSGameMode.h"

#include "Character/BaseFPSCharacter.h"
#include "Player/BaseFPSPlayerController.h"
#include "UI/BaseFPSHUD.h"
#include "UObject/ConstructorHelpers.h"

ABaseFPSGameMode::ABaseFPSGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PlayerControllerClass = ABaseFPSPlayerController::StaticClass();
	HUDClass = ABaseFPSHUD::StaticClass();
	
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/Characters/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;
}

void ABaseFPSGameMode::FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation)
{
	Super::FinishRestartPlayer(NewPlayer, StartRotation);

	if (ABaseFPSCharacter* Character = Cast<ABaseFPSCharacter>(NewPlayer->GetPawn()))
	{
		Character->SwitchToStartingWeapon();
	}
}

void ABaseFPSGameMode::SetPlayerDefaults(APawn* PlayerPawn)
{
	Super::SetPlayerDefaults(PlayerPawn);

	if (ABaseFPSCharacter* Character = Cast<ABaseFPSCharacter>(PlayerPawn))
	{
		Character->AddDefaultInventory();
	}
}

