// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "InteractionCard.generated.h"

class UImage;
class UCommonTextBlock;
class UInteractableComponent;

/**
 * Interaction Card displayed on player's HUD when they focus on an interactable object
 */
UCLASS(Abstract, Blueprintable, ClassGroup = UI)
class BASEFPS_API UInteractionCard : public UCommonUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UImage> InteractableIcon;

	UPROPERTY(EditDefaultsOnly, Category="Interaction Card", meta=(ClampMin=64))
	int32 MaxIconWidth;
	UPROPERTY(EditDefaultsOnly, Category="Interaction Card", meta=(ClampMin=64))
	int32 DesiredIconHeight;
	
	/** Displays when the weapon/inventory slot is empty */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> InteractableText;
	
protected:
	/** builds the formatted text to display in-game */
	FText BuildFormattedDisplayText(const FText* InteractKeyName, const UInteractableComponent* Interactable);
	
public:
	virtual void ShowPromptForInteractable(const FText* InteractionKeyName, const UInteractableComponent* Interactable);
};
