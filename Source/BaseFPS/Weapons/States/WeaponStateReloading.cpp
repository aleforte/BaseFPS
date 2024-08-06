// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/States/WeaponStateReloading.h"

#include "BaseFPS.h"

void UWeaponStateReloading::BeginState(UWeaponState* PrevState)
{
	OuterWeapon->SetPendingReload(false);
	float ReloadPlayRate = GetScaledAnimDuration(OuterWeapon->ReloadAnim);
	GetCharacterOwner()->PlayAnimMontage1P(OuterWeapon->ReloadAnim, ReloadPlayRate / OuterWeapon->ReloadTime);
	GetWorldTimerManager().SetTimer(ReloadHandle, this, &ThisClass::ReloadFinished, OuterWeapon->ReloadTime);
	GetCharacterOwner()->SetReloadStatus(true);
}

void UWeaponStateReloading::EndState()
{
	GetWorldTimerManager().ClearTimer(ReloadHandle);
}

void UWeaponStateReloading::ReloadFinished()
{
	OuterWeapon->OnReloadFinished();
	OuterWeapon->GotoState(OuterWeapon->ActiveState);
	GetCharacterOwner()->SetReloadStatus(false);
}
