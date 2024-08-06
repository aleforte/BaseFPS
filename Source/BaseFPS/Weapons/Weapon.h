// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/BaseFPSCharacter.h"
#include "Inventory/Inventory.h"
#include "Weapon.generated.h"

class UWeaponStateReloading;
class UWeaponStateFiring;
class AWeaponAttachment;
class APickupInstance_Weapon;
class UAnimMontage;
class UWeaponState;
class UWeaponStateActive;
class UWeaponStateInactive;
class UWeaponStateEquipping;
class UWeaponStateUnequipping;

USTRUCT(BlueprintType)
struct FFireMode
{
	GENERATED_BODY()

	UPROPERTY(Instanced, EditDefaultsOnly, Category="States")
	UWeaponState* FiringState;
	
	/** firing animations */
	UPROPERTY(EditDefaultsOnly, Category="Animation")
	UAnimMontage* FireAnim;
	
	/** the time between shots (or refire checks) */
	UPROPERTY(EditDefaultsOnly, Category="Weapon", meta = (ClampMin = 0.1f))
	float FiringInterval;
	
	/** the ammo cost to fire a single shot */
	UPROPERTY(EditAnywhere, Category = "Weapon", meta = (ClampMin = 0))
	int32 AmmoCost;

	// Constructor
	FFireMode()
		: FiringState()
		, FireAnim()
		, FiringInterval(0.2f)
		, AmmoCost(1)
	{}
};


/**
 * A weapon is essentially anything wieldable by the player, i.e. equipped and seen
 * in first-person. This means it could be a gun, melee weapon, medkit, etc.
 *
 * NOTE: See {@code WeaponAttachment} for additional weapon variables, like 3rd person mesh and animations
 */
UCLASS(Abstract, NotPlaceable, Blueprintable, Config = Game)
class BASEFPS_API AWeapon : public AInventory
{
	GENERATED_BODY()

	friend UWeaponState;
	friend UWeaponStateActive;
	friend UWeaponStateInactive;
	friend UWeaponStateEquipping;
	friend UWeaponStateUnequipping;
	friend UWeaponStateFiring;
	friend UWeaponStateReloading;
	
public:	
	// Sets default values for this actor's properties
	AWeapon(const FObjectInitializer& ObjectInitializer);

protected:
	//~Begin AActor interface
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void Destroyed() override;
	//~End AActor interface

	//~ Begin AInventory interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~ End AInventory interface
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	/************************************************************************/
	/* Inventory                                                            */
	/************************************************************************/	
protected:
	/** the amount of time to bring up or equip this weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	float BringUpTime;

	/** the amount of time to put down or unequip this weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	float PutDownTime;

public:
	//~ Begin AInventory interface
	virtual void OnAddedToInventory(ABaseFPSCharacter* NewOwner) override;
	virtual void OnRemovedFromInventory() override;
	//~ End AInventory interface
	
	/**
	 * weapon is being equipped by owner
	 * @param OverflowTime (optional) the fraction of DeltaTime that the previously equipped weapon overflowed past its
	 * PutDown timer (only applicable to weapon changes, should ensure server/clients stay in sync)  */
	void BringUp(float OverflowTime=0.0f);

	/** weapon is being unequipped by owner */
	bool PutDown() const;
	
	void AttachMeshToPawn();
	void DetachMeshFromPawn();
	
	/************************************************************************/
	/* Visuals                                                              */
	/************************************************************************/

	/** the name for this weapon */
	UPROPERTY(EditDefaultsOnly, Category="Visuals")
	FText DisplayName;
	
	/** weapon mesh: 1st person view */
	UPROPERTY(VisibleDefaultsOnly, Category="Visuals")
	USkeletalMeshComponent* Mesh;
	
	/** Socket to attach weapon to hands; if none, hands are hidden */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category="Visuals")
	FName HandsAttachSocket;
	
	/** the weapon attachment class for this weapon */
	UPROPERTY(EditDefaultsOnly, Category="Visuals")
	TSubclassOf<AWeaponAttachment> WeaponAttachmentType;
	
	/** the character's weapon pose/stance when holding this weapon */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Visuals")
	EAnimPose WeaponAnimPose;
	
	/************************************************************************/
	/* User Interface                                                       */
	/************************************************************************/
protected:
	/** weapon icon to appear in the HUD */
	UPROPERTY(EditDefaultsOnly, Category="HUD")
	UTexture2D* PrimaryIcon;

	/** ammo icon to appear in the HUD */
	UPROPERTY(EditDefaultsOnly, Category="HUD")
	UTexture2D* AmmoIcon;

	/************************************************************************/
	/* Pickup                                                               */
	/************************************************************************/
protected:
	/** can this weapon be thrown by a player from their inventory? */
	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	bool bCanBeThrown;
	
	/** the pickup class/type for this weapon when dropped by character */
	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	TSubclassOf<APickupInstance_Weapon> PickupClass;

public:
	/** Drops this weapon as a pickup from a specific location */
	void DropFrom(const FVector& StartLocation);
	
	/************************************************************************/
	/* Weapon States                                                        */
	/************************************************************************/
protected:
	UPROPERTY(Transient, BlueprintReadOnly, Category="Weapon")
	UWeaponState* CurrentState;

	UPROPERTY(Instanced, BlueprintReadOnly, Category="States")
	TObjectPtr<UWeaponState> ActiveState;
	UPROPERTY(Instanced, BlueprintReadOnly, Category="States")
	TObjectPtr<UWeaponState> InactiveState;
	
	UPROPERTY(Instanced, BlueprintReadOnly, Category="States")
	TObjectPtr<UWeaponStateEquipping> EquippingState;
	UPROPERTY(Instanced, BlueprintReadOnly, Category="States")
	TObjectPtr<UWeaponStateUnequipping> UnequippingState;

	UPROPERTY(Instanced, BlueprintReadOnly, Category="States")
	TObjectPtr<UWeaponStateReloading> ReloadingState;
	
	/**
	 * Go from the CurrentState to a NewState
	 * @param NewState the new state we want to begin
	 */
	void GotoState(UWeaponState* NewState);
	void StateChanged() {}
	
	void GoToEquippingState(float OverflowTime);

	/************************************************************************/
	/* Firing                                                               */
	/************************************************************************/
protected:
	/**
	 * firemodes with input currently being held down, either pending or actually firing.
	 * (set by equipped weapon if start fire request is accepted) */
	UPROPERTY(Transient)
	TArray<uint8> PendingFire;
	
public:
	/** is the fire mode held down? */
	bool IsPendingFire(uint8 InFireMode) const;

	/** set the fire mode's held/active status */
	void SetPendingFire(uint8 InFireMode, bool bIsActive);
	
	/** resets all pending fire flags */
	void ClearPendingFire();
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	TArray<FFireMode> FireModes;

	/** the currently active fire mode, only relevant when in firing state */ 
	UPROPERTY(BlueprintReadOnly, Category="Weapon")
	uint8 CurrentFireMode;
	
public:
	uint8 GetCurrentFireMode() const;
	void SetCurrentFireMode(uint8 InFireMode);

	void StartFire(uint8 InFireMode);
	void StopFire(uint8 InFireMode);
	
	UFUNCTION(Server, Reliable)
	void ServerStartFire(uint8 InFireMode, bool bClientFired);

	UFUNCTION(Server, Reliable)
	void ServerStopFire(uint8 InFireMode);
	
	/** sends this weapon to it's firing state, returns true if a shot is fired this frame */
	bool BeginFiringSequence(uint8 InFireMode, bool bClientFired);
	void EndFiringSequence(uint8 InFireMode);

protected:
	/** Checks to see if weapon should continue firing, or sends it back to active state */
	bool HandleContinuedFiring();
	
	UFUNCTION(BlueprintCallable, Category="Weapon")
	virtual void FireShot();

public:
	/** can this weapon fire again (after already starting firing)? */
	bool CanFireAgain() const;

	/** the weapon's next fire time based on its fire interval */
	float GetRefireTime(uint8 InFireMode) const;
	
	FVector GetFireStartLocation(uint8 FireMode) const;
	FRotator GetBaseFireRotation() const;
	
	/** plays effects emitted by weapon on fire, i.e. animations, particles and sound (not tracers or impact effects) */
	virtual void PlayFiringEffects();
	
	void SpawnTrailEffect(uint8 FireMode, const FVector& EndPoint);
	void SpawnImpactEffects(uint8 FireMode, const FVector& EndPoint);
	
	/** get the muzzle location of the weapon */
	FVector GetMuzzleLocation() const;

	/** get direction of weapon's muzzle */
	FVector GetMuzzleDirection() const;

	/** Used by both AWeapon & AWeaponAttachment to find a hit result for impact effects */
	static FHitResult GetImpactEffectHit(APawn* Shooter, const FVector& StartLoc, const FVector& TargetLoc);
	
	/************************************************************************/
	/* Ammo/Reloading                                                       */
	/************************************************************************/
protected:
	/** infinite ammo for reloads */
	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	bool bInfiniteAmmo;

	/** infinite ammo in clip, no reload required */
	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	bool bInfiniteClip;

public:
	/** check if weapon has infinite ammo */
	bool HasInfiniteAmmo() const;

	/** check if weapon has infinite clip */
	bool HasInfiniteClip() const;

protected:
	/** max ammo */
	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	int32 MaxReserveAmmo;

	/** initial clips of ammo */
	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	int32 InitialReserveAmmo;
	
	/** current total ammo (includes ammo in clip) */
	UPROPERTY(Transient, ReplicatedUsing=OnRep_Ammo)
	int32 CurrentReserveAmmo;
	
	/** clip size */
	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	int32 MaxAmmoPerClip;
	
	/** current ammo - inside clip */
	UPROPERTY(Transient, ReplicatedUsing=OnRep_Ammo)
	int32 CurrentAmmoInClip;

	UFUNCTION()
	void OnRep_Ammo();
	
public:
	/** is there enough ammo in clip to fire? */
	bool HasAmmoInClip(uint8 InFireMode) const;

	void ConsumeAmmoInClip(int32 InFireMode);
	void AddAmmoToReserve(int32 AddAmount);
	
	/** get max ammo amount (reserve) */
	int32 GetMaxReserveAmmo() const;
	
	/** get max ammo amount (clip) */
	int32 GetMaxAmmoPerClip() const;

	/** get max total ammo (reserve + clip) */
	int32 GetMaxTotalAmmo() const;
	
	/** get current ammo amount (reserve) */
	int32 GetCurrentReserveAmmo() const;
	
	/** get current ammo amount (clip) */
	int32 GetCurrentAmmoInClip() const;

	/** get current total ammo (reserve + clip) */
	int32 GetCurrentTotalAmmo() const;

protected:
	/** true when the owner has requested reload but input not consumed yet */
	UPROPERTY(Transient)
	uint8 bPendingReload:1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Weapon")
	float ReloadTime;
	
public:
	/** start/stop reload */
	void StartReload();
	void StopReload();

	UFUNCTION(Server, Reliable)
	void ServerStartReload();

	UFUNCTION(Server, Reliable)
	void ServerStopReload();
	
	void OnReloadFinished();

	/** check if weapon can be reloaded */
	bool CanReload() const;
	
	/** is the equipped weapon pending reload? */
	bool IsPendingReload() const;

	/** set the pending reload flag */
	void SetPendingReload(bool bInPendingReload);

	UFUNCTION(BlueprintPure, Category="Weapon")
	float GetReloadTime() const;
	
	/************************************************************************/
	/* Animations                                                           */
	/************************************************************************/
protected:
	UPROPERTY(EditDefaultsOnly, Category="Animation")
	UAnimMontage* BringUpAnim;
	
	UPROPERTY(EditDefaultsOnly, Category="Animation")
	UAnimMontage* PutDownAnim;

	UPROPERTY(EditDefaultsOnly, Category="Animation")
	UAnimMontage* ReloadAnim;
	
	/************************************************************************/
	/* Accessors                                                            */
	/************************************************************************/
public:
	/* -------------- Visuals -------------- */
	
	UFUNCTION(BlueprintPure, Category="Weapon")
	inline FText GetDisplayName() const;
	
	UFUNCTION(BlueprintPure, Category="Weapon")
	inline USkeletalMeshComponent* GetMesh() const;
	
	UFUNCTION(BlueprintPure, Category="Weapon")
	TSubclassOf<AWeaponAttachment> GetWeaponAttachmentType() const;
	
	UFUNCTION(BlueprintPure, Category="Weapon")
	inline EAnimPose GetWeaponAnimPose() const;

	/* -------------- User Interface -------------- */
	
	UFUNCTION(BlueprintPure, Category="Weapon")
	inline UTexture2D* GetPrimaryIcon() const;
	
	UFUNCTION(BlueprintPure, Category="Weapon")
	inline UTexture2D* GetAmmoIcon() const;

	/* -------------- Inventory -------------- */
	
	UFUNCTION(BlueprintPure, Category="Weapon")
	inline float GetBringUpTime() const;
	
	UFUNCTION(BlueprintPure, Category="Weapon")
	inline float GetPutDownTime() const;

	UFUNCTION(BlueprintPure, Category="Weapon")
	TSubclassOf<APickupInstance_Weapon> GetPickupClass() const;

	/* -------------- Pickup -------------- */

	UFUNCTION(BlueprintPure, Category="Weapon")
	inline bool CanBeThrown() const;

	/* -------------- Weapon States -------------- */

	inline bool IsEquipping() const;
	
	UFUNCTION(BlueprintPure, Category="Weapon")
	inline bool IsUnequipping() const;
	
};
