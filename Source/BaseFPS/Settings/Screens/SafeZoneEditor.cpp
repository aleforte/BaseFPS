// Fill out your copyright notice in the Description page of Project Settings.


#include "SafeZoneEditor.h"

#include "CommonButtonBase.h"
#include "Components/WidgetSwitcher.h"
#include "CommonRichTextBlock.h"
#include "GameSettingValueScalar.h"
#include "Settings/BaseFPSSettingsLocal.h"
#include "Widgets/Layout/SSafeZone.h"

#define LOCTEXT_NAMESPACE "BaseFPS"

namespace SafeZoneEditor
{
	const float JoystickDeadZone = 0.2f;
	const float SafeZoneChangeSpeed = 0.1f;	
}

USafeZoneEditor::USafeZoneEditor(const FObjectInitializer& Initializer)
{
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;
}

bool USafeZoneEditor::ExecuteActionForSetting_Implementation(FGameplayTag ActionTag, UGameSetting* InSetting)
{
	TArray<UGameSetting*> ChildSettings = InSetting ? InSetting->GetChildSettings() : TArray<UGameSetting*>();
	if (ChildSettings.Num() > 0 && ChildSettings[0])
	{
		ValueSetting = Cast<UGameSettingValueScalar>(ChildSettings[0]);
	}

	return true;
}

void USafeZoneEditor::NativeOnActivated()
{
	Super::NativeOnActivated();

	SSafeZone::SetGlobalSafeZoneScale(UBaseFPSSettingsLocal::Get()->GetSafeZone());

	Button_Done->OnClicked().AddUObject(this, &USafeZoneEditor::HandleDoneClicked);

	Button_Back->SetVisibility((bCanCancel) ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	if (bCanCancel)
	{
		Button_Back->OnClicked().AddUObject(this, &USafeZoneEditor::HandleBackClicked);
	}
}

void USafeZoneEditor::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	Switcher_SafeZoneMessage->SetActiveWidget(RichText_Default);
}

FReply USafeZoneEditor::NativeOnAnalogValueChanged(const FGeometry& InGeometry, const FAnalogInputEvent& InAnalogEvent)
{
	if (InAnalogEvent.GetKey() == EKeys::Gamepad_LeftY && FMath::Abs(InAnalogEvent.GetAnalogValue()) >= SafeZoneEditor::JoystickDeadZone)
	{
		const float SafeZoneMultiplier = FMath::Clamp(
			SSafeZone::GetGlobalSafeZoneScale().Get(1.0f) + InAnalogEvent.GetAnalogValue() * SafeZoneEditor::SafeZoneChangeSpeed, 0.0f, 1.0f);
		SSafeZone::SetGlobalSafeZoneScale(SafeZoneMultiplier >= 0 ? SafeZoneMultiplier : 0);

		return FReply::Handled();
	}
	return Super::NativeOnAnalogValueChanged(InGeometry, InAnalogEvent);
}

FReply USafeZoneEditor::NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	const float SafeZoneMultiplier = FMath::Clamp(
		SSafeZone::GetGlobalSafeZoneScale().Get(1.0f) + InMouseEvent.GetWheelDelta() * SafeZoneEditor::SafeZoneChangeSpeed, 0.0f, 1.0f);
	SSafeZone::SetGlobalSafeZoneScale(SafeZoneMultiplier >= 0 ? SafeZoneMultiplier : 0);

	return FReply::Handled();
}

void USafeZoneEditor::HandleInputModeChanged(ECommonInputType InInputType)
{
	const FText KeyName = InInputType == ECommonInputType::Gamepad?
		LOCTEXT("SafeZone_KeyToPress_Gamepad", "Left Stick") : LOCTEXT("SafeZone_KeyToPress_Mouse", "Mouse Wheel");
	RichText_Default->SetText(
		FText::Format(LOCTEXT("SafeZoneEditorInstructions", "Use <text color=\"FFFFFFFF\" fontface=\"black\">{0}</> to adjust the corners so it lines up with the edges of your display."), KeyName));
}

void USafeZoneEditor::HandleBackClicked()
{
	DeactivateWidget();
	SSafeZone::SetGlobalSafeZoneScale(UBaseFPSSettingsLocal::Get()->GetSafeZone());
}

void USafeZoneEditor::HandleDoneClicked()
{
	if (ValueSetting.IsValid())
	{
		ValueSetting.Get()->SetValue(SSafeZone::GetGlobalSafeZoneScale().Get(1.0f));
	}
	else
	{
		UBaseFPSSettingsLocal::Get()->SetSafeZone(SSafeZone::GetGlobalSafeZoneScale().Get(1.0f));
	}
	OnSafeZoneSet.Broadcast();
	DeactivateWidget();
}

#undef LOCTEXT_NAMESPACE
