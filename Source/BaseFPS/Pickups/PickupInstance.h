// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupInstance.generated.h"

class ABaseFPSCharacter;
class UInteractableComponent;
class USphereComponent;

/**
 * This is the pickup object that is spawned in the game world by the {@see Pickup} class. These
 * are what players will see and interact with in-game. We're using spawned instances (instead of
 * a persistent class) because it provides the flexibility of movable pickups, e.g. physics-based pickups
 */
UCLASS(Abstract, NotBlueprintable, NotPlaceable)
class BASEFPS_API APickupInstance : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APickupInstance(const FObjectInitializer& ObjectInitializer);

protected:
	// Called after all components are initialized
	virtual void PostInitializeComponents() override;
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/************************************************************************/
	/* Properties                                                           */
	/************************************************************************/
protected:
	UPROPERTY(EditDefaultsOnly, Category="Pickup")
	float RespawnTime;

public:
	float GetRespawnTime() const;
	
	/************************************************************************/
	/* Visuals                                                              */
	/************************************************************************/
	
	/** plays effects on pick up by character */
	virtual void PlayEffectsOnGiveTo();

	/** plays effects on spawn */
	virtual void PlayEffectsOnSpawn();

public:
	/** the template used to create the editor mesh for {@code Pickup} actor */
	UFUNCTION(BlueprintPure, Category="Pickup")
	virtual UMeshComponent* GetEditorMeshTemplate() const;
	
	/************************************************************************/
	/* Pickup Interaction                                                   */
	/************************************************************************/
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Pickup")
	UInteractableComponent* InteractableComponent;

	/** collision mesh used by characters to detect and interact with actor */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pickup")
	USphereComponent* InteractCollision;

protected:
	/** called after successful interaction by character with interaction component */
	void OnInteract(ABaseFPSCharacter* Character);

	/** gives pickup to character */
	virtual void GiveTo(ABaseFPSCharacter* Character);
	
	/** checks to see if pickup can be acquired by a specified character */
	bool CanBePickedUpBy(ABaseFPSCharacter* Character);

public:
	/** the template used to create the editor collision for {@code Pickup} actor */
	UFUNCTION(BlueprintPure, Category="Pickup")
	virtual UShapeComponent* GetEditorCollisionTemplate() const;
	
	/************************************************************************/
	/* Overlap                                                              */
	/************************************************************************/
protected:	
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	virtual void OnOverlap(ABaseFPSCharacter* Character);

};
