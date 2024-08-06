// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameSettingValueDiscrete.h"
#include "BaseFPSSettingValueDiscrete_Language.generated.h"

/**
 * 
 */
UCLASS()
class BASEFPS_API UBaseFPSSettingValueDiscrete_Language : public UGameSettingValueDiscrete
{
	GENERATED_BODY()	
public:

	UBaseFPSSettingValueDiscrete_Language();

	//~Begin UGameSettingValue interface
	virtual void StoreInitial() override;
	virtual void ResetToDefault() override;
	virtual void RestoreToInitial() override;
	//~End UGameSettingValue interface

	//~Begin UGameSettingValueDiscrete interface
	virtual void SetDiscreteOptionByIndex(int32 Index) override;
	virtual int32 GetDiscreteOptionIndex() const override;
	virtual TArray<FText> GetDiscreteOptions() const override;
	//~End UGameSettingValueDiscrete interface

protected:
	/** UGameSettingValue */
	virtual void OnInitialized() override;
	virtual void OnApply() override;

protected:
	TArray<FString> AvailableCultureNames;
};
