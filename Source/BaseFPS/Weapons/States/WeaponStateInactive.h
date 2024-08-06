// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/States/WeaponState.h"
#include "WeaponStateInactive.generated.h"

/**
 * 
 */
UCLASS(CustomConstructor)
class BASEFPS_API UWeaponStateInactive : public UWeaponState
{
	GENERATED_BODY()

public:
	// Constructor
	UWeaponStateInactive(const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer)
	{}

public:
	//~Begin UWeaponState interface
	virtual void BeginState(UWeaponState* PrevState) override;
	virtual void EndState() override;
	virtual void BringUp(float OverflowTime = 0.0f) override;
	//~End UWeaponState interface
};
