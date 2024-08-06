// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BaseFPSGameMode.generated.h"

/**
 * ABaseFPSGameMode
 *
 *	The base game mode class used by this project.
 */
UCLASS(Config = Game, Meta = (ShortTooltip = "The base game mode class used by this project."))
class ABaseFPSGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ABaseFPSGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ Begin AGameModeBase interface
	virtual void FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation) override;
	virtual void SetPlayerDefaults(APawn* PlayerPawn) override;
	//~ End AGameModeBase interface
	
};



