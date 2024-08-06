// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameSettingValueDiscrete.h"
#include "BaseFPSSettingValueDiscrete_OverallQuality.generated.h"

/**
 * 
 */
UCLASS()
class BASEFPS_API UBaseFPSSettingValueDiscrete_OverallQuality : public UGameSettingValueDiscrete
{
	GENERATED_BODY()

public:

	UBaseFPSSettingValueDiscrete_OverallQuality();

	/** UGameSettingValue */
	virtual void StoreInitial() override;
	virtual void ResetToDefault() override;
	virtual void RestoreToInitial() override;

	/** UGameSettingValueDiscrete */
	virtual void SetDiscreteOptionByIndex(int32 Index) override;
	virtual int32 GetDiscreteOptionIndex() const override;
	virtual TArray<FText> GetDiscreteOptions() const override;

protected:
	/** UGameSettingValue */
	virtual void OnInitialized() override;
	
	int32 GetCustomOptionIndex() const;
	int32 GetOverallQualityLevel() const;

	TArray<FText> Options;
	TArray<FText> OptionsWithCustom;
	
};
