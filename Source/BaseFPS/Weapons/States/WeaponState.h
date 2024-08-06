// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/Weapon.h"
#include "WeaponState.generated.h"

class ABaseFPSCharacter;
/**
 * 
 */
UCLASS(DefaultToInstanced, EditInlineNew, CustomConstructor, Within=Weapon)
class BASEFPS_API UWeaponState : public UObject
{
	GENERATED_BODY()

public:
	// Constructor
	UWeaponState(const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer)
	{}

protected:
	UPROPERTY(Transient)
	AWeapon* OuterWeapon;

public:
	/** @param PrevState the previous state */
	virtual void BeginState(UWeaponState* PrevState) {}
	virtual void EndState() {}

	virtual void Tick(float DeltaTime) {}
	
	virtual void BringUp(float OverflowTime=0.0f) {}
	virtual bool PutDown() { return false; }

	/** returns true if shot fired this frame */
	virtual bool BeginFiringSequence(uint8 InFireMode, bool bClientFired) { return false; }
	virtual void EndFiringSequence(uint8 FireModeNum) {}

	virtual bool Reload() { return false; }

	/************************************************************************/
	/* Accessors                                                            */
	/************************************************************************/
public:
	void SetOuterWeapon(AWeapon* InWeapon) { OuterWeapon = InWeapon; }

	/************************************************************************/
	/* Helpers                                                              */
	/************************************************************************/
protected:
	ABaseFPSCharacter* GetCharacterOwner() const
	{
		return OuterWeapon->GetCharacterOwner();
	}

	virtual UWorld* GetWorld() const override
	{
		return OuterWeapon->GetWorld();
	}

	FTimerManager& GetWorldTimerManager() const
	{
		return OuterWeapon->GetWorldTimerManager();
	}
};
