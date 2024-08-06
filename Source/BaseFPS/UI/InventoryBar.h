// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "InventoryBar.generated.h"

class UInventoryBarSlot;
class UHorizontalBox;
class AInventory;

/**
 * 
 */
UCLASS(ClassGroup = UI)
class BASEFPS_API UInventoryBar : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UHorizontalBox> WeaponBarHB;
	
private:
	/** The widget class that is displayed within the weapon bar */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="InventoryBar", meta=(AllowPrivateAccess))
	TSubclassOf<UInventoryBarSlot> SlotWidgetClass;
	
	/** cached widgets to prevent needing to recreate */
	UPROPERTY(Transient)
	TArray<UInventoryBarSlot*> SlotWidgetCache;
	
public:
	/** refreshes all inventory item cards in inventory bar */
	void SetDisplayedInventory(TArray<AInventory*>& Inv);

	/** sets the specified item card as active in inventory bar */
	void SetActiveItem(AInventory* NewActiveItem) const;
	
	bool ContainsItem(AInventory* Item) const;
	bool ContainsItems() const;

	UInventoryBarSlot* GetInventoryBarSlot(AInventory* Item) const;

protected:
	UFUNCTION(BlueprintCallable, Category="InventoryBar")
	void CreateInventoryCards(int32 Count);
	
	UFUNCTION(BlueprintCallable, Category="InventoryBar")
	void RemoveInventoryCards();

};
