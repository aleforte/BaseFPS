// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Components/InteractableComponent.h"
#include "Inventory/Inventory.h"
#include "BaseFPSCharacter.generated.h"

class AWeaponAttachment;
class APickupInstance;
class UInventoryComponent;
class UInputComponent;
class USkeletalMeshComponent;
class USceneComponent;
class UCameraComponent;
class UAnimMontage;
class USoundBase;

class AInventory;
class AWeapon;

UENUM(BlueprintType)
enum class EAnimPose : uint8
{
	Unarmed,
	Rifle
};

USTRUCT()
struct FInteractionData
{
	GENERATED_BODY()

	/** the current interactable component in our focus */
	UPROPERTY()		class UInteractableComponent* InteractableComponentInFocus;
	
	/** the time we last checked for an interactable */
	UPROPERTY()		float LastInteractionCheckTime;
	
	/** whether the local player is holding the interact key */
	UPROPERTY()		bool bInteractHeld;
	
	FInteractionData()
	{
		InteractableComponentInFocus = nullptr;
		LastInteractionCheckTime = 0.0f;
		bInteractHeld = false;
	}
};

UENUM()
enum EInteractionEventType
{
	Begin,
	End,      // ended the interaction
	Completed // successful completed the interaction
};

/**
 * Replicated movement data for Characters (optimized over engine's default {@link FRepMovement})
 */
USTRUCT()
struct FRepCharMovement
{
	GENERATED_BODY()
	
	UPROPERTY()		FVector_NetQuantize LinearVelocity;
	UPROPERTY()		FVector_NetQuantize Location;
	UPROPERTY()		uint16 ViewYaw;
	UPROPERTY()		uint8 ViewPitch;
	 
	/* Compressed acceleration direction (lowest 2 bits are forward/back, next 2 bits are left/right (-1, 0, 1) */
	UPROPERTY()
	uint8 AccelDir;

	FRepCharMovement()
		: LinearVelocity(ForceInit)
		, Location(ForceInit)
		, ViewYaw(ForceInit)
		, ViewPitch(ForceInit)
		, AccelDir(ForceInit)
	{}

	/* Should be disabled since WithNetSerializer is commented out, meaning we're sending sending deltas instead */ 
	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		bOutSuccess = true;
		bool bOutSuccessLocal = true;

		LinearVelocity.NetSerialize(Ar, Map, bOutSuccessLocal);
		bOutSuccess &= bOutSuccessLocal;
		Location.NetSerialize(Ar, Map, bOutSuccessLocal);
		bOutSuccess &= bOutSuccessLocal;
		Ar.SerializeBits(&ViewYaw, 16);
		Ar.SerializeBits(&ViewPitch, 8);
		Ar.SerializeBits(&AccelDir, 8);

		return true;
	}

	bool operator==(const FRepCharMovement& Other) const
	{
		if (LinearVelocity == Other.LinearVelocity
			&& Location ==  Other.Location
			&& ViewPitch == Other.ViewPitch
			&& ViewYaw == Other.ViewYaw
			&& AccelDir == Other.AccelDir)
		{
			return true;
		}
		return false;
	}

	bool operator!=(const FRepCharMovement& Other) const
	{
		return !(*this == Other);
	}
};

/* Tells the engine to use the NetSerialize() method found within {@link FRepCharMovement} */
template<>
struct TStructOpsTypeTraits<FRepCharMovement> : public TStructOpsTypeTraitsBase2<FRepCharMovement>
{
	enum
	{
		// WithNetSerializer = true
	};
};

UCLASS(config=Game)
class ABaseFPSCharacter : public ACharacter
{
	GENERATED_BODY()
	
public:
	ABaseFPSCharacter(const FObjectInitializer& ObjectInitializer);

protected:
	//~Begin AActor interface
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	//~End AActor interface
public:
	virtual void Tick(float DeltaSeconds) override;

	//~Begin APawn interface
	virtual void Restart() override;
	virtual FVector GetPawnViewLocation() const override;
	virtual void Destroyed() override;
	//~End APawn interface
	
	/************************************************************************/
	/* Delegates                                                            */
	/************************************************************************/
public:
	DECLARE_MULTICAST_DELEGATE(FOnInventoryUpdatedSignature);
	FOnInventoryUpdatedSignature OnInventoryUpdated;
	
	DECLARE_MULTICAST_DELEGATE(FOnEquippedNewWeaponSignature);
	FOnEquippedNewWeaponSignature OnEquippedNewWeapon;

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnAmmoUpdatedSignature, AWeapon* /* Weapon */);
	FOnAmmoUpdatedSignature OnAmmoUpdated;

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnFocusChangedSignature, UInteractableComponent* /* NewInteractable */);
	FOnFocusChangedSignature OnFocusChanged;
	
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnInteractionEventSignature, UInteractableComponent* /* Interactable */, EInteractionEventType /* EventType */)
	FOnInteractionEventSignature OnInteractionEvent;
	
	/************************************************************************/
	/* Networking                                                           */
	/************************************************************************/
protected:
	/** Network optimized version of {@code ReplicatedMovement} **/
	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedCharMovement)
	FRepCharMovement ReplicatedCharMovement;
	
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;
	virtual void PreNetReceive() override;
	virtual void PostNetReceive() override;
	
	UFUNCTION()
	void OnRep_ReplicatedCharMovement();
	
	/************************************************************************/
	/* Movement                                                             */
	/************************************************************************/
public:
	/** Fills ReplicatedCharMovement property */
	void GatherCharMovement();
	
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	/************************************************************************/
	/* Actions                                                              */
	/************************************************************************/

	/** start/stop for primary fire */
	void StartFire();
	void StopFire();

	/** start/stop for alt fire */
	void StartAltFire();
	void StopAltFire();
	
	UFUNCTION(BlueprintCallable)
	bool IsFiringDisabled() const;

	/* -------------- Burst/Flash counters (non-local character firing effects) -------------- */
protected:
	UPROPERTY(Transient, Replicated)
	uint8 FlashFireMode;
	
	UPROPERTY(Transient, ReplicatedUsing=FiringInfoReplicated)
	uint8 FlashCounter;

	UPROPERTY(Transient, ReplicatedUsing=FiringInfoReplicated)
	FVector_NetQuantize FlashLocation;

public:
	/** used when hit location is not important, e.g. projectile fire */
	void IncrementFlashCounter(uint8 InFireMode);

	/** used when hit location is important, e.g. hitscan fire */
	void SetFlashLocation(const FVector& InFlashLoc, uint8 InFireMode);
	const FVector_NetQuantize& GetFlashLocation() const;
	
protected:
	/** [local] controls burst/flash effects for non-local characters, called on both server & clients  */
	void FiringInfoUpdated();
	
	/** stops firing effects */
	void ClearFiringInfo();
	
	UFUNCTION()
	void FiringInfoReplicated();

private:
	UPROPERTY(Transient, ReplicatedUsing=ReloadingStatusReplicated)
	uint8 bReloading:1;
	
public:
	/** start/stop for reload */
	void StartReload();
	void StopReload();

	void SetReloadStatus(bool bIsReloading);
	void ReloadingStatusUpdated(); 

	UFUNCTION()
	void ReloadingStatusReplicated();
	
	/************************************************************************/
	/* Health/Damage                                                        */
	/************************************************************************/
protected:
	UPROPERTY(Replicated, BlueprintReadWrite, Category="Pawn")
	int32 Health;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Pawn")
	int32 HealthMax;
	
public:
	/** is this character dead? */
	bool IsDead() const;

	/************************************************************************/
	/* Object Interaction                                                   */
	/************************************************************************/
protected:
	/** how often in seconds to check for an interactable object. Set this to zero if we want to check every tick. */
	UPROPERTY(EditDefaultsOnly, Category="Interaction")
	float InteractableCheckFrequency;

	UPROPERTY(EditDefaultsOnly, Category="Interaction")
	float InteractableCheckDistance;

	/** information about the current state of the player's interaction */
	UPROPERTY(Transient)
	FInteractionData InteractionData;

	/** Timer handle set when an interaction begins and cleared when an it ends/successfully completes */
	FTimerHandle InteractTimerHandle;
	
protected:
	/** checks for an interactable object in view */
	void PerformInteractableCheck();

public:
	void BeginInteract();
	void EndInteract();

protected:
	/** Called when the player presses the interact key while focusing on this interactable actor */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerBeginInteract();

	/** Called when the player releases the interact key, stops looking at actor, or gets too far away */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerEndInteract();

	/** Called when we've successfully interacted with an object */
	void Interact();
	
	/** [local] the interactable in our character's focus has changed */
	void FocusChanged(UInteractableComponent* NewInteractable);

public:
	UFUNCTION(BlueprintPure, Category="Character")
	bool IsInteracting() const;

	/** is the provided interactable currently in focus? */
	bool IsCurrentlyInFocus(UInteractableComponent* InteractableComponent) const;

	/************************************************************************/
	/* Inventory                                                            */
	/************************************************************************/

protected:
	/** The array used to store this inventory's items */
	UPROPERTY(Transient, ReplicatedUsing=OnRep_Inventory)
	TArray<AInventory*> Inventory;
	
	/** The number of items this character's inventory can hold */
	UPROPERTY(EditDefaultsOnly, Category="Inventory", meta=(ClampMin=0, ClampMax=20))
	int32 InventorySize;

protected:
	UFUNCTION()
	void OnRep_Inventory();
	
private:
	/** Flag set when this character's inventory updates this frame */
	UPROPERTY(Transient)
	uint8 bInventoryUpdatedThisFrame:1;

public:
	/** [server]
	 * Adds item to character's inventory
	 * @return true if item was added, else false */
	bool AddInventory(AInventory* Inv, bool bAutoActivate=false);

	/** [server]
	 * Replaces an item currently stored in the inventory with a new one
	 * @return true if the item was replaced, else false */
	bool ReplaceInventory(AInventory* CurrInv, AInventory* NewInv);
	
	/** [server]
	 * Removes item from character's inventory.
	 * @return true if item was removed, else false */
	bool RemoveInventory(AInventory* Inv);

	UFUNCTION(BlueprintCallable)
	void RemoveAllInventory();
	
	bool IsInventoryEmpty() const;
	bool IsInventoryFull() const;
	
	/** returns true if item is in inventory list */
	bool IsInInventory(AInventory* TestInv) const;
	AInventory* GetInventoryOfType(TSubclassOf<AInventory> InvType) const;
	
	bool CanPickUp(APickupInstance* PickupInstance) const;

protected:
	/** the character's default starting inventory */
	UPROPERTY(EditDefaultsOnly, Category="Inventory")
	TArray<TSubclassOf<AWeapon>> DefaultCharacterInventory;

public:
	/** [server] adds default inventory for character, called on Restart() by Pawn/GameMode */
	void AddDefaultInventory();

	/** [server] used to create an inventory item to give to character */
	AInventory* CreateInventory(TSubclassOf<AInventory> NewInvClass);

	/** destroys all items in character's inventory */
	void DestroyAllInventory();
		
	/************************************************************************/
	/* Weapons                                                              */
	/************************************************************************/
protected:		
	/** the currently equipped weapon */
	UPROPERTY(Transient)
	AWeapon* EquippedWeapon;

	/** the pending to-be-equipped weapon (currently switching to) */
	UPROPERTY(Transient)
	AWeapon* PendingWeapon;

public:
	/** [local] equip next weapon within inventory */
	void NextWeapon();
	
	/** [local] equip prev weapon within inventory */
	void PrevWeapon();
	
	/**
	 * Picks the next weapon to equip in the player's inventory
	 * @param bPrev find the previous weapon in sequence if true
	 * @return the next weapon to equip
	 */
	AWeapon* GetNextWeaponFromSequence(bool bPrev);
	
	/** Sets the pending weapon for this character */
	void SetPendingWeapon(AWeapon* NewWeapon);

public:	
	bool IsPendingEquip(const AInventory* CheckWeapon) const;
	bool IsEquipped(const AInventory* CheckWeapon) const;

	/**
	 * Used by weapons to let character know their ammo changed
	 */
	void NotifyAmmoUpdated(AWeapon* Weapon) const;

protected:
	/** the currently equipped weapon attachment, i.e. the character's weapon in 3rd person */
	UPROPERTY(Transient)
	AWeaponAttachment* EquippedWeaponAttachment;

	/** the currently equipped weapon's class */
	UPROPERTY(Transient, ReplicatedUsing=OnRep_EquippedWeaponClass)
	TSubclassOf<AWeapon> EquippedWeaponClass;

protected:
	UFUNCTION()
	void OnRep_EquippedWeaponClass(TSubclassOf<AWeapon> PrevWeaponClass);
	FTimerHandle UpdateWeaponAttachmentTimerHandle;
	
	void UpdateWeaponAttachment();

protected:
	/** [server] Throws the character's currently equipped weapon */
	UFUNCTION(BlueprintCallable, Category="Character")
	void ThrowWeapon();
	
	/* -------------- Weapon Switching -------------- */
public:
	/** [server + local] switches weapon */
	void SwitchWeapon(AWeapon* NewWeapon);
	
	/** [server + local] Switches to starting weapon */
	void SwitchToStartingWeapon();

private:
	/** [server + local] handles actual weapon switch logic */
	void LocalSwitchWeapon(AWeapon* NewWeapon);
	
protected:
	/** [server] request to server switch to new weapon */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSwitchWeapon(AWeapon* NewWeapon);

	/** [server] weapon check after switching to prevent mismatch between server/client */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerVerifyEquippedWeapon(AWeapon* NewWeapon);

	/** [local] request to client switch to new weapon and send RPC to server to follow their lead */
	UFUNCTION(Client, Reliable)
	void ClientSwitchWeapon(AWeapon* NewWeapon);
	
public:
	/** [server + local]
	 * called when the character's weapon changes as a result of a weapon switch
	 * @param OverflowTime - amount of time the previous weapon's Unequip state exceeded it's set PutDown() timer
	 *						 (used to keep weapon switching in sync between server/client) */
	void WeaponChanged(float OverflowTime=0.0f);
	
protected:
	/** [client] flag set when the equipped weapon is replaced but new weapon is not fully replicated */
	int32 LastEquippedInventorySlot;
public:
	/** [client] request to client to verify their current weapon with server, initiated when mismatch detected */
	UFUNCTION(Client, Reliable)
	void ClientVerifyWeapon();
	
	/** [client] notifies client that their equipped weapon or pending weapon was lost on server */
	UFUNCTION(Client, Reliable)
	void ClientWeaponLost(AInventory* LostInv);
	
	/** [client] notifies client that their equipped weapon or pending weapon was replaced on server, usually via pickup */
	UFUNCTION(Client, Reliable)
	void ClientWeaponReplaced(AInventory* ReplacedInv, AInventory* NewInv);
	
	/************************************************************************/
	/* Perspective & Visuals                                                */
	/************************************************************************/
protected:
	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category="Mesh")
	USkeletalMeshComponent* Mesh1P;
	
	/** The character's current pose/stance, for use by AnimBP **/
	UPROPERTY(Transient)
	EAnimPose CurrentAnimPose;

	/** The current view pitch, used by AnimBP */
	UPROPERTY(Transient)
	float CurrentViewPitch;
	
	/** The view target replicated from the server to simulated proxies, allowing for smooth interpolation on tick */
	UPROPERTY(Transient)
	float TargetViewPitch;
	
	/** Used to achieve smooth view pitch rotation for simulated proxies */
	UPROPERTY(EditDefaultsOnly, Category="Animation");
	float ViewPitchInterpRate;
	
public:
	UFUNCTION(BlueprintCallable, Category="Weapon")
	void SetCurrentAnimPose(EAnimPose InAnimPose);
	
	/************************************************************************/
	/* Animations                                                           */
	/************************************************************************/
public:
	/** Play Animation Montage on the character's 1st person mesh. */
	UFUNCTION(BlueprintCallable, Category="Animation")
	float PlayAnimMontage1P(UAnimMontage* AnimMontage, float InPlayRate = 1.f, FName StartSectionName = NAME_None);

	/** Stop Animation Montage (1st person mesh only). */
	UFUNCTION(BlueprintCallable, Category="Animation")
	void StopAnimMontage1P(UAnimMontage* AnimMontage);

	/************************************************************************/
	/* Character Helpers                                                    */
	/************************************************************************/
public:
	/** get camera view type */
	UFUNCTION(BlueprintCallable, Category="Mesh")
	bool IsFirstPerson() const;

	/************************************************************************/
	/* Accessors                                                            */
	/************************************************************************/
public:
	/* -------------- Character Visuals -------------- */

	UFUNCTION(BlueprintPure, Category="Character")
	UCameraComponent* GetFirstPersonCameraComponent() const;

	UFUNCTION(BlueprintPure, Category="Character")
	USkeletalMeshComponent* GetMesh1P() const;
	
	UFUNCTION(BlueprintPure, Category="Character")
	EAnimPose GetCurrentAnimPose() const;
	
	UFUNCTION(BlueprintPure, Category="Character")
	float GetCurrentViewPitch() const;

	AWeaponAttachment* GetEquippedWeaponAttachment() const;

	/* -------------- Object Interaction -------------- */

	UFUNCTION(BlueprintPure, Category="Character")
	UInteractableComponent* GetInteractableInFocus() const;

	/* -------------- Inventory -------------- */

	UFUNCTION(BlueprintPure, Category="Character")
	AWeapon* GetEquippedWeapon() const;
	
	UFUNCTION(BlueprintPure, Category="Character")
	TArray<AInventory*>& GetInventory();

	/* -------------- Weapons -------------- */

	UFUNCTION(BlueprintPure, Category="Character")
	AWeapon* GetPendingWeapon() const;
	
};

/**
 ********************************************************************************************
 * TInventoryIterator
 ********************************************************************************************
 *[Adapted from UTAlpha]
 * Iterator used to access an inventory array, useful because it protects against invalid actors (e.g. not fully initialized/replicated locally)
 * @param InvType the inventory type, e.g. weapon, armor, powerups, etc.
 */

template <typename InvType = AInventory> class BASEFPS_API TInventoryIterator
{
private:
	ABaseFPSCharacter* Owner;
	TArray<AInventory*>& Inventory;
	bool bReverse;
	int32 CurrIdx;

	inline bool IsValidForIteration(int32 Index)
	{
		return Inventory[Index] != nullptr
			&& Inventory[Index]->GetOwner() != nullptr
			&& Inventory[Index]->GetOwner() == Owner
			&& Inventory[Index]->IsA(InvType::StaticClass());
	}

public:
	TInventoryIterator(ABaseFPSCharacter* InventoryOwner, bool bRev = false)
		: Owner(InventoryOwner), Inventory(InventoryOwner->GetInventory()), bReverse(bRev)
	{
		if (InventoryOwner)
		{
			CurrIdx = bReverse ? (Inventory.Num()-1) : 0;
			if (!IsValidForIteration(CurrIdx))
			{
				Next();
			}
		}
	}

	void Next()
	{
		do
		{
			if (bReverse)
			{
				CurrIdx--;
			}
			else
			{
				CurrIdx++;
			}
		} while(Inventory.IsValidIndex(CurrIdx) && !IsValidForIteration(CurrIdx));
	}
	
	FORCEINLINE bool IsValid()
	{
		return Inventory.IsValidIndex(CurrIdx) && IsValidForIteration(CurrIdx);
	}
	
	FORCEINLINE operator bool()
	{
		return IsValid();
	}
	
	FORCEINLINE InvType* operator*()
	{
		if (!Inventory.IsValidIndex(CurrIdx) || Inventory[CurrIdx] == nullptr || !Inventory[CurrIdx]->IsA(InvType::StaticClass()))
		{
			return nullptr;
		}
		return static_cast<InvType*>(Inventory[CurrIdx]);
	}
	
	FORCEINLINE InvType* operator->()
	{
		if (!Inventory.IsValidIndex(CurrIdx) || Inventory[CurrIdx] == nullptr || !Inventory[CurrIdx]->IsA(InvType::StaticClass()))
		{
			return nullptr;
		}
		return static_cast<InvType*>(Inventory[CurrIdx]);
	}

	FORCEINLINE int32 Index() const
	{
		return CurrIdx;
	}
};

