// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Messaging/CommonMessagingSubsystem.h"
#include "BaseFPSUIMessagingSubsystem.generated.h"

class UCommonGameDialog;
/**
 * 
 */
UCLASS()
class BASEFPS_API UBaseFPSUIMessagingSubsystem : public UCommonMessagingSubsystem
{
	GENERATED_BODY()

public:
	UBaseFPSUIMessagingSubsystem();

protected:
	UPROPERTY(EditDefaultsOnly, Category="MessageDialog")
	TSubclassOf<UCommonGameDialog> ConfirmationDialogClass;

	UPROPERTY(EditDefaultsOnly, Category="MessageDialog")
	TSubclassOf<UCommonGameDialog> ErrorDialogClass;

public:
	virtual void ShowConfirmation(UCommonGameDialogDescriptor* DialogDescriptor, FCommonMessagingResultDelegate ResultCallback) override;
	virtual void ShowError(UCommonGameDialogDescriptor* DialogDescriptor, FCommonMessagingResultDelegate ResultCallback) override;
};
