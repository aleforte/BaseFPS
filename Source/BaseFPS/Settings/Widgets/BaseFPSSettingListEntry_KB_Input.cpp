// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseFPSSettingListEntry_KB_Input.h"

#include "BaseFPS.h"
#include "UI/Foundation/BaseFPSButtonBase.h"
#include "CommonUIExtensions.h"
#include "Settings/CustomSettings/BaseFPSSettingKeyboardInput.h"
#include "Widgets/Misc/KeyAlreadyBoundWarning.h"
#include "Widgets/Misc/GameSettingPressAnyKey.h"

#define LOCTEXT_NAMESPACE "BaseFPSSettings"

void UBaseFPSSettingListEntry_KB_Input::SetSetting(UGameSetting* InSetting)
{
	KeyboardInputSetting = CastChecked<UBaseFPSSettingKeyboardInput>(InSetting);

	Super::SetSetting(InSetting);

	Refresh();
}

void UBaseFPSSettingListEntry_KB_Input::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Button_PrimaryKey->OnClicked().AddUObject(this, &ThisClass::HandlePrimaryKeyClicked);
	Button_SecondaryKey->OnClicked().AddUObject(this, &ThisClass::HandleSecondaryKeyClicked);
	Button_Clear->OnClicked().AddUObject(this, &ThisClass::HandleClearClicked);
}

void UBaseFPSSettingListEntry_KB_Input::NativeOnEntryReleased()
{
	Super::NativeOnEntryReleased();

	KeyboardInputSetting = nullptr;
}

void UBaseFPSSettingListEntry_KB_Input::OnSettingChanged()
{
	Refresh();
}

void UBaseFPSSettingListEntry_KB_Input::HandlePrimaryKeyClicked()
{
	UGameSettingPressAnyKey* PressAnyKeyPanel = CastChecked<UGameSettingPressAnyKey>(
		UCommonUIExtensions::PushContentToLayer_ForPlayer(GetOwningLocalPlayer(), FrontendTags::TAG_UI_LAYER_MODAL, PressAnyKeyPanelClass));
	PressAnyKeyPanel->OnKeySelected.AddUObject(this, &ThisClass::HandlePrimaryKeySelected, PressAnyKeyPanel);
	PressAnyKeyPanel->OnKeySelectionCanceled.AddUObject(this, &ThisClass::HandleKeySelectionCanceled, PressAnyKeyPanel);
}

void UBaseFPSSettingListEntry_KB_Input::HandleSecondaryKeyClicked()
{
	UGameSettingPressAnyKey* PressAnyKeyPanel = CastChecked<UGameSettingPressAnyKey>(
	UCommonUIExtensions::PushContentToLayer_ForPlayer(GetOwningLocalPlayer(), FrontendTags::TAG_UI_LAYER_MODAL, PressAnyKeyPanelClass));
	PressAnyKeyPanel->OnKeySelected.AddUObject(this, &ThisClass::HandleSecondaryKeySelected, PressAnyKeyPanel);
	PressAnyKeyPanel->OnKeySelectionCanceled.AddUObject(this, &ThisClass::HandleKeySelectionCanceled, PressAnyKeyPanel);
}

void UBaseFPSSettingListEntry_KB_Input::HandleClearClicked()
{
	KeyboardInputSetting->ChangeBinding(0, EKeys::Invalid);
	KeyboardInputSetting->ChangeBinding(1, EKeys::Invalid);
}

void UBaseFPSSettingListEntry_KB_Input::HandlePrimaryKeySelected(FKey InKey, UGameSettingPressAnyKey* PressAnyKeyPanel)
{
	PressAnyKeyPanel->OnKeySelected.RemoveAll(this);
	ChangeBinding(0, InKey);
}

void UBaseFPSSettingListEntry_KB_Input::HandleSecondaryKeySelected(FKey InKey, UGameSettingPressAnyKey* PressAnyKeyPanel)
{
	PressAnyKeyPanel->OnKeySelected.RemoveAll(this);
	ChangeBinding(1, InKey);
}

void UBaseFPSSettingListEntry_KB_Input::HandlePrimaryDuplicateKeySelected(FKey InKey,
	UKeyAlreadyBoundWarning* DuplicateKeyPressAnyKeyPanel) const
{
	DuplicateKeyPressAnyKeyPanel->OnKeySelected.RemoveAll(this);
	KeyboardInputSetting->ChangeBinding(0, OriginalKeyToBind);
}

void UBaseFPSSettingListEntry_KB_Input::HandleSecondaryDuplicateKeySelected(FKey InKey,
	UKeyAlreadyBoundWarning* DuplicateKeyPressAnyKeyPanel) const
{
	DuplicateKeyPressAnyKeyPanel->OnKeySelected.RemoveAll(this);
	KeyboardInputSetting->ChangeBinding(1, OriginalKeyToBind);
}

void UBaseFPSSettingListEntry_KB_Input::ChangeBinding(int32 InKeyBindSlot, FKey InKey)
{
	OriginalKeyToBind = InKey;
	TArray<FName> ActionsForKey;
	KeyboardInputSetting->GetAllMappedActionsFromKey(InKeyBindSlot, InKey, ActionsForKey);
	if (!ActionsForKey.IsEmpty())
	{
		UKeyAlreadyBoundWarning* KeyAlreadyBoundWarning = CastChecked<UKeyAlreadyBoundWarning>(
			UCommonUIExtensions::PushContentToLayer_ForPlayer(GetOwningLocalPlayer(), FGameplayTag::EmptyTag, KeyAlreadyBoundWarningPanelClass));

		FString ActionNames;
		for (FName ActionName : ActionsForKey)
		{
			ActionNames += ActionName.ToString() += ", ";
		}

		FFormatNamedArguments Args;
		Args.Add(TEXT("InKey"), InKey.GetDisplayName());
		Args.Add(TEXT("ActionNames"), FText::FromString(ActionNames));

		KeyAlreadyBoundWarning->SetWarningText(FText::Format(LOCTEXT("WarningText", "{InKey} is already bound to {ActionNames} are you sure you want to rebind it?"), Args));
		KeyAlreadyBoundWarning->SetCancelText(FText::Format(LOCTEXT("CancelText", "Press escape to cancel, or press {InKey} again to confirm rebinding"), Args));

		if (InKeyBindSlot == 1)
		{
			KeyAlreadyBoundWarning->OnKeySelected.AddUObject(this, &ThisClass::HandleSecondaryDuplicateKeySelected, KeyAlreadyBoundWarning);
		}
		else
		{
			KeyAlreadyBoundWarning->OnKeySelected.AddUObject(this, &ThisClass::HandlePrimaryDuplicateKeySelected, KeyAlreadyBoundWarning);
		}
		KeyAlreadyBoundWarning->OnKeySelectionCanceled.AddUObject(this, &ThisClass::HandleKeySelectionCanceled, KeyAlreadyBoundWarning);		
	}
	else
	{
		KeyboardInputSetting->ChangeBinding(InKeyBindSlot, InKey);
	}
}

void UBaseFPSSettingListEntry_KB_Input::HandleKeySelectionCanceled(UGameSettingPressAnyKey* PressAnyKeyPanel)
{
	PressAnyKeyPanel->OnKeySelectionCanceled.RemoveAll(this);
}

void UBaseFPSSettingListEntry_KB_Input::HandleKeySelectionCanceled(UKeyAlreadyBoundWarning* PressAnyKeyPanel)
{
	PressAnyKeyPanel->OnKeySelectionCanceled.RemoveAll(this);
}

void UBaseFPSSettingListEntry_KB_Input::Refresh()
{
	if (ensure(KeyboardInputSetting))
	{
		Button_PrimaryKey->SetButtonText(KeyboardInputSetting->GetPrimaryKeyText());
		Button_SecondaryKey->SetButtonText(KeyboardInputSetting->GetSecondaryKeyText());
	}
}

#undef LOCTEXT_NAMESPACE
