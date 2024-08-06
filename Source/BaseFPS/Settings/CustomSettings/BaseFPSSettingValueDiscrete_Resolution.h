// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameSettingValueDiscrete.h"
#include "BaseFPSSettingValueDiscrete_Resolution.generated.h"

struct FScreenResolutionRHI;

/**
 * 
 */
UCLASS()
class BASEFPS_API UBaseFPSSettingValueDiscrete_Resolution : public UGameSettingValueDiscrete
{
	GENERATED_BODY()

public:
	
	UBaseFPSSettingValueDiscrete_Resolution();

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
	//~Begin UGameSettingValue interface
	virtual void OnInitialized() override;
	virtual void OnDependencyChanged() override;
	//~End UGameSettingValue interface

	void InitializeResolutions();
	bool ShouldAllowFullScreenResolution(const FScreenResolutionRHI& SrcScreenRes, int32 FilterThreshold) const;
	static void GetStandardWindowResolutions(const FIntPoint& MinResolution, const FIntPoint& MaxResolution, float MinAspectRatio, TArray<FIntPoint>& OutResolutions);
	void SelectAppropriateResolutions();
	int32 FindIndexOfDisplayResolution(const FIntPoint& InPoint) const;
	int32 FindIndexOfDisplayResolutionForceValid(const FIntPoint& InPoint) const;
	int32 FindClosestResolutionIndex(const FIntPoint& Resolution) const;

	TOptional<EWindowMode::Type> LastWindowMode;

	struct FScreenResolutionEntry
	{
		uint32	Width = 0;
		uint32	Height = 0;
		uint32	RefreshRate = 0;
		FText   OverrideText;

		FIntPoint GetResolution() const { return FIntPoint(Width, Height); }
		FText GetDisplayText() const;
	};

	/** An array of strings the map to resolutions, populated based on the window mode */
	TArray< TSharedPtr< FScreenResolutionEntry > > Resolutions;

	/** An array of strings the map to fullscreen resolutions */
	TArray< TSharedPtr< FScreenResolutionEntry > > ResolutionsFullscreen;

	/** An array of strings the map to windowed fullscreen resolutions */
	TArray< TSharedPtr< FScreenResolutionEntry > > ResolutionsWindowedFullscreen;

	/** An array of strings the map to windowed resolutions */
	TArray< TSharedPtr< FScreenResolutionEntry > > ResolutionsWindowed;
};
