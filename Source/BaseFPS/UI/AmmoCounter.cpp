// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/AmmoCounter.h"

#include "CommonTextBlock.h"
#include "Components/Image.h"
#include "Weapons/Weapon.h"

static FName TextureParam = "TextureMask";

void UAmmoCounter::SetDisplayedWeapon(const AWeapon* EquippedWeapon) const
{
	if (EquippedWeapon)
	{
		CurrentAmmoInClip->SetText(FText::AsNumber(EquippedWeapon->GetCurrentAmmoInClip()));
		CurrentAmmoReserve->SetText(FText::AsNumber(EquippedWeapon->GetCurrentReserveAmmo()));
		
		if (UTexture2D* Icon = EquippedWeapon->GetAmmoIcon())
		{
			AmmoIcon->GetDynamicMaterial()->SetTextureParameterValueByInfo(TextureParam, Icon);
		}
	}
}

void UAmmoCounter::SetCurrentAmmoInClipText(int32 InAmount) const
{
	CurrentAmmoInClip->SetText(FText::AsNumber(InAmount));
}

void UAmmoCounter::SetCurrentAmmoReserveText(int32 InAmount) const
{
	CurrentAmmoReserve->SetText(FText::AsNumber(InAmount));
}

