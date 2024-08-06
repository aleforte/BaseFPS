// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/States/WeaponState.h"
#include "WeaponStateReloading.generated.h"

/**
 * 
 */
UCLASS()
class BASEFPS_API UWeaponStateReloading : public UWeaponState
{
	GENERATED_BODY()
	
public:
	// Constructor
	UWeaponStateReloading(const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer)
	{}

protected:
	/** handle for managing duration of reload state */
	FTimerHandle ReloadHandle;
	
public:
	//~ Begin UWeaponState interface
	virtual void BeginState(UWeaponState* PrevState) override;
	virtual void EndState() override;
	//~ End UWeaponState interface
	
	void ReloadFinished();
};
