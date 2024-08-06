// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Subsystem/BaseFPSUIMessagingSubsystem.h"

#include "BaseFPS.h"
#include "CommonLocalPlayer.h"
#include "PrimaryGameLayout.h"
#include "Messaging/CommonGameDialog.h"

UBaseFPSUIMessagingSubsystem::UBaseFPSUIMessagingSubsystem()
{
	static ConstructorHelpers::FClassFinder<UCommonGameDialog> ConfirmationDialogClassFinder(TEXT("/Game/UI/Shared/Dialogs/W_ConfirmationDialog"));
	ConfirmationDialogClass = ConfirmationDialogClassFinder.Class;

	static ConstructorHelpers::FClassFinder<UCommonGameDialog> ErrorDialogClassFinder(TEXT("/Game/UI/Shared/Dialogs/W_ConfirmationDialog"));
	ErrorDialogClass = ErrorDialogClassFinder.Class;
}

void UBaseFPSUIMessagingSubsystem::ShowConfirmation(UCommonGameDialogDescriptor* DialogDescriptor, FCommonMessagingResultDelegate ResultCallback)
{
	if (UCommonLocalPlayer* LocalPlayer = GetLocalPlayer<UCommonLocalPlayer>())
	{
		if (UPrimaryGameLayout* RootLayout = LocalPlayer->GetRootUILayout())
		{
			RootLayout->PushWidgetToLayerStack<UCommonGameDialog>(FrontendTags::TAG_UI_LAYER_MODAL, ConfirmationDialogClass,
				[DialogDescriptor, ResultCallback](UCommonGameDialog& Dialog) {
					Dialog.SetupDialog(DialogDescriptor, ResultCallback);
			});
		}
	}
}

void UBaseFPSUIMessagingSubsystem::ShowError(UCommonGameDialogDescriptor* DialogDescriptor, FCommonMessagingResultDelegate ResultCallback)
{
	if (UCommonLocalPlayer* LocalPlayer = GetLocalPlayer<UCommonLocalPlayer>())
	{
		if (UPrimaryGameLayout* RootLayout = LocalPlayer->GetRootUILayout())
		{
			RootLayout->PushWidgetToLayerStack<UCommonGameDialog>(FrontendTags::TAG_UI_LAYER_MODAL, ErrorDialogClass,
				[DialogDescriptor, ResultCallback](UCommonGameDialog& Dialog) {
					Dialog.SetupDialog(DialogDescriptor, ResultCallback);
			});
		}
	}
}
