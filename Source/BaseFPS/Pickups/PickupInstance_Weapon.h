// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickups/PickupInstance.h"
#include "PickupInstance_Weapon.generated.h"

class AWeapon;
class USkeletalMeshComponent;

/**
 * Weapon Pickup
 */
UCLASS(Abstract, Blueprintable, NotPlaceable)
class BASEFPS_API APickupInstance_Weapon : public APickupInstance
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APickupInstance_Weapon(const FObjectInitializer& ObjectInitializer);

protected:
	//~Begin AActor interface
	virtual void BeginPlay() override;
	virtual void Destroyed() override;
	//~End AActor interface
	
	/************************************************************************/
	/* Editor Only                                                          */
	/************************************************************************/
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	void WeaponTypeUpdated() const;
#endif

	/************************************************************************/
	/* Properties                                                           */
	/************************************************************************/

	/** The weapon type that this pickup represents */
	UPROPERTY(EditDefaultsOnly, Category="Pickup")
	TSubclassOf<AWeapon> WeaponType;

protected:
	virtual void GiveTo(ABaseFPSCharacter* Character) override;

	/************************************************************************/
	/* Dropped Weapon                                                       */
	/************************************************************************/
protected:
	/** was this pickup the result of a player dropping their weapon? */
	UPROPERTY(Transient)
	uint8 bIsDropped:1;

	/** Pointer to the dropped weapon actor */
	UPROPERTY(Transient)
	AWeapon* DroppedWeapon;

	/** the amount of ammo amount carried when this weapon dropped, to be given to next owner  */
	UPROPERTY(Transient)
	uint8 DroppedAmmoAmount;

	/** how long a dropped pickup remains on the ground before it is destroyed */
	UPROPERTY(EditDefaultsOnly, Category="DroppedPickup")
	float DroppedPickupLifetime;
	
	/** handle for tracking dropped weapon timer */
	FTimerHandle DroppedTimerHandle;
	
public:
	/** sets flag that this dropped weapon pickup and sets necessary pointers/data */
	void SetDroppedWeapon(AWeapon* WeaponToDrop);

	/** called by a dropped pickup if it's lifetime duration expires */
	void OnDroppedPickupLifetimeExpired();
	
	UFUNCTION(BlueprintCallable, Category="Pickup")
	TSubclassOf<AWeapon> GetWeaponType() const;
	
	/************************************************************************/
	/* Visuals                                                              */
	/************************************************************************/
protected:
	UPROPERTY(EditDefaultsOnly, Category="Pickup")
	USkeletalMeshComponent* Mesh;

public:
	virtual UMeshComponent* GetEditorMeshTemplate() const override;
	
};
