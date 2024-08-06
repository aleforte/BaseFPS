// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/GameSettingListEntry.h"

#include "BaseFPSSettingListEntry_KB_Input.generated.h"

class UGameSetting;
class UGameSettingPressAnyKey;
class UKeyAlreadyBoundWarning;
class UBaseFPSSettingKeyboardInput;
class UBaseFPSButtonBase;

/**************************************************************************/
/* UBaseFPSSettingsListEntry_Keyboard_Input(shortened to meet max length) */
/**************************************************************************/

UCLASS(Abstract, Blueprintable, meta = (Category="Settings", DisableNativeTick))
class BASEFPS_API UBaseFPSSettingListEntry_KB_Input : public UGameSettingListEntry_Setting
{
	GENERATED_BODY()

public:

	virtual void SetSetting(UGameSetting* InSetting) override;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnEntryReleased() override;
	virtual void OnSettingChanged() override;

	void HandlePrimaryKeyClicked();
	void HandleSecondaryKeyClicked();
	void HandleClearClicked();

	void HandlePrimaryKeySelected(FKey InKey, UGameSettingPressAnyKey* PressAnyKeyPanel);
	void HandleSecondaryKeySelected(FKey InKey, UGameSettingPressAnyKey* PressAnyKeyPanel);
	void HandlePrimaryDuplicateKeySelected(FKey InKey, UKeyAlreadyBoundWarning* DuplicateKeyPressAnyKeyPanel) const;
	void HandleSecondaryDuplicateKeySelected(FKey InKey, UKeyAlreadyBoundWarning* DuplicateKeyPressAnyKeyPanel) const;
	void ChangeBinding(int32 InKeyBindSlot, FKey InKey);
	void HandleKeySelectionCanceled(UGameSettingPressAnyKey* PressAnyKeyPanel);
	void HandleKeySelectionCanceled(UKeyAlreadyBoundWarning* PressAnyKeyPanel);

	void Refresh();

private:
	UPROPERTY(Transient)
	FKey OriginalKeyToBind = EKeys::Invalid;

protected:
	UPROPERTY()
	TObjectPtr<UBaseFPSSettingKeyboardInput> KeyboardInputSetting;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameSettingPressAnyKey> PressAnyKeyPanelClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UKeyAlreadyBoundWarning> KeyAlreadyBoundWarningPanelClass;
	
private: // Bound Widgets
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UBaseFPSButtonBase> Button_PrimaryKey;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UBaseFPSButtonBase> Button_SecondaryKey;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UBaseFPSButtonBase> Button_Clear;
};
