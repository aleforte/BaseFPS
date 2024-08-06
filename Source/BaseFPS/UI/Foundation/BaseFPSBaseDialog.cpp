// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseFPSBaseDialog.h"

#include "CommonInputSettings.h"
#include "UI/Foundation/BaseFPSButtonBase.h"
#include "CommonRichTextBlock.h"
#include "CommonTextBlock.h"
#include "Components/DynamicEntryBox.h"
#include "ICommonInputModule.h"

void UBaseFPSBaseDialog::SetupDialog(UCommonGameDialogDescriptor* Descriptor,
                                    FCommonMessagingResultDelegate ResultCallback)
{
	Super::SetupDialog(Descriptor, ResultCallback);

	Text_Title->SetText(Descriptor->Header);
	RichText_Description->SetText(Descriptor->Body);

	EntryBox_Buttons->Reset<UBaseFPSButtonBase>([](const UBaseFPSButtonBase& Button)
	{
		Button.OnClicked().Clear();
	});

	for (const FConfirmationDialogAction& Action : Descriptor->ButtonActions)
	{
		FDataTableRowHandle ActionRow;
		switch (Action.Result)
		{
			case ECommonMessagingResult::Confirmed:
				ActionRow = ICommonInputModule::GetSettings().GetDefaultClickAction();
				break;
			case ECommonMessagingResult::Declined:
				ActionRow = ICommonInputModule::GetSettings().GetDefaultBackAction();
				break;
			case ECommonMessagingResult::Cancelled:
				ActionRow = ICommonInputModule::GetSettings().GetDefaultBackAction(); // TODO see Lyra, uses UniversalActions table
				break;
			default:
				ensure(false);
				continue;
		}

		UBaseFPSButtonBase* Button = EntryBox_Buttons->CreateEntry<UBaseFPSButtonBase>();
		Button->SetTriggeredInputAction(ActionRow);
		Button->OnClicked().AddUObject(this, &ThisClass::CloseConfirmationWindow, Action.Result);
		Button->SetButtonText(Action.OptionalDisplayText);
	}

	OnResultCallback = ResultCallback;
}

void UBaseFPSBaseDialog::KillDialog()
{
	Super::KillDialog();
}

void UBaseFPSBaseDialog::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UBaseFPSBaseDialog::CloseConfirmationWindow(ECommonMessagingResult Result)
{
	DeactivateWidget();
	OnResultCallback.ExecuteIfBound(Result);
}

#if WITH_EDITOR
void UBaseFPSBaseDialog::ValidateCompiledDefaults(IWidgetCompilerLog& CompileLog) const
{
	Super::ValidateCompiledDefaults(CompileLog);
}
#endif