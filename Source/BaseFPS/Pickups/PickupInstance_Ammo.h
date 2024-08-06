// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickups/PickupInstance.h"
#include "PickupInstance_Ammo.generated.h"

class AWeapon;
/**
 * Ammo Pickup
 */
UCLASS(Abstract, Blueprintable, NotPlaceable)
class BASEFPS_API APickupInstance_Ammo : public APickupInstance
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	APickupInstance_Ammo(const FObjectInitializer& ObjectInitializer);

	/************************************************************************/
	/* Editor Only                                                          */
	/************************************************************************/
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
	/************************************************************************/
	/* Properties                                                           */
	/************************************************************************/
protected:
	/** the weapon type this ammo belongs to */
	UPROPERTY(EditDefaultsOnly, Category="Pickup")
	TSubclassOf<AWeapon> WeaponType;
	
	/** ammo icon to appear in the HUD */
	UPROPERTY(EditDefaultsOnly, Category="HUD")
	UTexture2D* AmmoIcon;
	
	UPROPERTY(EditDefaultsOnly, Category="Pickup", meta=(ClampMin=0, ClampMax=999))
	uint32 AmmoAmount;
	
	/************************************************************************/
	/* Visuals                                                              */
	/************************************************************************/
protected:
	UPROPERTY(EditDefaultsOnly, Category="Pickup")
	UStaticMeshComponent* Mesh;
	
public:
	virtual UMeshComponent* GetEditorMeshTemplate() const override;
};
