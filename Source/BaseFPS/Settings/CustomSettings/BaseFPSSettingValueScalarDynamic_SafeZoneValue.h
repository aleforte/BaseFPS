// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameSettingAction.h"
#include "GameSettingValueScalarDynamic.h"
#include "BaseFPSSettingValueScalarDynamic_SafeZoneValue.generated.h"

/**
 * 
 */
UCLASS()
class BASEFPS_API UBaseFPSSettingValueScalarDynamic_SafeZoneValue : public UGameSettingValueScalarDynamic
{
	GENERATED_BODY()
	
public:
	virtual void ResetToDefault() override;
	virtual void RestoreToInitial() override;
};

UCLASS()
class BASEFPS_API UBaseFPSSettingAction_SafeZoneEditor : public UGameSettingAction
{
	GENERATED_BODY()
	
public:
	UBaseFPSSettingAction_SafeZoneEditor();
	virtual TArray<UGameSetting*> GetChildSettings() override;

private:
	UPROPERTY()
	TObjectPtr<UBaseFPSSettingValueScalarDynamic_SafeZoneValue> SafeZoneValueSetting;
};
