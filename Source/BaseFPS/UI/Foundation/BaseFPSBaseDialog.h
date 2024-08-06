// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Messaging/CommonGameDialog.h"
#include "BaseFPSBaseDialog.generated.h"


class UCommonTextBlock;
class UCommonRichTextBlock;
class UDynamicEntryBox;

/**
 * 
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class BASEFPS_API UBaseFPSBaseDialog : public UCommonGameDialog
{
	GENERATED_BODY()

public:
	virtual void SetupDialog(UCommonGameDialogDescriptor* Descriptor, FCommonMessagingResultDelegate ResultCallback) override;
	virtual void KillDialog() override;

protected:
	virtual void NativeOnInitialized() override;
	virtual void CloseConfirmationWindow(ECommonMessagingResult Result);

#if WITH_EDITOR
	virtual void ValidateCompiledDefaults(IWidgetCompilerLog& CompileLog) const override;
#endif

private:

	FCommonMessagingResultDelegate OnResultCallback;
	
private:
	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Text_Title;

	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<UCommonRichTextBlock> RichText_Description;

	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<UDynamicEntryBox> EntryBox_Buttons;
};
