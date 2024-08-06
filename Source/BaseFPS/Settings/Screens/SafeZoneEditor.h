// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Input/Reply.h"
#include "CommonInputBaseTypes.h"
#include "UObject/WeakObjectPtrTemplates.h"
#include "Widgets/IGameSettingActionInterface.h"
#include "SafeZoneEditor.generated.h"

class UGameSetting;
class UGameSettingValueScalar;
class UWidgetSwitcher;
class UCommonRichTextBlock;
class UCommonButtonBase;
struct FAnalogInputEvent;
struct FGameplayTag;
struct FGeometry;
struct FPointerEvent;

/**
 * 
 */
UCLASS(Abstract)
class BASEFPS_API USafeZoneEditor : public UCommonActivatableWidget, public IGameSettingActionInterface
{
	GENERATED_BODY()
	
public:
	FSimpleMulticastDelegate OnSafeZoneSet;

public:
	USafeZoneEditor(const FObjectInitializer& Initializer);

	//~Begin IGameSettingActionInterface interface
	virtual bool ExecuteActionForSetting_Implementation(FGameplayTag ActionTag, UGameSetting* InSetting) override;
	//~End IGameSettingActionInterface interface
	protected:

	UPROPERTY(EditAnywhere, Category="Restrictions")
	bool bCanCancel = true;

	void NativeOnActivated() override;
	void NativeOnInitialized() override;
	virtual FReply NativeOnAnalogValueChanged(const FGeometry& InGeometry, const FAnalogInputEvent& InAnalogEvent) override;
	virtual FReply NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	void HandleInputModeChanged(ECommonInputType InInputType);

private:
	UFUNCTION()
	void HandleBackClicked();

	UFUNCTION()
	void HandleDoneClicked();

	TWeakObjectPtr<UGameSettingValueScalar> ValueSetting;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = true))
	TObjectPtr<UWidgetSwitcher> Switcher_SafeZoneMessage;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = true))
	TObjectPtr<UCommonRichTextBlock> RichText_Default;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = true))
	TObjectPtr<UCommonButtonBase> Button_Back;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = true))
	TObjectPtr<UCommonButtonBase> Button_Done;
};
