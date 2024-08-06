// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameSettingRegistry.h"
#include "Logging/LogMacros.h"
#include "Misc/AssertionMacros.h"
#include "Settings/BaseFPSSettingsLocal.h" // IWYU pragma: keep
#include "BaseFPSSettingRegistry.generated.h"

class UGameSettingCollection;
class UBaseFPSLocalPlayer;

DECLARE_LOG_CATEGORY_EXTERN(LogBaseFPSSettingRegistry, Log, Log);

#define GET_SHARED_SETTINGS_FUNCTION_PATH(FunctionOrPropertyName)							\
	MakeShared<FGameSettingDataSourceDynamic>(TArray<FString>({								\
		GET_FUNCTION_NAME_STRING_CHECKED(UBaseFPSLocalPlayer, GetSharedSettings),			\
		GET_FUNCTION_NAME_STRING_CHECKED(UBaseFPSSettingsShared, FunctionOrPropertyName)	\
	}))

#define GET_LOCAL_SETTINGS_FUNCTION_PATH(FunctionOrPropertyName)							\
	MakeShared<FGameSettingDataSourceDynamic>(TArray<FString>({								\
		GET_FUNCTION_NAME_STRING_CHECKED(UBaseFPSLocalPlayer, GetLocalSettings),			\
		GET_FUNCTION_NAME_STRING_CHECKED(UBaseFPSSettingsLocal, FunctionOrPropertyName)		\
	}))

/**
 * UBaseFPSSettingRegistry
 */
UCLASS()
class BASEFPS_API UBaseFPSSettingRegistry : public UGameSettingRegistry
{
	GENERATED_BODY()

public:
	UBaseFPSSettingRegistry();

	static UBaseFPSSettingRegistry* Get(UBaseFPSLocalPlayer* InLocalPlayer);

	virtual void SaveChanges() override;

protected:
	virtual void OnInitialize(ULocalPlayer* InLocalPlayer) override;
	virtual bool IsFinishedInitializing() const override;

	UGameSettingCollection* InitializeVideoSettings(UBaseFPSLocalPlayer* InLocalPlayer);
	void InitializeVideoSettings_FrameRates(UGameSettingCollection* Screen, UBaseFPSLocalPlayer* InLocalPlayer);
	void AddPerformanceStatPage(UGameSettingCollection* Screen, UBaseFPSLocalPlayer* InLocalPlayer);

	UGameSettingCollection* InitializeAudioSettings(UBaseFPSLocalPlayer* InLocalPlayer);
	UGameSettingCollection* InitializeGameplaySettings(UBaseFPSLocalPlayer* InLocalPlayer);

	UGameSettingCollection* InitializeMouseAndKeyboardSettings(UBaseFPSLocalPlayer* InLocalPlayer);
	UGameSettingCollection* InitializeGamepadSettings(UBaseFPSLocalPlayer* InLocalPlayer);

	UPROPERTY()
	TObjectPtr<UGameSettingCollection> VideoSettings;

	UPROPERTY()
	TObjectPtr<UGameSettingCollection> AudioSettings;

	UPROPERTY()
	TObjectPtr<UGameSettingCollection> GameplaySettings;

	UPROPERTY()
	TObjectPtr<UGameSettingCollection> MouseAndKeyboardSettings;

	UPROPERTY()
	TObjectPtr<UGameSettingCollection> GamepadSettings;	
};
