// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InventoryBarSlot.h"

#include "CommonNumericTextBlock.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Weapons/Weapon.h"

static FName TextureParam = "TextureMask";

void UInventoryBarSlot::SetDisplayedItem(AInventory* InInventory)
{
	DisplayedItem = InInventory;
	if (DisplayedItem)
	{
		if (AWeapon* Weapon = Cast<AWeapon>(DisplayedItem))
		{
			int32 TotalAmmo = Weapon->GetCurrentAmmoInClip() + Weapon->GetCurrentReserveAmmo();
			SetAmmoCount(TotalAmmo);

			if (UTexture2D* Icon = Weapon->GetPrimaryIcon())
			{
				WeaponCard->GetDynamicMaterial()->SetTextureParameterValue(TextureParam, Icon);
			}
			if (UTexture2D* Icon = Weapon->GetAmmoIcon())
			{
				AmmoIcon->GetDynamicMaterial()->SetTextureParameterValue(TextureParam, Icon);
			}
		}
		ShowAsInactive(); // default to inactive
	}
	else
	{
		DisplayEmptyText();
	}
}

AInventory* UInventoryBarSlot::GetDisplayedItem() const
{
	return DisplayedItem;
}

bool UInventoryBarSlot::IsDisplaying(AInventory* TestInv) const
{
	return TestInv == DisplayedItem;
}

void UInventoryBarSlot::SetActive(bool bIsActive) const
{
	if (DisplayedItem)
	{
		if (bIsActive)
		{
			ShowAsActive();
		}
		else
		{
			ShowAsInactive();
		}		
	}
	else
	{
		DisplayEmptyText();
	}
}

void UInventoryBarSlot::SetAmmoCount(int32 InAmount) const
{
	AmmoCount->SetCurrentValue(InAmount);
}

void UInventoryBarSlot::ShowAsActive() const
{
	WeaponCard->SetVisibility(ESlateVisibility::HitTestInvisible);
	AmmoIcon->SetVisibility(ESlateVisibility::HitTestInvisible);
	AmmoCount->SetVisibility(ESlateVisibility::HitTestInvisible);
	EmptyText->SetVisibility(ESlateVisibility::Hidden);
	
	WeaponCard->SetRenderOpacity(1.f);
	AmmoIcon->SetRenderOpacity(1.f);
	AmmoCount->SetRenderOpacity(1.f);
	BackgroundBorder->SetRenderOpacity(1.f);
}

void UInventoryBarSlot::ShowAsInactive() const
{
	WeaponCard->SetVisibility(ESlateVisibility::HitTestInvisible);
	AmmoIcon->SetVisibility(ESlateVisibility::HitTestInvisible);
	AmmoCount->SetVisibility(ESlateVisibility::HitTestInvisible);
	EmptyText->SetVisibility(ESlateVisibility::Hidden);
	
	WeaponCard->SetRenderOpacity(0.4f);
	AmmoIcon->SetRenderOpacity(0.4f);
	AmmoCount->SetRenderOpacity(0.4f);
	BackgroundBorder->SetRenderOpacity(0.3f);
	EmptyText->SetVisibility(ESlateVisibility::Hidden);
}

void UInventoryBarSlot::DisplayEmptyText() const
{
	WeaponCard->SetVisibility(ESlateVisibility::Hidden);
	AmmoIcon->SetVisibility(ESlateVisibility::Hidden);
	AmmoCount->SetVisibility(ESlateVisibility::Hidden);
	EmptyText->SetVisibility(ESlateVisibility::HitTestInvisible);
	BackgroundBorder->SetRenderOpacity(0.3f);
}

