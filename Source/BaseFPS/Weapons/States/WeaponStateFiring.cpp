// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/States/WeaponStateFiring.h"

void UWeaponStateFiring::BeginState(UWeaponState* PrevState)
{
	GetWorldTimerManager().SetTimer(RefireCheckHandle, this, &ThisClass::RefireCheck, OuterWeapon->GetRefireTime(OuterWeapon->CurrentFireMode), true);
	FireShot();
}

void UWeaponStateFiring::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void UWeaponStateFiring::EndState()
{
	GetWorldTimerManager().ClearAllTimersForObject(this);
}

void UWeaponStateFiring::FireShot()
{
	OuterWeapon->FireShot();
}

void UWeaponStateFiring::RefireCheck()
{
	if (OuterWeapon->HandleContinuedFiring())
	{
		FireShot();
	}
}
