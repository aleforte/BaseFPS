// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "InventoryBarSlot.generated.h"

class AInventory;
class UBorder;
class UCommonTextBlock;
class UCommonNumericTextBlock;
class UImage;

class AWeapon;

/**
 * 
 */
UCLASS(ClassGroup = UI)
class BASEFPS_API UInventoryBarSlot : public UCommonUserWidget
{
	GENERATED_BODY()

protected:
	/** the inventory actor that this card displays */
	UPROPERTY(Transient)
	TObjectPtr<AInventory> DisplayedItem;
	
public:
	void SetDisplayedItem(AInventory* InInventory);
	AInventory* GetDisplayedItem() const;

	UFUNCTION(BlueprintPure, Category="InventoryCard")
	bool IsDisplaying(AInventory* TestInv) const;
	
protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UImage> WeaponCard;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UImage> AmmoIcon;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UCommonNumericTextBlock> AmmoCount;

	/** Displays when the weapon/inventory slot is empty */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> EmptyText;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UBorder> BackgroundBorder;

public:
	void SetActive(bool bIsActive) const;
	void SetAmmoCount(int32 InAmount) const;
	void DisplayEmptyText() const;
	
protected:
	void ShowAsActive() const;
	void ShowAsInactive() const;
	
};
