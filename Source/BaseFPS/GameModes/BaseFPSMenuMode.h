// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BaseFPSMenuMode.generated.h"

class UCommonActivatableWidget;
/**
 * 
 */
UCLASS(Blueprintable)
class BASEFPS_API ABaseFPSMenuMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	ABaseFPSMenuMode();

protected:
	UPROPERTY(EditDefaultsOnly, Category="MainMenu")
	TSubclassOf<UCommonActivatableWidget> MainScreenClass;
	
	//~ Begin AGameModeBase interface
	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	//~ End AGameModeBase interface	
};
