// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseFPSSettingScreen.h"

#include "CommonLocalPlayer.h"
#include "Settings/BasefPSSettingRegistry.h"
#include "Delegates/Delegate.h"
#include "Input/CommonUIInputTypes.h"

class UGameSettingRegistry;

void UBaseFPSSettingScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	BackHandle = RegisterUIActionBinding(FBindUIActionArgs(BackInputActionData, true, FSimpleDelegate::CreateUObject(this, &ThisClass::HandleBackAction)));
	ApplyHandle = RegisterUIActionBinding(FBindUIActionArgs(ApplyInputActionData, true, FSimpleDelegate::CreateUObject(this, &ThisClass::HandleApplyAction)));
	CancelChangesHandle = RegisterUIActionBinding(FBindUIActionArgs(CancelChangesInputActionData, true, FSimpleDelegate::CreateUObject(this, &ThisClass::HandleCancelChangesAction)));
}

UGameSettingRegistry* UBaseFPSSettingScreen::CreateRegistry()
{
	UBaseFPSSettingRegistry* NewRegistry = NewObject<UBaseFPSSettingRegistry>();

	if (UCommonLocalPlayer* LocalPlayer = CastChecked<UCommonLocalPlayer>(GetOwningLocalPlayer()))
	{
		NewRegistry->Initialize(LocalPlayer);
	}
	
	return NewRegistry;
}

void UBaseFPSSettingScreen::HandleBackAction()
{
	if (AttemptToPopNavigation())
	{
		return;
	}

	ApplyChanges();
	DeactivateWidget();
}

void UBaseFPSSettingScreen::HandleApplyAction()
{
	ApplyChanges();
}

void UBaseFPSSettingScreen::HandleCancelChangesAction()
{
	CancelChanges();
}

void UBaseFPSSettingScreen::OnSettingsDirtyStateChanged_Implementation(bool bSettingsDirty)
{
	if (bSettingsDirty)
	{
		if (!GetActionBindings().Contains(ApplyHandle))
		{
			AddActionBinding(ApplyHandle);
		}
		if (!GetActionBindings().Contains(CancelChangesHandle))
		{
			AddActionBinding(CancelChangesHandle);
		}
	}
	else
	{
		RemoveActionBinding(ApplyHandle);
		RemoveActionBinding(CancelChangesHandle);
	}
}
