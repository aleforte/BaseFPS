// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickups/PickupInstance.h"
#include "PickupInstance_Shield.generated.h"

/**
 * Shield Pickup
 */
UCLASS(Abstract, Blueprintable, NotPlaceable)
class BASEFPS_API APickupInstance_Shield : public APickupInstance
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	APickupInstance_Shield(const FObjectInitializer& ObjectInitializer);
	
	/************************************************************************/
	/* Properties                                                           */
	/************************************************************************/
	protected:
	/** Shield icon to appear in the HUD */
	UPROPERTY(EditDefaultsOnly, Category="HUD")
	UTexture2D* ShieldIcon;
	
	UPROPERTY(EditDefaultsOnly, Category="Pickup", meta=(ClampMin=0, ClampMax=999))
	uint32 ShieldAmount;
	
	/************************************************************************/
	/* Visuals                                                              */
	/************************************************************************/
	protected:
	UPROPERTY(EditDefaultsOnly, Category="Pickup")
	UStaticMeshComponent* Mesh;
	
public:
	virtual UMeshComponent* GetEditorMeshTemplate() const override;	
};
