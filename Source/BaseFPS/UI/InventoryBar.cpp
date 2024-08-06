// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InventoryBar.h"

#include "Components/HorizontalBox.h"
#include "UI/InventoryBarSlot.h"
#include "Weapons/Weapon.h"

void UInventoryBar::SetDisplayedInventory(TArray<AInventory*>& Inv)
{
	RemoveInventoryCards();
	for (int32 i = 0; i < Inv.Num(); i++)
	{
		UInventoryBarSlot* SlotCard = !SlotWidgetCache.IsEmpty()
					? SlotWidgetCache.Pop()
					: CreateWidget<UInventoryBarSlot>(this, SlotWidgetClass);
		SlotCard->SetDisplayedItem(Inv[i]);
		WeaponBarHB->AddChild(SlotCard);
	}
}

void UInventoryBar::SetActiveItem(AInventory* NewActiveItem) const
{
	for (UWidget* Widget : WeaponBarHB->GetAllChildren())
	{
		if (UInventoryBarSlot* SlotCard = Cast<UInventoryBarSlot>(Widget))
		{
			SlotCard->SetActive(SlotCard->IsDisplaying(NewActiveItem));
		}
	}
}

bool UInventoryBar::ContainsItem(AInventory* Item) const
{
	for (UWidget* Widget : WeaponBarHB->GetAllChildren())
	{
		UInventoryBarSlot* SlotCard = Cast<UInventoryBarSlot>(Widget);
		if (SlotCard && Item && SlotCard->IsDisplaying(Item))
		{
			return true;
		}
	}
	return false;
}

bool UInventoryBar::ContainsItems() const
{
	for (UWidget* Widget : WeaponBarHB->GetAllChildren())
	{
		TObjectPtr<UInventoryBarSlot> SlotCard = Cast<UInventoryBarSlot>(Widget);
		if (SlotCard && SlotCard->GetDisplayedItem())
		{
			return true;
		}
	}
	return false;	
}

UInventoryBarSlot* UInventoryBar::GetInventoryBarSlot(AInventory* Item) const
{
	for (UWidget* Widget : WeaponBarHB->GetAllChildren())
	{
		UInventoryBarSlot* SlotCard = Cast<UInventoryBarSlot>(Widget);
		if (SlotCard && Item && SlotCard->IsDisplaying(Item))
		{
			return SlotCard;
		}
	}
	return nullptr;
}

void UInventoryBar::CreateInventoryCards(int32 Count)
{
	if (SlotWidgetClass)
	{
		for (int32 i = 0; i < Count; i++)
		{
			UInventoryBarSlot* SlotCard = !SlotWidgetCache.IsEmpty()
				? SlotWidgetCache.Pop()
				: CreateWidget<UInventoryBarSlot>(this, SlotWidgetClass);
			SlotCard->DisplayEmptyText();
			WeaponBarHB->AddChild(SlotCard);
		}
		SetVisibility(ESlateVisibility::Visible);
	}
}

void UInventoryBar::RemoveInventoryCards()
{
	for (UWidget* Widget : WeaponBarHB->GetAllChildren())
	{
		if (UInventoryBarSlot* SlotCard = Cast<UInventoryBarSlot>(Widget))
		{
			SlotCard->SetDisplayedItem(nullptr);
			SlotWidgetCache.Push(SlotCard);
		}
	}
	WeaponBarHB->ClearChildren();
}
