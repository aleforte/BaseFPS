// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/States/WeaponState.h"
#include "WeaponStateActive.generated.h"

/**
 * 
 */
UCLASS(CustomConstructor)
class BASEFPS_API UWeaponStateActive : public UWeaponState
{
	GENERATED_BODY()
	
public:
	// Constructor
	UWeaponStateActive(const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer)
	{}

public:
	//~Begin UWeaponState interface
	virtual void BeginState(UWeaponState* PrevState) override;
	virtual bool PutDown() override;
	virtual bool BeginFiringSequence(uint8 InFireMode, bool bClientFired) override;
	virtual bool Reload() override;
	//~End UWeaponState interface
};
