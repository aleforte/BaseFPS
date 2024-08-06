// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Input/CommonBoundActionButton.h"
#include "BaseFPSBoundActionButton.generated.h"

/**
 * 
 */
UCLASS(Abstract, meta = (DisableNativeTick))
class BASEFPS_API UBaseFPSBoundActionButton : public UCommonBoundActionButton
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

private:
	void HandleInputMethodChanged(ECommonInputType NewInputMethod);

	UPROPERTY(EditAnywhere, Category="Styles")
	TSubclassOf<UCommonButtonStyle> KeyboardStyle;

	UPROPERTY(EditAnywhere, Category="Styles")
	TSubclassOf<UCommonButtonStyle> GamepadStyle;

	UPROPERTY(EditAnywhere, Category="Styles")
	TSubclassOf<UCommonButtonStyle> TouchStyle;
};
