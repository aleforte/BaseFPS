// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Input/UIActionBindingHandle.h"
#include "Widgets/GameSettingScreen.h"
#include "BaseFPSSettingScreen.generated.h"

class UGameSettingRegistry;
class UBaseFPSTabListWidgetBase;

/**
 * 
 */
UCLASS(Abstract, meta = (Category="Settings", DisableNativeTick))
class BASEFPS_API UBaseFPSSettingScreen : public UGameSettingScreen
{
	GENERATED_BODY()

public:

protected:
	virtual void NativeOnInitialized() override;
	virtual UGameSettingRegistry* CreateRegistry() override;

	void HandleBackAction();
	void HandleApplyAction();
	void HandleCancelChangesAction();

	virtual void OnSettingsDirtyStateChanged_Implementation(bool bSettingsDirty) override;

protected:
	UPROPERTY(BlueprintReadOnly, Category="Input", meta = (BindWidget, OptionalWidget = true, AllowPrivateAccess = true))
	TObjectPtr<UBaseFPSTabListWidgetBase> TopSettingsTabs;

	UPROPERTY(EditDefaultsOnly)
	FDataTableRowHandle BackInputActionData;

	UPROPERTY(EditDefaultsOnly)
	FDataTableRowHandle ApplyInputActionData;

	UPROPERTY(EditDefaultsOnly)
	FDataTableRowHandle CancelChangesInputActionData;

	FUIActionBindingHandle BackHandle;
	FUIActionBindingHandle ApplyHandle;
	FUIActionBindingHandle CancelChangesHandle;
};
