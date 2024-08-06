// Copyright Epic Games, Inc. All Rights Reserved.

#include "BaseFPSCharacter.h"

#include "BaseFPS.h"
#include "BaseFPSCharacterMovement.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Input/Reply.h"
#include "Net/UnrealNetwork.h"
#include "Weapons/Weapon.h"
#include "Weapons/WeaponAttachment.h"

ABaseFPSCharacter::ABaseFPSCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UBaseFPSCharacterMovement>(ACharacter::CharacterMovementComponentName))
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
		
	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
 
	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetCollisionResponseToAllChannels(ECR_Ignore);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	//Mesh1P->SetRelativeRotation(FRotator(0.9f, -19.19f, 5.2f)); // FIXME (aleforte) set manually in scene editor
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

	// Health/Damage
	Health = 0;
	HealthMax = 100;

	// Actions
	FlashFireMode = 0;
	FlashCounter = 0;
	FlashLocation = FVector::ZeroVector;
	bReloading = false;
	
	// Inventory
	InventorySize = 2;
	bInventoryUpdatedThisFrame = false;

	EquippedWeapon = nullptr;
	EquippedWeaponAttachment = nullptr;
	EquippedWeaponClass = nullptr;
	LastEquippedInventorySlot = INDEX_NONE;
	
	CurrentAnimPose = EAnimPose::Unarmed;
	ViewPitchInterpRate = 14.f;


	InteractableCheckFrequency = 0.1f;
	InteractableCheckDistance = 400.0f;
}

void ABaseFPSCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
	
	// ApplyCustomPlayerKeyMappings();

	UE_LOG(LogTemp, Warning, TEXT("Testing:: BEGIN PLAY!!!"));

	if (HasAuthority())
	{
		Health = HealthMax;
	}
}

void ABaseFPSCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// initialize inventory
	Inventory.Init(nullptr, InventorySize);
}

#if WITH_EDITOR
void ABaseFPSCharacter::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (const FProperty* PropertyChanged = PropertyChangedEvent.Property)
	{
		// ensures default inventory never exceeds inventory size
		if (PropertyChanged->GetName() == FString(TEXT("DefaultCharacterInventory"))
			|| PropertyChanged->GetName() == FString(TEXT("InventorySize")))
		{
			while (DefaultCharacterInventory.Num() > InventorySize)
			{
				UE_LOG(LogTemp, Warning, TEXT("DefaultCharacterInventory size cannot exceed max inventory capacity (%d)"), InventorySize);
				DefaultCharacterInventory.Pop();
			}
		}
	}
}
#endif

void ABaseFPSCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	/* -------------- calculating current view pitch -------------- */
	if (GetNetMode() != NM_DedicatedServer)
	{
		if (GetLocalRole() != ROLE_SimulatedProxy)
		{
			CurrentViewPitch = GetControlRotation().Pitch;
		}
		else
		{
			// use interpolation for sim proxies to smooth look movement
			float ViewInterpTime = FMath::Min(1.f, ViewPitchInterpRate*DeltaSeconds);
			CurrentViewPitch = (1.f - ViewInterpTime)*CurrentViewPitch + ViewInterpTime*TargetViewPitch;
		}
		CurrentViewPitch = CurrentViewPitch > 90.f ? CurrentViewPitch - 360.f : CurrentViewPitch; // avoid wrap scenario
		CurrentViewPitch = FMath::Clamp(CurrentViewPitch, -90.f, 90.f);		
	}

	/* -------------- object interaction check -------------- */
	const bool bIsInteractingOnServer = (HasAuthority() && IsInteracting());
	if ((IsLocallyControlled() || bIsInteractingOnServer)
		&& GetWorld()->TimeSince(InteractionData.LastInteractionCheckTime) >= InteractableCheckFrequency)
	{
		PerformInteractableCheck();
	}
	
	/* -------------- inventory updates -------------- */
	if (bInventoryUpdatedThisFrame && GetLocalRole() != ROLE_SimulatedProxy)
	{
		OnInventoryUpdated.Broadcast();
		bInventoryUpdatedThisFrame = false;
	}
}

void ABaseFPSCharacter::Restart()
{
	Super::Restart();
	
	SwitchToStartingWeapon();
}

FVector ABaseFPSCharacter::GetPawnViewLocation() const
{
	return GetActorLocation() + FVector(0.f,0.f,60.f); // TODO (aleforte) Handle crouch eyeffset, using 60.f for now...
}


void ABaseFPSCharacter::Destroyed()
{
	Super::Destroyed();

	if (EquippedWeaponAttachment)
	{
		EquippedWeaponAttachment->Destroy();
		EquippedWeaponAttachment = nullptr;
	}
	DestroyAllInventory();
}

/************************************************************************/
/* Networking                                                           */
/************************************************************************/

void ABaseFPSCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	// both replicated via @code ReplicatedCharMovement
	DISABLE_REPLICATED_PROPERTY(APawn, RemoteViewPitch);
	DISABLE_REPLICATED_PRIVATE_PROPERTY(AActor, ReplicatedMovement);
	
	DOREPLIFETIME_CONDITION(ABaseFPSCharacter, ReplicatedCharMovement, COND_SimulatedOrPhysics);
	DOREPLIFETIME_CONDITION(ABaseFPSCharacter, Health, COND_None);
	DOREPLIFETIME_CONDITION(ABaseFPSCharacter, Inventory, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ABaseFPSCharacter, EquippedWeaponClass, COND_SkipOwner);

	DOREPLIFETIME_CONDITION(ABaseFPSCharacter, FlashFireMode, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ABaseFPSCharacter, FlashCounter, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ABaseFPSCharacter, FlashLocation, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ABaseFPSCharacter, bReloading, COND_SkipOwner);
}

void ABaseFPSCharacter::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	// Super::PreReplication(ChangedPropertyTracker);
	// No super() call -- move all logic that's applicable to this function. Copy/paste super() logic as needed
	
	GatherCharMovement();

	//~ ACharacter logic
	bProxyIsJumpForceApplied = (JumpForceTimeRemaining > 0.0f);
	ReplicatedMovementMode = GetCharacterMovement()->PackNetworkMovementMode();	
	ReplicatedBasedMovement = BasedMovement;

	// Optimization: only update and replicate these values if they are actually going to be used.
	if (BasedMovement.HasRelativeLocation())
	{
		// When velocity becomes zero, force replication so the position is updated to match the server (it may have moved due to simulation on the client).
		ReplicatedBasedMovement.bServerHasVelocity = !GetCharacterMovement()->Velocity.IsZero();

		// Make sure absolute rotations are updated in case rotation occurred after the base info was saved.
		if (!BasedMovement.HasRelativeRotation())
		{
			ReplicatedBasedMovement.Rotation = GetActorRotation();
		}
	}

	// Save bandwidth by not replicating this value unless it is necessary, since it changes every update.
	if ((GetCharacterMovement()->NetworkSmoothingMode == ENetworkSmoothingMode::Linear) || GetCharacterMovement()->bNetworkAlwaysReplicateTransformUpdateTimestamp)
	{
		ReplicatedServerLastTransformUpdateTimeStamp = GetCharacterMovement()->GetServerLastTransformUpdateTimeStamp();
	}
	else
	{
		ReplicatedServerLastTransformUpdateTimeStamp = 0.f;
	}
	//~ End ACharacter logic
	
}

void ABaseFPSCharacter::PreNetReceive()
{
	Super::PreNetReceive();
}

void ABaseFPSCharacter::PostNetReceive()
{
	Super::PostNetReceive();
}


void ABaseFPSCharacter::OnRep_ReplicatedCharMovement()
{
	if (GetLocalRole() == ROLE_SimulatedProxy)
	{
		FRepMovement RepMovement;
		RepMovement.bSimulatedPhysicSleep = false;
		RepMovement.bRepPhysics = false;
		RepMovement.Location = ReplicatedCharMovement.Location;
		RepMovement.LinearVelocity = ReplicatedCharMovement.LinearVelocity;
		RepMovement.AngularVelocity = FVector(0.f);
		
		const FRotator ReplicatedRotation =
			FRotator(0.f, FRotator::DecompressAxisFromShort(ReplicatedCharMovement.ViewYaw), 0.f);
		RepMovement.Rotation = ReplicatedRotation;

		TargetViewPitch = FRotator::DecompressAxisFromByte(ReplicatedCharMovement.ViewPitch);
		TargetViewPitch = TargetViewPitch > 90.f ? TargetViewPitch - 360.f : TargetViewPitch;
		TargetViewPitch = FMath::Clamp(TargetViewPitch, -90.f, 90.f);

		SetReplicatedMovement(RepMovement);
		OnRep_ReplicatedMovement(); // Need to override if we want to add any logic around ragdoll/root motion/dying
		
		if (UBaseFPSCharacterMovement* CharMovement = CastChecked<UBaseFPSCharacterMovement>(GetCharacterMovement()))
		{
			CharMovement->SetReplicatedAcceleration(ReplicatedRotation, ReplicatedCharMovement.AccelDir);
		}
	}
}

/************************************************************************/
/* Movement                                                             */
/************************************************************************/

void ABaseFPSCharacter::GatherCharMovement()
{
	ReplicatedCharMovement.Location = RootComponent->GetComponentLocation();
	ReplicatedCharMovement.ViewYaw = FRotator::CompressAxisToShort(RootComponent->GetComponentRotation().Yaw);
	
	ReplicatedCharMovement.ViewPitch = FRotator::CompressAxisToByte(GetControlRotation().Pitch);
	ReplicatedCharMovement.LinearVelocity = GetVelocity();

	FVector AccelDir = GetCharacterMovement()->GetCurrentAcceleration();
	AccelDir = AccelDir.GetSafeNormal();

	const FRotator FacingRot = FRotator(0.f, ReplicatedCharMovement.ViewYaw, 0.f);

	const FVector FacingDir = FacingRot.Vector();
	const float ForwardDot = FacingDir | AccelDir;

	ReplicatedCharMovement.AccelDir = 0;
	if (ForwardDot > 0.5f)
	{
		ReplicatedCharMovement.AccelDir |= 1;
	} else if (ForwardDot < -0.5f)
	{
		ReplicatedCharMovement.AccelDir |= 2;
	}

	const FVector SideDir = (FacingDir ^ FVector::UpVector).GetSafeNormal();
	const float SideDot = SideDir | AccelDir;
	if (SideDot > 0.5f)
	{
		ReplicatedCharMovement.AccelDir |= 4;
	}
	else if (SideDot < -0.5f)
	{
		ReplicatedCharMovement.AccelDir |= 8;
	}
}

void ABaseFPSCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void ABaseFPSCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

/************************************************************************/
/* Actions                                                              */
/************************************************************************/

void ABaseFPSCharacter::StartFire()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->StartFire(0);
	}
}

void ABaseFPSCharacter::StopFire()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->StopFire(0);
	}
}

void ABaseFPSCharacter::StartAltFire()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->StartFire(1);
	}
}

void ABaseFPSCharacter::StopAltFire()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->StopFire(1);
	}
}

bool ABaseFPSCharacter::IsFiringDisabled() const
{
	return false;
}

void ABaseFPSCharacter::FiringInfoUpdated()
{
	// TODO (aleforte) properly handle Start/Stop Firing effects, right now animations break
	// if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	// {
	// 	AnimInstance->StopAllMontages(0.2f);
	// }

	if (IsLocallyControlled() && EquippedWeapon)
	{
		const uint8 EffectFireMode = EquippedWeapon->GetCurrentFireMode();
		EquippedWeapon->SpawnTrailEffect(EffectFireMode, FlashLocation);
		EquippedWeapon->SpawnImpactEffects(EffectFireMode, FlashLocation);
	}
	else if (EquippedWeaponAttachment)
	{
		if (FlashCounter != 0 || !FlashLocation.IsZero())
		{
			EquippedWeaponAttachment->PlayFiringEffects(FlashFireMode);
			EquippedWeaponAttachment->SpawnTrailEffect(FlashFireMode, FlashLocation);
			EquippedWeaponAttachment->SpawnImpactEffects(FlashFireMode, FlashLocation);
		}
	}
}

void ABaseFPSCharacter::IncrementFlashCounter(uint8 InFireMode)
{
	FlashCounter++;
	if (FlashCounter == 0)
	{
		FlashCounter++; // in case of wrap scenario
	}
	FlashFireMode = InFireMode;

	// TODO (aleforte) pack firemode into flash counter to handle alternating prim/alt fire (see UT)
	FiringInfoUpdated();
}

void ABaseFPSCharacter::SetFlashLocation(const FVector& InFlashLoc, uint8 InFireMode)
{
	// ensure new flash loc is not the same as previous, otherwise it will not replicate to clients
	FlashLocation = ((FlashLocation - InFlashLoc).SizeSquared() > 0.5f) ?
		InFlashLoc : (InFlashLoc + FVector(0.f, 0.f, 1.0f));

	// zero vector is reserved for stop flash effects, bump value if near zero
	if (FlashLocation.IsNearlyZero(0.5f))
	{
		FlashLocation.Z += 0.6f;
	}
	FlashFireMode = InFireMode;
	FiringInfoUpdated();
}

const FVector_NetQuantize& ABaseFPSCharacter::GetFlashLocation() const
{
	return FlashLocation;
}

void ABaseFPSCharacter::ClearFiringInfo()
{
	// set flash vars to their "not firing" values
	FlashCounter = 0;
	FlashLocation = FVector::ZeroVector;
	FiringInfoUpdated();
}

void ABaseFPSCharacter::FiringInfoReplicated()
{
	if (!IsLocallyControlled())
	{
		FiringInfoUpdated();
	}
}

void ABaseFPSCharacter::StartReload()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->StartReload();
	}
}

void ABaseFPSCharacter::StopReload()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->StopReload();
	}
}

void ABaseFPSCharacter::SetReloadStatus(bool bIsReloading)
{
	bReloading = bIsReloading;
	ReloadingStatusUpdated();
}

void ABaseFPSCharacter::ReloadingStatusUpdated()
{
	if (EquippedWeaponAttachment &&EquippedWeaponClass)
	{
		UAnimMontage* ReloadAnim = EquippedWeaponAttachment->GetReloadAnim();
		if (bReloading)
		{
			const float ReloadTime = EquippedWeaponClass.GetDefaultObject()->GetReloadTime();
			PlayAnimMontage(ReloadAnim, GetScaledAnimDuration(ReloadAnim) / ReloadTime);
		}
		else
		{
			StopAnimMontage(ReloadAnim);
		}
	}
}

void ABaseFPSCharacter::ReloadingStatusReplicated()
{
	if (!IsLocallyControlled())
	{
		ReloadingStatusUpdated();
	}
}


/************************************************************************/
/* Health/Damage                                                        */
/************************************************************************/

bool ABaseFPSCharacter::IsDead() const
{
	return GetTearOff() || IsPendingKillPending();
}

/************************************************************************/
/* Object Interaction                                                   */
/************************************************************************/

void ABaseFPSCharacter::PerformInteractableCheck()
{
	if (Controller)
	{
		InteractionData.LastInteractionCheckTime = GetWorld()->GetTimeSeconds();

		FVector OutViewLocation;
		FRotator OutViewRotation;
		Controller->GetPlayerViewPoint(OutViewLocation, OutViewRotation);

		const FVector TraceStart = OutViewLocation;
		const FVector TraceDirection = OutViewRotation.Vector();
		const FVector TraceEnd = OutViewLocation + (TraceDirection * InteractableCheckDistance);
		
		FCollisionQueryParams TraceParams;
		TraceParams.AddIgnoredActor(this);

		TArray<FHitResult> OutHits;
		GetWorld()->LineTraceMultiByChannel(OutHits, TraceStart, TraceEnd, COLLISION_INTERACTABLE, TraceParams);
		if (OutHits.Num() > 0 && !OutHits[0].bBlockingHit)
		{
			UInteractableComponent* HitInFocus = nullptr;
			double DotForHitInFocus = -1.0f;
			for (FHitResult& Hit : OutHits)
			{
				if (UInteractableComponent* HitInteractable = Cast<UInteractableComponent>(Hit.GetActor()->GetComponentByClass(UInteractableComponent::StaticClass())))
				{
					const float DistanceSquared = (TraceStart - Hit.ImpactPoint).SizeSquared(); // dist^2 for optimization
					if (DistanceSquared <= HitInteractable->GetInteractableDistanceSquared())
					{
						if (IsInteracting() && IsCurrentlyInFocus(HitInteractable))
						{
							HitInFocus = HitInteractable;
							break; // note break here, current interactable takes priority if we're interacting with it
						}
						else if (HitInteractable->CanInteract(this))
						{
							// using dot product to find which interactable is closest to our viewpoint
							FVector VectorToHit = Hit.GetActor()->GetActorLocation() - TraceStart;					
							double DotForHit = FVector::DotProduct(TraceDirection, VectorToHit.GetSafeNormal());
							if (DotForHit > DotForHitInFocus)
							{
								HitInFocus = HitInteractable;
								DotForHitInFocus = DotForHit;
							}	
						}
					}					
				} else if (!Hit.bBlockingHit)
				{
					UE_LOG(LogBaseFPS, Warning, TEXT("Interactable check found actor without interactable component -- %s (class %s)"), *Hit.GetActor()->GetName(), *Hit.GetActor()->GetClass()->GetFName().ToString());
				}
			}
			
			if (!IsCurrentlyInFocus(HitInFocus))
			{
				FocusChanged(HitInFocus);
			}
			return;
		}
		// if we reach here, we've lost focus from all objects... set "in focus" to nullptr
		if (InteractionData.InteractableComponentInFocus != nullptr)
		{
			FocusChanged(nullptr);
		}
	}
}

void ABaseFPSCharacter::ServerBeginInteract_Implementation()
{
	if (HasAuthority())
	{
		BeginInteract();
	}
}

bool ABaseFPSCharacter::ServerBeginInteract_Validate()
{
	return true;
}


void ABaseFPSCharacter::BeginInteract()
{
	UE_LOG(LogTemp, Log, TEXT("BeginInteract!!"));
	if (!HasAuthority())
	{
		ServerBeginInteract();
	}

	// As an optimization, the server only checks when we begin interacting with an object (handled below). The below
	// flag is set to tell the server to begin running interaction checks (set to false on EndInteract)
	InteractionData.bInteractHeld = true;
	if (HasAuthority() && !IsLocallyControlled())
	{
		PerformInteractableCheck();
	}

	if (UInteractableComponent* Interactable = GetInteractableInFocus())
	{
		Interactable->BeginInteract(this);
		OnInteractionEvent.Broadcast(Interactable, EInteractionEventType::Begin);
		if (FMath::IsNearlyZero(Interactable->GetInteractableHoldTime()))
		{
			Interact();
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Timer SET!!"));
			GetWorldTimerManager().SetTimer(
				InteractTimerHandle,
				this,
				&ABaseFPSCharacter::Interact,
				Interactable->GetInteractableHoldTime(),
				false); // do not loop
		}
	}
}

void ABaseFPSCharacter::ServerEndInteract_Implementation()
{
	if (HasAuthority())
	{
		EndInteract();
	}
}

bool ABaseFPSCharacter::ServerEndInteract_Validate()
{
	return true;
}

void ABaseFPSCharacter::EndInteract()
{
	UE_LOG(LogTemp, Log, TEXT("End Interact..."));
	if (!HasAuthority())
	{
		ServerEndInteract();
	}

	InteractionData.bInteractHeld = false;
	GetWorldTimerManager().ClearTimer(InteractTimerHandle);
	if (UInteractableComponent* Interactable = GetInteractableInFocus())
	{
		Interactable->EndInteract(this);
		OnInteractionEvent.Broadcast(Interactable, EInteractionEventType::End);	
	}
}

void ABaseFPSCharacter::Interact()
{
	UE_LOG(LogTemp, Warning, TEXT("Interact Called!!!!"));

	InteractionData.bInteractHeld = false;
	GetWorldTimerManager().ClearTimer(InteractTimerHandle);
	if (UInteractableComponent* Interactable = GetInteractableInFocus())
	{
		Interactable->Interact(this);
		OnInteractionEvent.Broadcast(Interactable, EInteractionEventType::Completed);
	}
}

void ABaseFPSCharacter::FocusChanged(UInteractableComponent* NewInteractable)
{
	if (IsInteracting())
	{
		EndInteract(); // cancel out any previous interaction since we're now looking at a new object
	}
	
	InteractionData.InteractableComponentInFocus = NewInteractable;
	OnFocusChanged.Broadcast(NewInteractable);
}


bool ABaseFPSCharacter::IsInteracting() const
{
	return GetWorldTimerManager().IsTimerActive(InteractTimerHandle);
}

bool ABaseFPSCharacter::IsCurrentlyInFocus(UInteractableComponent* InteractableComponent) const
{
	return InteractableComponent == InteractionData.InteractableComponentInFocus;
}

/************************************************************************/
/* Inventory                                                            */
/************************************************************************/

void ABaseFPSCharacter::OnRep_Inventory()
{
	bInventoryUpdatedThisFrame = true;
	if (LastEquippedInventorySlot > INDEX_NONE)
	{
		if (!PendingWeapon)
		{
			SetPendingWeapon(Cast<AWeapon>(Inventory[LastEquippedInventorySlot]));
			WeaponChanged();
		}
		LastEquippedInventorySlot = INDEX_NONE;
	}
}

bool ABaseFPSCharacter::AddInventory(AInventory* Inv, bool bAutoActivate)
{
	if (HasAuthority() && Inv)
	{
		const int32 Slot = Inventory.IndexOfByPredicate([](const AInventory* Inv){ return Inv == nullptr; });
		if (Slot > INDEX_NONE)
		{
			Inventory[Slot] = Inv;
			Inv->OnAddedToInventory(this);
			bInventoryUpdatedThisFrame = true;
		}
		return true;
	}
	return false;
}

bool ABaseFPSCharacter::ReplaceInventory(AInventory* CurrInv, AInventory* NewInv)
{
	if (HasAuthority() && CurrInv && NewInv)
	{
		const int32 Slot = Inventory.Find(CurrInv);
		if (Slot > INDEX_NONE)
		{
			Inventory[Slot]->OnRemovedFromInventory();
			Inventory[Slot] = NewInv;
			Inventory[Slot]->OnAddedToInventory(this);

			if (!IsLocallyControlled() && (IsPendingEquip(CurrInv) || IsEquipped(CurrInv)))
			{
				ClientWeaponReplaced(CurrInv, NewInv);
			}
			
			if (IsPendingEquip(CurrInv))
			{
				SetPendingWeapon(nullptr);
				WeaponChanged();
			}
			else if(IsEquipped(CurrInv))
			{
				EquippedWeapon = nullptr;
				if (!PendingWeapon)
				{
					SetPendingWeapon(Cast<AWeapon>(NewInv));	
				}
				WeaponChanged();
			}
			
			bInventoryUpdatedThisFrame = true;
		}
		return true;
	}
	return false;
}

bool ABaseFPSCharacter::RemoveInventory(AInventory* Inv)
{
	if (HasAuthority() && Inv)
	{
		const int32 Slot = Inventory.Find(Inv);
		if (Slot > INDEX_NONE)
		{	
			Inventory[Slot]->OnRemovedFromInventory();
			Inventory[Slot] = nullptr;

			if (!IsLocallyControlled() && (IsPendingEquip(Inv) || IsEquipped(Inv)))
			{
				ClientWeaponLost(Inv);
			}
			
			if (IsPendingEquip(Inv))
			{
				SetPendingWeapon(nullptr);
				WeaponChanged();
			}
			else if (IsEquipped(Inv))
			{
				EquippedWeapon = nullptr;
				if (!PendingWeapon)
				{
					TInventoryIterator<AWeapon> It(this);
					SetPendingWeapon(*It);
				}
				WeaponChanged();
			}
			
			bInventoryUpdatedThisFrame = true;
			return true;
		}
	}
	return false;
}

void ABaseFPSCharacter::RemoveAllInventory()
{
	if (HasAuthority())
	{
		for (int32 i = 0; i < InventorySize; i++)
		{
			if (Inventory[i] != nullptr)
			{
				Inventory[i]->OnRemovedFromInventory();
				Inventory[i] = nullptr;
			}
		}
	}
	bInventoryUpdatedThisFrame = true;
}

bool ABaseFPSCharacter::IsInventoryEmpty() const
{
	for (int32 i = 0; i < Inventory.Num(); i++)
	{
		if (Inventory[i] != nullptr && Inventory[i]->IsOwnedBy(this))
		{
			return false;
		}
	}
	return true;
}

bool ABaseFPSCharacter::IsInventoryFull() const
{
	for (int32 i = 0; i < Inventory.Num(); i++)
	{
		if (Inventory[i] == nullptr)
		{
			return false;
		}
	}
	return true;
}

bool ABaseFPSCharacter::IsInInventory(AInventory* TestInv) const
{
	return IsValid(TestInv) && Inventory.Contains(TestInv);
}

AInventory* ABaseFPSCharacter::GetInventoryOfType(TSubclassOf<AInventory> InvType) const
{
	for (int32 i = 0; i < Inventory.Num(); i++)
	{
		if (Inventory[i] && Inventory[i]->IsA(InvType))
		{
			return Inventory[i];
		}
	}
	return nullptr;
}

bool ABaseFPSCharacter::CanPickUp(APickupInstance* PickupInstance) const
{
	return !IsDead();
}

void ABaseFPSCharacter::AddDefaultInventory()
{
	// TODO (aleforte) add logic to GameMode can pass its own array of weapons
	for (int32 i = 0; i < DefaultCharacterInventory.Num(); i++)
	{
		UE_LOG(LogTemp, Error, TEXT("Adding New Inventory (Inv=%s, Owner=%s)"), *DefaultCharacterInventory[i]->GetName(), *GetName());
		CreateInventory(DefaultCharacterInventory[i]);
	}
}

AInventory* ABaseFPSCharacter::CreateInventory(TSubclassOf<AInventory> NewInvClass)
{
	if (NewInvClass && HasAuthority())
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AWeapon* NewWeapon = GetWorld()->SpawnActor<AWeapon>(NewInvClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		const int32 Slot = AddInventory(NewWeapon, false);
		if (Slot == INDEX_NONE)
		{
			NewWeapon->Destroy();
			return nullptr;
		}
		return NewWeapon;
	}
	return nullptr;
}

void ABaseFPSCharacter::DestroyAllInventory()
{
	for (int32 i = 0; i < InventorySize; i++)
	{
		if (Inventory[i])
		{
			Inventory[i]->Destroy();
			Inventory[i] = nullptr;
		}
	}
	EquippedWeapon = nullptr;
	PendingWeapon = nullptr;
}

/************************************************************************/
/* Weapons                                                              */
/************************************************************************/

void ABaseFPSCharacter::NextWeapon()
{
	UE_LOG(LogTemp, Warning, TEXT("Next Weapon called!"));
	SwitchWeapon(GetNextWeaponFromSequence(false));
}

void ABaseFPSCharacter::PrevWeapon()
{
	UE_LOG(LogTemp, Warning, TEXT("Prev Weapon called!"));
	SwitchWeapon(GetNextWeaponFromSequence(true));
}

AWeapon* ABaseFPSCharacter::GetNextWeaponFromSequence(bool bPrev)
{
	const AWeapon* CurrWeapon = PendingWeapon ? PendingWeapon : EquippedWeapon;
	
	bool bFoundCurrWeapon = false;
	AWeapon* WrapChoice = nullptr; // wrap around scenario
	AWeapon* BestChoice = nullptr;

	for (TInventoryIterator<AWeapon> It(this, !bPrev); It; It.Next()) // note, we're iterating in inverse direction
	{
		if (CurrWeapon == *It)
		{
			bFoundCurrWeapon = true;
		}
		if (!bFoundCurrWeapon)
		{
			BestChoice = *It;
		}
		WrapChoice = *It;
	}
	return BestChoice ? BestChoice : WrapChoice;
}

void ABaseFPSCharacter::SetPendingWeapon(AWeapon* NewWeapon)
{
	PendingWeapon = NewWeapon;
}

bool ABaseFPSCharacter::IsPendingEquip(const AInventory* CheckWeapon) const
{
	return PendingWeapon == CheckWeapon;
}

bool ABaseFPSCharacter::IsEquipped(const AInventory* CheckWeapon) const
{
	return EquippedWeapon == CheckWeapon;
}

void ABaseFPSCharacter::NotifyAmmoUpdated(AWeapon* Weapon) const
{
	OnAmmoUpdated.Broadcast(Weapon);
}

void ABaseFPSCharacter::OnRep_EquippedWeaponClass(TSubclassOf<AWeapon> PrevWeaponClass)
{
	if (IsLocallyControlled())
	{
		return; // not needed for our local character
	}

	if (PrevWeaponClass && EquippedWeaponAttachment && EquippedWeaponAttachment->GetPutDownAnim())
	{
		const float PutDownTime = PrevWeaponClass.GetDefaultObject()->GetPutDownTime();
		GetWorldTimerManager().SetTimer(UpdateWeaponAttachmentTimerHandle, this, &ThisClass::UpdateWeaponAttachment, PutDownTime);

		UAnimMontage* PutDownAnim = EquippedWeaponAttachment->GetPutDownAnim();
		PlayAnimMontage(PutDownAnim, GetScaledAnimDuration(PutDownAnim) / PutDownTime);
	}
	else
	{
		UpdateWeaponAttachment();
	}
}

void ABaseFPSCharacter::UpdateWeaponAttachment()
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return; // strictly cosmetic, so no action needed on dedicated
	}

	if (EquippedWeaponAttachment)
	{
		// always call stop anim, just in case we're not bringing up a new weapon
		// otherwise, character could get stuck in finish put down stance
		StopAnimMontage(EquippedWeaponAttachment->GetPutDownAnim());
	}
	
	const TSubclassOf<AWeaponAttachment> AttachmentClass = EquippedWeaponClass ? EquippedWeaponClass.GetDefaultObject()->GetWeaponAttachmentType() : nullptr;
	if (EquippedWeaponAttachment && (AttachmentClass == nullptr || !EquippedWeaponAttachment->IsA(AttachmentClass)))
	{
		EquippedWeaponAttachment->Destroy();
		EquippedWeaponAttachment = nullptr;
		if (GetLocalRole() == ROLE_SimulatedProxy)
		{
			SetCurrentAnimPose(EAnimPose::Unarmed);
		}
	}
	if (EquippedWeaponAttachment == nullptr && AttachmentClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Instigator = this;
		SpawnParams.Owner = this;
		EquippedWeaponAttachment = GetWorld()->SpawnActor<AWeaponAttachment>(AttachmentClass, SpawnParams);
		EquippedWeaponAttachment->AttachToOwnerEquipped();
		
		if (GetLocalRole() == ROLE_SimulatedProxy)
		{
			// anim and pose handled for by weapon state for local characters
			if (EquippedWeaponAttachment->GetBringUpAnim())
			{
				const float BringUpTime = EquippedWeaponClass.GetDefaultObject()->GetBringUpTime();
				UAnimMontage* BringUpAnim = EquippedWeaponAttachment->GetBringUpAnim();
				PlayAnimMontage(BringUpAnim, GetScaledAnimDuration(BringUpAnim) / BringUpTime);
			}
			SetCurrentAnimPose(EquippedWeaponClass ? EquippedWeaponClass.GetDefaultObject()->GetWeaponAnimPose() : EAnimPose::Unarmed);
		}
	}
}

void ABaseFPSCharacter::ThrowWeapon()
{
	if (HasAuthority() && EquippedWeapon && EquippedWeapon->CanBeThrown())
	{
		AWeapon* ThrownWeapon = EquippedWeapon;
		RemoveInventory(ThrownWeapon);
		ThrownWeapon->DropFrom(GetActorLocation());
	}
}

void ABaseFPSCharacter::SwitchWeapon(AWeapon* NewWeapon)
{
	if (NewWeapon == nullptr || IsDead())
	{
		return;
	}

	if (HasAuthority())
	{
		ClientSwitchWeapon(NewWeapon);
	}
	else if (IsLocallyControlled())
	{
		ServerSwitchWeapon(NewWeapon);
		LocalSwitchWeapon(NewWeapon);
	}
}


void ABaseFPSCharacter::SwitchToStartingWeapon()
{
	if (IsInventoryEmpty())
	{
		return;
	}

	if (IsLocallyControlled())
	{
		// for now, just switch to first weapon found...
		TInventoryIterator<AWeapon> It(this);
		SwitchWeapon(*It);	
	}
}

void ABaseFPSCharacter::LocalSwitchWeapon(AWeapon* NewWeapon)
{
	if (IsDead() || (NewWeapon && !Inventory.Contains(NewWeapon)))
	{
		return;
	}

	// [borrowed from UT] ensures clients don't try to switch to non-fully replicated weapons or
	// weapons that have been removed (e.g. sent by client before they received the inventory update)
	if (NewWeapon && (NewWeapon->GetCharacterOwner() != this || (HasAuthority() && !Inventory.Contains(NewWeapon))))
	{
		UE_LOG(LogTemp, Error, TEXT("MISSING OWNER!!! (bServer=%d)"), HasAuthority());
		ClientSwitchWeapon(EquippedWeapon);
		return;
	}
	
	if (!EquippedWeapon)
	{
		// initial equip scenario, just bring up new weapon if present
		if (NewWeapon)
		{
			SetPendingWeapon(NewWeapon);
			WeaponChanged();
		}
	}
	else if (NewWeapon)
	{
		if (EquippedWeapon != NewWeapon)
		{
			// switching to new weapon
			if (EquippedWeapon->PutDown())
			{
				SetPendingWeapon(NewWeapon);
			}
		}
		else if (PendingWeapon)
		{
			// switching back to current weapon
			SetPendingWeapon(nullptr);
			EquippedWeapon->BringUp();
		}
	}
	else if (PendingWeapon && EquippedWeapon->IsUnequipping())
	{
		// stop switch in progress
		SetPendingWeapon(nullptr);
		EquippedWeapon->BringUp();
	}
}

void ABaseFPSCharacter::ServerSwitchWeapon_Implementation(AWeapon* NewWeapon)
{
	if (NewWeapon)
	{
		LocalSwitchWeapon(NewWeapon);	
	}
}

bool ABaseFPSCharacter::ServerSwitchWeapon_Validate(AWeapon* NewWeapon)
{
	return true;
}

void ABaseFPSCharacter::ServerVerifyEquippedWeapon_Implementation(AWeapon* NewWeapon)
{
	// only check for non-local players
	if (HasAuthority() && !IsLocallyControlled() && NewWeapon && ! IsEquipped(NewWeapon) && !IsPendingEquip(NewWeapon))
	{
		UE_LOG(LogBaseFPS, Warning, TEXT("%s (%s) weapon mismatch: server %s, client %s"), *GetName(), GetPlayerState() ? *GetPlayerState()->GetPlayerName() : TEXT("None"), *GetNameSafe(PendingWeapon ? PendingWeapon : EquippedWeapon), *GetNameSafe(NewWeapon));
		LocalSwitchWeapon(NewWeapon);
		if (EquippedWeapon != NewWeapon && PendingWeapon != NewWeapon)
		{
			UE_LOG(LogBaseFPS, Warning, TEXT("%s -- requested weapon was invalid"), *GetName());
			ClientSwitchWeapon(PendingWeapon ? PendingWeapon : EquippedWeapon);
		}
	}
}

bool ABaseFPSCharacter::ServerVerifyEquippedWeapon_Validate(AWeapon* NewWeapon)
{
	return true;
}

void ABaseFPSCharacter::ClientSwitchWeapon_Implementation(AWeapon* NewWeapon)
{
	UE_LOG(LogTemp, Warning, TEXT("ABaseFPSCharacter::ClientSwitchWeapon, New Weapon: %s (bServer=%d)"), *GetNameSafe(NewWeapon), HasAuthority());
	if (!HasAuthority() && IsLocallyControlled())
	{
		ServerSwitchWeapon(NewWeapon);
	}
	LocalSwitchWeapon(NewWeapon);
}

void ABaseFPSCharacter::WeaponChanged(float OverflowTime)
{
	if (PendingWeapon && PendingWeapon->IsOwnedBy(this))
	{
		if (EquippedWeapon)
		{
			EquippedWeapon->DetachMeshFromPawn();
		}
		EquippedWeapon = PendingWeapon;
		SetPendingWeapon(nullptr);
		EquippedWeaponClass = EquippedWeapon->GetClass();
		UpdateWeaponAttachment();
		EquippedWeapon->BringUp(OverflowTime);
		OnEquippedNewWeapon.Broadcast();
	}
	else if (EquippedWeapon) // bring back up current weapon
	{
		EquippedWeapon->BringUp();
	}
	else
	{
		EquippedWeapon = nullptr;
		SetPendingWeapon(nullptr);
		EquippedWeaponClass = nullptr;
		SetCurrentAnimPose(EAnimPose::Unarmed);
		UpdateWeaponAttachment();
		OnEquippedNewWeapon.Broadcast();
	}
	
	// verify our weapon on server if we're a client
	if (!HasAuthority() && IsLocallyControlled())
	{
		ServerVerifyEquippedWeapon(EquippedWeapon);
	}
}

void ABaseFPSCharacter::ClientVerifyWeapon_Implementation()
{
	// if PendingWeapon set, then we are currently switching weapons
	// ServerVerifyWeapon will already happen when switch is finished
	if (!PendingWeapon)
	{
		ServerVerifyEquippedWeapon(EquippedWeapon);
	}
}

void ABaseFPSCharacter::ClientWeaponLost_Implementation(AInventory* LostInv)
{
	if (!HasAuthority() && IsLocallyControlled())
	{

		if (IsPendingEquip(LostInv))
		{
			SetPendingWeapon(nullptr);
			WeaponChanged();
		}
		else if (IsEquipped(LostInv))
		{
			EquippedWeapon = nullptr;

			if (!PendingWeapon)
			{
				TInventoryIterator<AWeapon> It(this);
				SetPendingWeapon(*It);	
			}
			WeaponChanged();
		}
	}
}

void ABaseFPSCharacter::ClientWeaponReplaced_Implementation(AInventory* ReplacedInv, AInventory* NewInv)
{
	if (!HasAuthority() && IsLocallyControlled())
	{
		if (!ReplacedInv)
		{
			// trying to infer replaced inv if it's nullptr on client
			if (PendingWeapon && !PendingWeapon->GetCharacterOwner())
			{
				ReplacedInv = PendingWeapon;
			}
			else if (EquippedWeapon && !EquippedWeapon->GetCharacterOwner())
			{
				ReplacedInv = EquippedWeapon;
			}
		}
		
		if (IsPendingEquip(ReplacedInv))
		{
			SetPendingWeapon(NewInv ? Cast<AWeapon>(NewInv) : nullptr);
		}
		else if (IsEquipped(ReplacedInv))
		{
			if (NewInv)
			{
				EquippedWeapon = nullptr;
				if (!PendingWeapon)
				{
					SetPendingWeapon(Cast<AWeapon>(NewInv));
				}
				WeaponChanged();
			}
			else
			{
				// if we're here, then new inv was not present on client yet, setting flag to switch to OnRep/RepNotify.
				// (not sure if/when this could happen, but keeping just in case...)
				LastEquippedInventorySlot = Inventory.Find(EquippedWeapon);
			}
		}
	}
}

/************************************************************************/
/* Perspective & Visuals                                                */
/************************************************************************/

void ABaseFPSCharacter::SetCurrentAnimPose(EAnimPose InAnimPose)
{
	CurrentAnimPose = InAnimPose;
}

/************************************************************************/
/* Animations                                                           */
/************************************************************************/

float ABaseFPSCharacter::PlayAnimMontage1P(UAnimMontage* AnimMontage, float InPlayRate, FName StartSectionName)
{
	UAnimInstance * AnimInstance = (Mesh1P)? Mesh1P->GetAnimInstance() : nullptr; 
	if ( AnimMontage && AnimInstance )
	{
		float const Duration = AnimInstance->Montage_Play(AnimMontage, InPlayRate);
		if (Duration > 0.f)
		{
			// Start at a given Section.
			if( StartSectionName != NAME_None )
			{
				AnimInstance->Montage_JumpToSection(StartSectionName, AnimMontage);
			}

			return Duration;
		}
	}	
	return 0.f;
}

void ABaseFPSCharacter::StopAnimMontage1P(UAnimMontage* AnimMontage)
{
	UAnimInstance * AnimInstance = (Mesh1P)? Mesh1P->GetAnimInstance() : nullptr; 
	UAnimMontage * MontageToStop = (AnimMontage) ? AnimMontage : GetCurrentMontage();
	bool bShouldStopMontage =  AnimInstance && MontageToStop && !AnimInstance->Montage_GetIsStopped(MontageToStop);
	if ( bShouldStopMontage )
	{
		AnimInstance->Montage_Stop(MontageToStop->BlendOut.GetBlendTime(), MontageToStop);
	}
}

/************************************************************************/
/* Character Helpers                                                    */
/************************************************************************/

bool ABaseFPSCharacter::IsFirstPerson() const
{
	return !IsDead() && Controller && Controller->IsLocalPlayerController();
}

/************************************************************************/
/* Accessors                                                            */
/************************************************************************/

/* -------------- Character Visuals -------------- */

UCameraComponent* ABaseFPSCharacter::GetFirstPersonCameraComponent() const
{
	return FirstPersonCameraComponent;
}

USkeletalMeshComponent* ABaseFPSCharacter::GetMesh1P() const
{
	return Mesh1P;
}

EAnimPose ABaseFPSCharacter::GetCurrentAnimPose() const
{
	return CurrentAnimPose;
}

float ABaseFPSCharacter::GetCurrentViewPitch() const
{
	return CurrentViewPitch;
}

AWeaponAttachment* ABaseFPSCharacter::GetEquippedWeaponAttachment() const
{
	return EquippedWeaponAttachment;
}

/* -------------- Object Interaction -------------- */

UInteractableComponent* ABaseFPSCharacter::GetInteractableInFocus() const
{
	return InteractionData.InteractableComponentInFocus;
}

/* -------------- Inventory -------------- */

AWeapon* ABaseFPSCharacter::GetEquippedWeapon() const
{
	return EquippedWeapon;
}

TArray<AInventory*>& ABaseFPSCharacter::GetInventory()
{
	return Inventory;
}

AWeapon* ABaseFPSCharacter::GetPendingWeapon() const
{
	return PendingWeapon;
}




