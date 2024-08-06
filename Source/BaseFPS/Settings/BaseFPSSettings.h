// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "Engine/DeveloperSettings.h"
#include "BaseFPSSettings.generated.h"

class UPlayerMappableInputConfig;
class UGameUIPolicy;
class UCommonActivatableWidget;
class UCommonGameDialog;

/**
 * 
 */
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "BaseFPS Settings"))
class BASEFPS_API UBaseFPSSettings : public UDeveloperSettings
{
	GENERATED_BODY()

	public:

	UBaseFPSSettings(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()) {}

	/************************************************************************/
	/* General                                                              */
	/************************************************************************/
public:
	TSoftClassPtr<UGameUIPolicy> GetDefaultUIPolicyClass() const { return DefaultUIPolicyClass; }
	
private:
	UPROPERTY(config, EditDefaultsOnly, Category="General")
	TSoftClassPtr<UGameUIPolicy> DefaultUIPolicyClass;

	/************************************************************************/
	/* Input                                                                */
	/************************************************************************/
public:
	TSoftObjectPtr<UPlayerMappableInputConfig> GetDefaultInputConfig() const { return DefaultInputConfig; }

private:
	UPROPERTY(Config, EditAnywhere, Category="Input")
	TSoftObjectPtr<UPlayerMappableInputConfig> DefaultInputConfig;
};
