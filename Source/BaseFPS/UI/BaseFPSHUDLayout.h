// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Common/BaseFPSActivatableWidget.h"
#include "BaseFPSHUDLayout.generated.h"

class UImage;
class UAmmoCounter;
class UInteractionCard;
class UInventoryBar;

/**
 * A HUD layout is essentially just a canvas of child widgets. It's serves two purposes:
 *   1) sets the size and position of all HUD elements (hence the name layout)
 *   2) provides a clean interface for the player's @code AHUD to access HUD elements
 *
 * (Try to avoid adding complex logic here, try to keep everything in the @code AHUD class)
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class BASEFPS_API UBaseFPSHUDLayout : public UBaseFPSActivatableWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UInventoryBar> InventoryBar;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UAmmoCounter> AmmoCounter;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UInteractionCard> InteractionCard;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UImage> AimPoint;

public:
	UFUNCTION(BlueprintCallable, Category="HUD")
	UInventoryBar* GetInventoryBar() const;

	UFUNCTION(BlueprintCallable, Category="HUD")
	UAmmoCounter* GetAmmoCounter() const;

	UFUNCTION(BlueprintCallable, Category="HUD")
	UInteractionCard* GetInteractionCard() const;

	UFUNCTION(BlueprintCallable, Category="HUD")
	UImage* GetAimPoint() const;
};
