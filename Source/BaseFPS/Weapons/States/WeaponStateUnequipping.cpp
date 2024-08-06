// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/States/WeaponStateUnequipping.h"

#include "BaseFPS.h"
#include "WeaponStateEquipping.h"
#include "Weapons/WeaponAttachment.h"

void UWeaponStateUnequipping::BeginState(UWeaponState* PrevState)
{
	UnequipTimeElapsed = 0.0f;

	UWeaponStateEquipping* PrevEquip = Cast<UWeaponStateEquipping>(PrevState);
	UnequipTime = (PrevEquip != nullptr)
		? FMath::Min(PrevEquip->GetPartialEquipTime(), OuterWeapon->GetPutDownTime())
		: OuterWeapon->GetPutDownTime();
	
	if (UnequipTime <= 0.f)
	{
		PutDownFinished();
	}
	else
	{
		const float AnimPlayRate = GetScaledAnimDuration(OuterWeapon->PutDownAnim) / UnequipTime;
		GetWorldTimerManager().SetTimer(PutDownFinishedHandle, this, &ThisClass::PutDownFinished, UnequipTime);
		GetCharacterOwner()->PlayAnimMontage1P(OuterWeapon->PutDownAnim, AnimPlayRate);

		const AWeaponAttachment* CurrAttachment = GetCharacterOwner()->GetEquippedWeaponAttachment(); 
		if (CurrAttachment && CurrAttachment->GetPutDownAnim())
		{
			// handles 3PP anims for local characters, i.e. all characters on server & client's controlled character
			// for simulated proxies, we use ABaseFPSCharacter::OnRep_EquippedWeaponClass to start unequip animation
			GetCharacterOwner()->PlayAnimMontage(CurrAttachment->GetPutDownAnim(), GetScaledAnimDuration(CurrAttachment->GetPutDownAnim()) / UnequipTime);
		}
	}
}

void UWeaponStateUnequipping::EndState()
{
	GetCharacterOwner()->StopAnimMontage1P(OuterWeapon->PutDownAnim);
	GetWorldTimerManager().ClearTimer(PutDownFinishedHandle);
}

void UWeaponStateUnequipping::BringUp(float OverflowTime /*=0.0f*/)
{
	PartialUnequipTime = FMath::Max(0.01f, GetWorldTimerManager().GetTimerElapsed(PutDownFinishedHandle));
	OuterWeapon->GoToEquippingState(OverflowTime);
}

void UWeaponStateUnequipping::Tick(float DeltaTime)
{
	UnequipTimeElapsed += DeltaTime;
	if (UnequipTimeElapsed > UnequipTime)
	{
		PutDownFinished();
	}
}

void UWeaponStateUnequipping::PutDownFinished()
{
	OuterWeapon->DetachMeshFromPawn();
	float OverflowTime = FMath::Max(0.0f, UnequipTimeElapsed - UnequipTime);
	if (GetCharacterOwner())
	{
		GetCharacterOwner()->WeaponChanged(OverflowTime);		
	}
	OuterWeapon->GotoState(OuterWeapon->InactiveState);	
}

/************************************************************************/
/* Accessors                                                            */
/************************************************************************/

float UWeaponStateUnequipping::GetUnequipTime() const
{
	return UnequipTime;
}

float UWeaponStateUnequipping::GetUnequipTimeElapsed() const
{
	return UnequipTimeElapsed;
}

float UWeaponStateUnequipping::GetPartialUnequipTime() const
{
	return PartialUnequipTime;
}
