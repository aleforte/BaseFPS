// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/States/WeaponState.h"
#include "WeaponStateUnequipping.generated.h"

/**
 * Following UT's approach for tracking {@code UnequipTime} and {@code UnequipTimeElapsed}, using this
 * to calculate the "OverflowTime" so we can deduct this from Equip time on Weapon switching
 * (which I believe will help keep weapon switching in sync between server/client)
 *
 * "OverflowTime" is the amount that our manual timer ({@code UnequipTimeElapsed}) exceeds the {@code UnequipTime},
 * which can occur when the time elapsed during our current frame exceeds the set timer but the delegate is not called
 * until the next frame
 */
UCLASS(CustomConstructor)
class BASEFPS_API UWeaponStateUnequipping : public UWeaponState
{
	GENERATED_BODY()

public:
	// Constructor
	UWeaponStateUnequipping(const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer)
	{}

protected:
	/** total time to put down weapon */
	float UnequipTime;

	/** manual timer used to track the time elapsed during unequip */
	float UnequipTimeElapsed;

	/** time elapsed after exiting unequip early */
	float PartialUnequipTime;

	/** handle for managing duration of unequipping state */
	FTimerHandle PutDownFinishedHandle;
	
public:
	//~Begin UWeaponState interface
	virtual void BeginState(UWeaponState* PrevState) override;
	virtual void EndState() override;
	virtual void Tick(float DeltaTime) override;
	virtual void BringUp(float OverflowTime=0.0f) override;
	//~End UWeaponState interface

	void StartUnequip();
	virtual void PutDownFinished();

	/************************************************************************/
	/* Accessors                                                            */
	/************************************************************************/
public:
	float GetUnequipTime() const;
	float GetUnequipTimeElapsed() const;
	float GetPartialUnequipTime() const;
};
