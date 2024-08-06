// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponAttachment.generated.h"

class ABaseFPSCharacter;
class AWeapon;
/**
 * This is the world/3rd person representation of a weapon
 * Note: does not spawn/exist on dedicated servers (since it's visual only)
 */
UCLASS()
class BASEFPS_API AWeaponAttachment : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponAttachment(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	//~ Begin AActor interface
	virtual void Destroyed() override;
	//~ End AActor interface
	
protected:
	UPROPERTY(Transient)
	ABaseFPSCharacter* CharacterOwner;

	/************************************************************************/
	/* Visuals                                                              */
	/************************************************************************/
	
	/** The world mesh for this attachment */
	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	USkeletalMeshComponent* Mesh;

	/** the attach point when this weapon is equipped */
	UPROPERTY(VisibleDefaultsOnly, Category="Weapon")
	FName EquippedAttachSocket;
	
	/** the attach point when this weapon is holstered */
	UPROPERTY(EditDefaultsOnly, Category="Weapon", meta=(GetOptions="GetHolsterAttachPoints"))
	FName HolsterAttachSocket;
	
protected:
	// strictly used for UPROPERTY meta options
	UFUNCTION(BlueprintCallable, Category="Weapon")
	TArray<FName> GetHolsterAttachPoints() const;
	
public:
	/** attach to character as equipped */
	void AttachToOwnerEquipped();

	/** attach to character as holstered */
	void AttachToOwnerHolstered();

	/** detaches from character */
	void DetachFromOwner();

	/** plays effects like particles and sound on weapon fire */
	virtual void PlayFiringEffects(uint8 FireMode);
	
	void SpawnTrailEffect(uint8 FireMode, const FVector& EndPoint);
	void SpawnImpactEffects(uint8 FireMode, const FVector& EndPoint);

	/** get the muzzle location of the weapon */
	FVector GetMuzzleLocation() const;

	/** get direction of weapon's muzzle */
	FVector GetMuzzleDirection() const;
	
	/************************************************************************/
	/* Animations                                                           */
	/************************************************************************/
protected:
	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	UAnimMontage* FireAnim;

	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	UAnimMontage* BringUpAnim;
	
	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	UAnimMontage* PutDownAnim;

	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	UAnimMontage* ReloadAnim;
public:
	UAnimMontage* GetFireAnim() const;
	UAnimMontage* GetBringUpAnim() const;
	UAnimMontage* GetPutDownAnim() const;
	UAnimMontage* GetReloadAnim() const;
	
};
