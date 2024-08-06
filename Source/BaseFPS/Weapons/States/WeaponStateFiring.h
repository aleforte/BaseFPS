// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/States/WeaponState.h"
#include "WeaponStateFiring.generated.h"

/**
 * 
 */
UCLASS()
class BASEFPS_API UWeaponStateFiring : public UWeaponState
{
	GENERATED_BODY()

public:
	// Constructor
	UWeaponStateFiring(const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer)
	{}

protected:
	FTimerHandle RefireCheckHandle;

public:
	//~Begin UWeaponState interface
	virtual void BeginState(UWeaponState* PrevState) override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndState() override;
	//~End UWeaponState interface

	/** calls fire shot on weapon */
	virtual void FireShot();
	
	/** called after refire delay -- usually either fire or go back to active state */
	virtual void RefireCheck();
	
};
