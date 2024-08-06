// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "BaseFPSActivatableWidget.generated.h"

UENUM(BlueprintType)
enum class EBaseFPSWidgetInputMode : uint8
{
	Default,
	GameAndMenu,
	Game,
	Menu
};

// Activatable Widget that automatically drives the desired input config when activated
UCLASS(Abstract, Blueprintable)
class BASEFPS_API UBaseFPSActivatableWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UBaseFPSActivatableWidget(const FObjectInitializer& ObjectInitializer);

	//~Begin UCommonActivatableWidget interface
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
	//~End UCommonActivatableWidget interface

	#if WITH_EDITOR
	virtual void ValidateCompiledWidgetTree(const UWidgetTree& BlueprintWidgetTree, class IWidgetCompilerLog& CompileLog) const override;
#endif
	
protected:
	/** The desired input mode to use while this UI is activated, for example do you want key presses to still reach the game/player controller? */
	UPROPERTY(EditDefaultsOnly, Category="Input")
	EBaseFPSWidgetInputMode InputConfig = EBaseFPSWidgetInputMode::Default;

	/** The desired mouse behavior when the game gets input. */
	UPROPERTY(EditDefaultsOnly, Category="Input")
	EMouseCaptureMode GameMouseCaptureMode = EMouseCaptureMode::CapturePermanently;

};
