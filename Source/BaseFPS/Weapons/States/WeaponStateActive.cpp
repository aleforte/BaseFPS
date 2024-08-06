// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponStateActive.h"
#include "WeaponStateUnequipping.h"
#include "WeaponStateReloading.h"

#include "Weapons/Weapon.h"

void UWeaponStateActive::BeginState(UWeaponState* PrevState)
{
	Super::BeginState(PrevState);
	UE_LOG(LogTemp, Log, TEXT("Begin Active State (Weapon=%s, bServer=%d)!"), *OuterWeapon->GetName(), OuterWeapon->HasAuthority());
}

bool UWeaponStateActive::PutDown()
{
	OuterWeapon->GotoState(OuterWeapon->UnequippingState);
	return true;
}

bool UWeaponStateActive::BeginFiringSequence(uint8 InFireMode, bool bClientFired)
{
	if (OuterWeapon->FireModes.IsValidIndex(InFireMode) && OuterWeapon->HasAmmoInClip(InFireMode))
	{
		OuterWeapon->SetCurrentFireMode(InFireMode);
		OuterWeapon->GotoState(OuterWeapon->FireModes[InFireMode].FiringState);
		return true; // we'll fire this frame when we switch to firing state, so returning true
	}
	return false;
}

bool UWeaponStateActive::Reload()
{
	if (OuterWeapon->CanReload())
	{
		OuterWeapon->GotoState(OuterWeapon->ReloadingState);
		return true;		
	}
	return false;
}

