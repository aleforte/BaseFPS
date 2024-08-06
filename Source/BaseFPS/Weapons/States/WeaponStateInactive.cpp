// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/States/WeaponStateInactive.h"

#include "Weapons/Weapon.h"

void UWeaponStateInactive::BeginState(UWeaponState* PrevState)
{
	// clients will force to InactiveState before BeginPlay, so need OuterWeapon check
	if (OuterWeapon)
	{
		UE_LOG(LogTemp, Log, TEXT("Begin Inactive State (Weapon=%s, bServer=%d)!"), *OuterWeapon->GetName(), OuterWeapon->HasAuthority());
		OuterWeapon->ClearPendingFire();
		OuterWeapon->SetPendingReload(false);
		OuterWeapon->DetachMeshFromPawn();
		OuterWeapon->SetActorTickEnabled(false);
	}
	
	// TODO undo/revert any outer weapon state properties, e.g. zoom
}

void UWeaponStateInactive::EndState()
{
	Super::EndState();
	if (OuterWeapon)
	{
		OuterWeapon->SetActorTickEnabled(true);
	}
}

void UWeaponStateInactive::BringUp(float OverflowTime /*=0.0f*/)
{
	OuterWeapon->GoToEquippingState(OverflowTime);
}
