// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/States/WeaponStateEquipping.h"

#include "BaseFPS.h"
#include "WeaponStateUnequipping.h"
#include "Weapons/Weapon.h"
#include "Weapons/WeaponAttachment.h"

void UWeaponStateEquipping::BeginState(UWeaponState* PrevState)
{
	UWeaponStateUnequipping* PrevEquip = Cast<UWeaponStateUnequipping>(PrevState);
	EquipTime = (PrevEquip != nullptr)
		? FMath::Min(PrevEquip->GetPartialUnequipTime(), OuterWeapon->GetBringUpTime())
		: OuterWeapon->GetBringUpTime();
	
	if (GetCharacterOwner())
	{
		GetCharacterOwner()->SetCurrentAnimPose(OuterWeapon->GetWeaponAnimPose());
	}
}

void UWeaponStateEquipping::EndState()
{
	GetCharacterOwner()->StopAnimMontage1P(OuterWeapon->BringUpAnim);
	GetWorldTimerManager().ClearTimer(BringUpFinishedHandle);
	GetWorldTimerManager().ClearAllTimersForObject(OuterWeapon);
}

void UWeaponStateEquipping::Tick(float DeltaTime)
{
	if (!GetWorldTimerManager().IsTimerActive(BringUpFinishedHandle))
	{
		BringUpFinished();
	}
}

bool UWeaponStateEquipping::PutDown()
{
	PartialEquipTime = FMath::Max(0.01f, GetWorldTimerManager().GetTimerElapsed(BringUpFinishedHandle));
	OuterWeapon->GotoState(OuterWeapon->UnequippingState);
	return true;
}

void UWeaponStateEquipping::StartEquip(float OverflowTime)
{
	EquipTime -= OverflowTime; // keeps weapon swapping in sync
		
	// wait a tick to ensure the anim blueprint switches to the appropriate pose first
	GetWorldTimerManager().SetTimerForNextTick(OuterWeapon, &AWeapon::AttachMeshToPawn);
	if (EquipTime <= 0.0f)
	{
		BringUpFinished();
	}
	else
	{
		const float AnimPlayRate = GetScaledAnimDuration(OuterWeapon->BringUpAnim) / EquipTime;
		GetCharacterOwner()->PlayAnimMontage1P(OuterWeapon->BringUpAnim, AnimPlayRate);
		GetWorldTimerManager().SetTimer(BringUpFinishedHandle, this, &UWeaponStateEquipping::BringUpFinished, EquipTime);

		const AWeaponAttachment* CurrAttachment = GetCharacterOwner()->GetEquippedWeaponAttachment(); 
		if (CurrAttachment && CurrAttachment->GetBringUpAnim())
		{
			// handles 3PP anims for local characters, i.e. all characters on server & client's controlled character
			// for simulated proxies, we use ABaseFPSCharacter::OnRep_EquippedWeaponClass to start unequip animation
			GetCharacterOwner()->PlayAnimMontage(CurrAttachment->GetBringUpAnim(), GetScaledAnimDuration(CurrAttachment->GetBringUpAnim()) / EquipTime);
		}
	}
}

void UWeaponStateEquipping::BringUpFinished()
{
	OuterWeapon->GotoState(OuterWeapon->ActiveState);
}

/************************************************************************/
/* Accessors                                                            */
/************************************************************************/

float UWeaponStateEquipping::GetEquipTime() const
{
	return EquipTime;
}

float UWeaponStateEquipping::GetPartialEquipTime() const
{
	return PartialEquipTime;
}
