// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/States/WeaponState.h"
#include "WeaponStateEquipping.generated.h"

/**
 * 
 */
UCLASS(CustomConstructor)
class BASEFPS_API UWeaponStateEquipping : public UWeaponState
{
	GENERATED_BODY()

	friend AWeapon;
	
public:
	// Constructor
	UWeaponStateEquipping(const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer)
	{}
	
protected:
	/** the total time to bring up weapon */
	float EquipTime;

	/** time elapsed after exiting equip early */
	float PartialEquipTime;

	/** handle for managing duration of equipping state */
	FTimerHandle BringUpFinishedHandle;

public:
	//~Begin UWeaponState interface
	virtual void BeginState(UWeaponState* PrevState) override;
	virtual void EndState() override;
	virtual void Tick(float DeltaTime) override;
	virtual bool PutDown() override;
	//~End UWeaponState interface
	
	void StartEquip(float OverflowTime);
	void BringUpFinished();

	/************************************************************************/
	/* Accessors                                                            */
	/************************************************************************/
public:
	float GetEquipTime() const;
	float GetPartialEquipTime() const;
	
	
};
