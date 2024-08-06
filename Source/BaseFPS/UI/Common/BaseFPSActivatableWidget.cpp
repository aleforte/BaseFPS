// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseFPSActivatableWidget.h"

#include "Editor/WidgetCompilerLog.h"
#include "CommonInputModeTypes.h"
#include "Input/UIActionBindingHandle.h"

#define LOCTEXT_NAMESPACE "BaseFPS"

UBaseFPSActivatableWidget::UBaseFPSActivatableWidget(const FObjectInitializer& ObjectInitializer)
	: Super (ObjectInitializer)
{
}

TOptional<FUIInputConfig> UBaseFPSActivatableWidget::GetDesiredInputConfig() const
{
	switch (InputConfig)
	{
	case EBaseFPSWidgetInputMode::GameAndMenu:
		return FUIInputConfig(ECommonInputMode::All, GameMouseCaptureMode);
	case EBaseFPSWidgetInputMode::Game:
		return FUIInputConfig(ECommonInputMode::Game, GameMouseCaptureMode);
	case EBaseFPSWidgetInputMode::Menu:
		return FUIInputConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture);
	case EBaseFPSWidgetInputMode::Default:
	default:
		return TOptional<FUIInputConfig>();
	}
}

#if WITH_EDITOR

void UBaseFPSActivatableWidget::ValidateCompiledWidgetTree(const UWidgetTree& BlueprintWidgetTree, class IWidgetCompilerLog& CompileLog) const
{
	Super::ValidateCompiledWidgetTree(BlueprintWidgetTree, CompileLog);

	if (!GetClass()->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(UBaseFPSActivatableWidget, BP_GetDesiredFocusTarget)))
	{
		if (GetParentNativeClass(GetClass()) == UBaseFPSActivatableWidget::StaticClass())
		{
			CompileLog.Warning(LOCTEXT("ValidateGetDesiredFocusTarget_Warning", "GetDesiredFocusTarget wasn't implemented, you're going to have trouble using gamepads on this screen."));
		}
		else
		{
			//TODO - Note for now, because we can't guarantee it isn't implemented in a native subclass of this one.
			CompileLog.Note(LOCTEXT("ValidateGetDesiredFocusTarget_Note", "GetDesiredFocusTarget wasn't implemented, you're going to have trouble using gamepads on this screen.  If it was implemented in the native base class you can ignore this message."));
		}
	}
}

#endif


#undef LOCTEXT_NAMESPACE
