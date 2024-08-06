// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Weapon.h"

#include "BaseFPS.h"
#include "WeaponAttachment.h"
#include "Net/UnrealNetwork.h"
#include "Pickups/PickupInstance_Weapon.h"

#include "States/WeaponState.h"
#include "States/WeaponStateActive.h"
#include "States/WeaponStateInactive.h"
#include "States/WeaponStateEquipping.h"
#include "States/WeaponStateUnequipping.h"
#include "States/WeaponStateFiring.h"
#include "States/WeaponStateReloading.h"

// Weapon attach points for {@code ABaseFPSCharacter}
static FName Mesh1PGripPoint = "GripPoint_1P";

// Weapon attach point for muzzle/particle effects
static FName MuzzleAttachPoint = "MuzzleAttachPoint";

// Sets default values
AWeapon::AWeapon(const FObjectInitializer& ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	PrimaryActorTick.bAllowTickOnDedicatedServer = true;
		
	// Components
	Mesh = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("Mesh1P"));
	Mesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	Mesh->bReceivesDecals = false;
	Mesh->CastShadow = false;
	Mesh->bSelfShadowOnly = true;
	Mesh->SetCollisionObjectType(ECC_WorldDynamic);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	Mesh->SetOnlyOwnerSee(true);
	Mesh->SetHiddenInGame(true);
	RootComponent = Mesh;

	// Properties
	DisplayName = NSLOCTEXT("BaseFPS", "WeaponName", "Weapon");
	BringUpTime = 0.4f;
	PutDownTime = 0.4f;
	HandsAttachSocket = Mesh1PGripPoint;
	WeaponAnimPose = EAnimPose::Unarmed;

	
	bCanBeThrown = true;
	
	// Ammo/Reloading
	bInfiniteAmmo = false;
	bInfiniteClip = false;
	MaxReserveAmmo = 120;
	InitialReserveAmmo = 90;
	MaxAmmoPerClip = 30;

	bPendingReload = false;
	ReloadTime = 2.2f;
	
	// States
	InactiveState = ObjectInitializer.CreateDefaultSubobject<UWeaponStateInactive>(this, TEXT("StateInactive"));
	ActiveState = ObjectInitializer.CreateDefaultSubobject<UWeaponStateActive>(this, TEXT("StateActive"));
	EquippingState = ObjectInitializer.CreateDefaultSubobject<UWeaponStateEquipping>(this, TEXT("StateEquipping"));
	UnequippingState = ObjectInitializer.CreateDefaultSubobject<UWeaponStateUnequipping>(this, TEXT("StateUnequipping"));
	ReloadingState = ObjectInitializer.CreateDefaultSubobject<UWeaponStateReloading>(this, TEXT("StateReloading"));
	for (int32 i = 0; i < 2; ++i)
	{
		UWeaponStateFiring* NewState = ObjectInitializer.CreateDefaultSubobject<UWeaponStateFiring, UWeaponStateFiring>(this, FName(*FString::Printf(TEXT("FiringState%i"), i)));
		if (NewState)
		{
			FireModes.Add(FFireMode());
			FireModes[i].FiringState = NewState;
			FireModes[i].FiringInterval = 1.0f;
			FireModes[i].AmmoCost = 1;
		}
	}
	
}


void AWeapon::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	InactiveState->SetOuterWeapon(this);
	ActiveState->SetOuterWeapon(this);
	EquippingState->SetOuterWeapon(this);
	UnequippingState->SetOuterWeapon(this);
	for (int i = 0; i < FireModes.Num(); ++i)
	{
		FireModes[i].FiringState->SetOuterWeapon(this);
	}
	ReloadingState->SetOuterWeapon(this);
	GotoState(InactiveState);

	CurrentAmmoInClip = MaxAmmoPerClip;
	if (InitialReserveAmmo > 0)
	{
		AddAmmoToReserve(InitialReserveAmmo);
	}
}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();
}

void AWeapon::Destroyed()
{
	Super::Destroyed();
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AWeapon, CurrentAmmoInClip, COND_None);
	DOREPLIFETIME_CONDITION(AWeapon, CurrentReserveAmmo, COND_None);
}

// Called every frame
void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (CurrentState != InactiveState)
	{
		CurrentState->Tick(DeltaTime);
	}
}

/************************************************************************/
/* Inventory                                                            */
/************************************************************************/

void AWeapon::OnAddedToInventory(ABaseFPSCharacter* NewOwner)
{
	Super::OnAddedToInventory(NewOwner);
	if (CurrentState != InactiveState)
	{
		UE_LOG(LogTemp, Warning, TEXT("Uh Oh! Non-Inactive Weapon added to inventory (Weapon=%s, Owner=%s"), *GetName(), *NewOwner->GetName());
		GotoState(InactiveState);
	}
}

void AWeapon::OnRemovedFromInventory()
{
	// ensure we got to inactive state
	if (CurrentState != InactiveState)
	{
		GotoState(InactiveState);
	}
	DetachMeshFromPawn();
	
	Super::OnRemovedFromInventory();
}

void AWeapon::BringUp(float OverflowTime /*=0.0f*/)
{
	UE_LOG(LogTemp, Log, TEXT("BringUp (Weapon=%s, bServer=%d)!!!"), *GetName(), HasAuthority());
	CurrentState->BringUp(OverflowTime);
}

bool AWeapon::PutDown() const
{
	UE_LOG(LogTemp, Log, TEXT("PutDown (Weapon=%s, bServer=%d)!!!"), *GetName(), HasAuthority());
	return CurrentState->PutDown();
}

void AWeapon::AttachMeshToPawn()
{
	if (CharacterOwner)
	{
		DetachMeshFromPawn(); // quick clean up, just in case
		if (!Mesh->IsAttachedTo(CharacterOwner->GetMesh1P()))
		{
			Mesh->AttachToComponent(CharacterOwner->GetMesh1P(), FAttachmentTransformRules::KeepRelativeTransform, Mesh1PGripPoint);
			SetActorHiddenInGame(false);
		}
	}
}

void AWeapon::DetachMeshFromPawn()
{
	if (CharacterOwner)
	{
		CharacterOwner->StopAnimMontage1P(nullptr); // clears any weapon-related anim that might be playing	
	}
	Mesh->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	SetActorHiddenInGame(true);
}

/************************************************************************/
/* Pickup                                                               */
/************************************************************************/

void AWeapon::DropFrom(const FVector& StartLocation)
{
	UE_LOG(LogTemp, Warning, TEXT("Attemping throw wea pon!"));
	if (PickupClass)
	{
		// using line trace to detect ground normal, then using normal
		// to set the rotation of pickup on the ground.
		FHitResult Hit;
		FCollisionQueryParams TraceParams;
		TraceParams.AddIgnoredActor(this);
		if (GetWorld()->LineTraceSingleByChannel(
			Hit,
			StartLocation,
			StartLocation - FVector(0.f, 0.f, 300.f),
			ECC_WorldDynamic,
			TraceParams,
			WorldResponseParams)) 
		{
			UE_LOG(LogTemp, Warning, TEXT("Found Ground!"));
			// quaternion required to combine rotations, e.g. on slopes
			FVector NormalVector = Hit.ImpactNormal;
			FVector RotationAxis = FVector::CrossProduct(GetRootComponent()->GetUpVector(), NormalVector);
			RotationAxis.Normalize();

			float DotProduct = FVector::DotProduct(GetRootComponent()->GetUpVector(), NormalVector);
			float RotationAngle = acosf(DotProduct);

			FQuat Quat = FQuat(RotationAxis, RotationAngle);
			const FRotator PickupRotation = FRotator(Quat * FRotator(0.f, FMath::RandRange(0.f, 360.f), 0.f).Quaternion());

			const FVector PickupLocation = Hit.ImpactPoint + FVector(0.f, 0.f, 2.f); // need to raise off the ground a little to avoid clipping
			const FTransform Transform = FTransform(PickupRotation, PickupLocation);
			if (APickupInstance_Weapon* Pickup = GetWorld()->SpawnActorDeferred<APickupInstance_Weapon>(PickupClass,
				Transform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn))
			{
				UE_LOG(LogTemp, Warning, TEXT("Spawned Pickup!"));
				Pickup->SetDroppedWeapon(this);
				Pickup->FinishSpawning(Transform);
			}
			return;
		}
	}
	// destroy is checks fail...
	Destroy();
}

/************************************************************************/
/* Weapon States                                                        */
/************************************************************************/

void AWeapon::GotoState(UWeaponState* NewState)
{
	if (!NewState || !NewState->IsIn(this))
	{
		UE_LOG(LogTemp, Error, TEXT("Attempted to send weapon to invalid state (Weapon=%s, State=%s)"), *GetName(), *GetNameSafe(NewState));
		return;
	}
	if (ensureMsgf(CharacterOwner != nullptr || NewState == InactiveState,
		TEXT("Attempted to send weapon [%s] to non-inactive state [%s] while not owned"), *GetName(), *GetNameSafe(NewState)))
	{
		if (CurrentState != NewState)
		{
			UWeaponState* PrevState = CurrentState;
			if (CurrentState)
			{
				CurrentState->EndState();
			}
			CurrentState = NewState;
			CurrentState->BeginState(PrevState);
			StateChanged(); // allows for weapon-specific state change logic
		}
	}
}

void AWeapon::GoToEquippingState(float OverflowTime)
{
	GotoState(EquippingState);
	if (CurrentState == EquippingState)
	{
		EquippingState->StartEquip(OverflowTime);
	}
}

/************************************************************************/
/* Firing                                                               */
/************************************************************************/

bool AWeapon::IsPendingFire(uint8 InFireMode) const
{
	return !CharacterOwner->IsFiringDisabled() && (InFireMode < PendingFire.Num() && PendingFire[InFireMode] != 0);
}

void AWeapon::SetPendingFire(uint8 InFireMode, bool bIsActive)
{
	if (PendingFire.Num() < InFireMode + 1)
	{
		PendingFire.SetNumZeroed(InFireMode + 1);
	}
	PendingFire[InFireMode] = bIsActive;
}

void AWeapon::ClearPendingFire()
{
	for (int i = 0; i < PendingFire.Num(); i++)
	{
		PendingFire[i] = 0;
	}
}

uint8 AWeapon::GetCurrentFireMode() const
{
	return CurrentFireMode;
}

void AWeapon::SetCurrentFireMode(uint8 InFireMode)
{
	CurrentFireMode = InFireMode;
}

void AWeapon::StartFire(uint8 InFireMode)
{
	UE_LOG(LogTemp, Log, TEXT("Start Firing Mode (%d)!!!"), InFireMode);
	if (!CharacterOwner || CharacterOwner->IsFiringDisabled())
	{
		return;
	}
	bool bClientFired = BeginFiringSequence(InFireMode, false);
	if (!HasAuthority())
	{
		ServerStartFire(InFireMode, bClientFired);
	}
}

void AWeapon::StopFire(uint8 InFireMode)
{
	UE_LOG(LogTemp, Warning, TEXT("STOP Firing Mode (%d)."), InFireMode);
	EndFiringSequence(InFireMode);
	if (!HasAuthority())
	{
		ServerStopFire(InFireMode);
	}
}

void AWeapon::ServerStartFire_Implementation(uint8 InFireMode, bool bClientFired)
{
	if (!CharacterOwner->IsEquipped(this) && !CharacterOwner->IsLocallyControlled())
	{
		CharacterOwner->ClientVerifyWeapon();
		return;
	}
	BeginFiringSequence(InFireMode, bClientFired);
}

void AWeapon::ServerStopFire_Implementation(uint8 InFireMode)
{
	EndFiringSequence(InFireMode);
}

bool AWeapon::BeginFiringSequence(uint8 InFireMode, bool bClientFired)
{
	if (CharacterOwner)
	{
		SetPendingFire(InFireMode, true);
		return CurrentState->BeginFiringSequence(InFireMode, bClientFired);
	}
	return false;
}

void AWeapon::EndFiringSequence(uint8 InFireMode)
{
	if (CharacterOwner)
	{
		SetPendingFire(InFireMode, false);
		CurrentState->EndFiringSequence(InFireMode);
	}
}

bool AWeapon::HandleContinuedFiring()
{
	ensure(CharacterOwner);
	if (!CanFireAgain() || !CharacterOwner || !IsPendingFire(CurrentFireMode))
	{
		GotoState(ActiveState);
		return false;
	}
	return true;
}

void AWeapon::FireShot()
{
	ConsumeAmmoInClip(CurrentFireMode);
	
	const FVector StartLoc = GetFireStartLocation(CurrentFireMode);
	const FRotator BaseRot = GetBaseFireRotation();

	const FVector FireDir = BaseRot.Vector();
	const FVector EndTrace = StartLoc + FireDir * 1000.f;

	const ECollisionChannel TraceChannel = COLLISION_TRACE_WEAPON;
	
	FHitResult OutHit;
	FCollisionQueryParams TraceParams(GetClass()->GetFName(), false, CharacterOwner);
	GetWorld()->LineTraceSingleByChannel(OutHit, StartLoc, EndTrace, TraceChannel, TraceParams);

	if (OutHit.bBlockingHit)
	{
		UE_LOG(LogTemp, Log, TEXT("Hit!!! (Target=%s)"), *GetNameSafe(OutHit.GetActor()));
	}

	CharacterOwner->SetFlashLocation(OutHit.bBlockingHit ? OutHit.ImpactPoint : EndTrace, CurrentFireMode);
	PlayFiringEffects();
}

bool AWeapon::CanFireAgain() const
{
	return (CharacterOwner && !CharacterOwner->GetPendingWeapon()) && HasAmmoInClip(CurrentFireMode);
}

float AWeapon::GetRefireTime(uint8 InFireMode) const
{
	if (FireModes.IsValidIndex(InFireMode))
	{
		return FireModes[InFireMode].FiringInterval;
	}
	return 0.1f;
}

FVector AWeapon::GetFireStartLocation(uint8 FireMode) const
{
	if (!CharacterOwner)
	{
		UE_LOG(LogTemp, Warning, TEXT("AWeapon::GetFireStartLocation -- Missing Character Owner (Weapon=%s)"), *GetName());
		return FVector::Zero();
	}

	const bool bIsFirstPerson = Cast<APlayerController>(CharacterOwner->GetController()) != nullptr;
	return bIsFirstPerson ? CharacterOwner->GetPawnViewLocation() : CharacterOwner->GetActorLocation();;
}

FRotator AWeapon::GetBaseFireRotation() const
{
	if (!CharacterOwner)
	{
		UE_LOG(LogTemp, Warning, TEXT("AWeapon::GetBaseFireRotation - Missing Character Owner (Weapon=%s)"), *GetName());
		return FRotator::ZeroRotator;
	}
	return CharacterOwner->GetViewRotation();
}

void AWeapon::PlayFiringEffects()
{
	if (CharacterOwner)
	{
		CharacterOwner->PlayAnimMontage1P(FireModes[CurrentFireMode].FireAnim);
		if (AWeaponAttachment* WeaponAttachment = CharacterOwner->GetEquippedWeaponAttachment())
		{
			WeaponAttachment->PlayFiringEffects(CurrentFireMode);
		}
	}
}

void AWeapon::SpawnTrailEffect(uint8 FireMode, const FVector& EndPoint)
{
	const FVector StartPoint = GetMuzzleLocation();
	DrawDebugLine(GetWorld(), StartPoint, EndPoint, FColor::Green, false, 1.0f, 0, 0.3f);
}

void AWeapon::SpawnImpactEffects(uint8 FireMode, const FVector& EndPoint)
{
	FHitResult Hit = AWeapon::GetImpactEffectHit(CharacterOwner, GetMuzzleLocation(), EndPoint);
	if (AActor* HitActor = Hit.GetActor())
	{
		const bool bHitCharacter = HitActor->IsA(ABaseFPSCharacter::StaticClass());
		DrawDebugPoint(GetWorld(), EndPoint, bHitCharacter ? 10.f : 6.f,  bHitCharacter ? FColor::Red : FColor::Green, false, 0.8f);		
	}
}

FVector AWeapon::GetMuzzleLocation() const
{
	return Mesh->GetSocketLocation(MuzzleAttachPoint);
}

FVector AWeapon::GetMuzzleDirection() const
{
	return Mesh->GetSocketRotation(MuzzleAttachPoint).Vector();
}

FHitResult AWeapon::GetImpactEffectHit(APawn* Shooter, const FVector& StartLoc, const FVector& TargetLoc)
{
	FHitResult Hit;
	FVector TargetToGun = (StartLoc - TargetLoc).GetSafeNormal();
	const bool bHit = Shooter->GetWorld()->LineTraceSingleByChannel(Hit,
		TargetLoc + TargetToGun * 32.0f,
		TargetLoc - TargetToGun * 32.0f,
		COLLISION_TRACE_WEAPON,
		FCollisionQueryParams(FName(TEXT("ImpactEffect")), true, Shooter));
	return bHit ? Hit : FHitResult(NULL, NULL, TargetLoc, TargetToGun);
}

bool AWeapon::HasInfiniteAmmo() const
{
	return bInfiniteAmmo;
}

bool AWeapon::HasInfiniteClip() const
{
	return bInfiniteClip;
}

/************************************************************************/
/* Ammo/Reloading                                                       */
/************************************************************************/

void AWeapon::OnRep_Ammo()
{
	if (CharacterOwner)
	{
		CharacterOwner->NotifyAmmoUpdated(this);
	}
}

bool AWeapon::HasAmmoInClip(uint8 InFireMode) const
{
	return FireModes.IsValidIndex(InFireMode) && CurrentAmmoInClip >= FireModes[InFireMode].AmmoCost;
}

void AWeapon::ConsumeAmmoInClip(int32 InFireMode)
{
	if (!FireModes.IsValidIndex(InFireMode))
	{
		UE_LOG(LogTemp, Warning, TEXT("Attempted to consume ammo for invalid fire mode (Weapon=%s, FireModeIndex=%d)"), *GetName(), InFireMode);
		return;
	}
	if (HasAuthority())
	{
		if (!HasInfiniteClip())
		{
			CurrentAmmoInClip = FMath::Max(CurrentAmmoInClip - FireModes[InFireMode].AmmoCost, 0);
			if (CharacterOwner)
			{
				CharacterOwner->NotifyAmmoUpdated(this);
			}
		}
	}
}

void AWeapon::AddAmmoToReserve(int32 AddAmount)
{
	if (HasAuthority())
	{
		CurrentReserveAmmo = FMath::Clamp(CurrentReserveAmmo + AddAmount, 0, MaxReserveAmmo);
		if (CharacterOwner)
		{
			CharacterOwner->NotifyAmmoUpdated(this);
		}
	}
}

int32 AWeapon::GetMaxReserveAmmo() const
{
	return MaxReserveAmmo;
}

int32 AWeapon::GetCurrentReserveAmmo() const
{
	return CurrentReserveAmmo;
}

int32 AWeapon::GetMaxAmmoPerClip() const
{
	return MaxAmmoPerClip;
}

int32 AWeapon::GetMaxTotalAmmo() const
{
	return MaxReserveAmmo + MaxAmmoPerClip;
}

int32 AWeapon::GetCurrentAmmoInClip() const
{
	return CurrentAmmoInClip;
}

int32 AWeapon::GetCurrentTotalAmmo() const
{
	return CurrentReserveAmmo + CurrentAmmoInClip;
}

void AWeapon::StartReload()
{
	if (CharacterOwner && !CharacterOwner->IsFiringDisabled())
	{
		if (!HasAuthority())
		{
			ServerStartReload();
		}
		SetPendingReload(true);
		CurrentState->Reload();		
	}
}

void AWeapon::StopReload()
{
	if (CharacterOwner && !CharacterOwner->IsFiringDisabled())
	{
		if (!HasAuthority())
		{
			ServerStopReload();
		}
		SetPendingReload(false);
	}
}

void AWeapon::ServerStartReload_Implementation()
{
	if (!CharacterOwner->IsEquipped(this) && !CharacterOwner->IsLocallyControlled())
	{
		CharacterOwner->ClientVerifyWeapon();
		return;
	}
	StartReload();
}

void AWeapon::ServerStopReload_Implementation()
{
	StopReload();
}

void AWeapon::OnReloadFinished()
{
	if (HasAuthority())
	{
		int32 ClipDelta = FMath::Min(MaxAmmoPerClip - CurrentAmmoInClip, CurrentReserveAmmo);
		if (HasInfiniteAmmo())
		{
			// in case we didn't have enough reserve ammo for some reason
			ClipDelta = MaxAmmoPerClip - CurrentAmmoInClip;
		}
		if (ClipDelta > 0)
		{
			CurrentAmmoInClip += ClipDelta;
			if (!HasInfiniteAmmo())
			{
				CurrentReserveAmmo -= ClipDelta;
			}
		}
		if (CharacterOwner)
		{
			CharacterOwner->NotifyAmmoUpdated(this);
		}
	}
}

bool AWeapon::CanReload() const
{
	const bool bCanReload = CharacterOwner && !CharacterOwner->IsFiringDisabled();
	const bool bHasAmmo = (CurrentAmmoInClip < MaxAmmoPerClip) && (CurrentReserveAmmo > 0);
	return bCanReload && bHasAmmo;
}

bool AWeapon::IsPendingReload() const
{
	return bPendingReload;
}

void AWeapon::SetPendingReload(bool bInPendingReload)
{
	bPendingReload = bInPendingReload;
}

float AWeapon::GetReloadTime() const
{
	return ReloadTime;
}

/************************************************************************/
/* Accessors                                                            */
/************************************************************************/

/* -------------- Visuals -------------- */

FText AWeapon::GetDisplayName() const
{
	return DisplayName;
}

USkeletalMeshComponent* AWeapon::GetMesh() const
{
	return Mesh;
}

TSubclassOf<AWeaponAttachment> AWeapon::GetWeaponAttachmentType() const
{
	return WeaponAttachmentType;
}

EAnimPose AWeapon::GetWeaponAnimPose() const
{
	return WeaponAnimPose;
}

/* -------------- User Interface -------------- */

UTexture2D* AWeapon::GetPrimaryIcon() const
{
	return PrimaryIcon;
}

UTexture2D* AWeapon::GetAmmoIcon() const
{
	return AmmoIcon;
}

/* -------------- Inventory -------------- */

float AWeapon::GetBringUpTime() const
{
	return BringUpTime;
}

float AWeapon::GetPutDownTime() const
{
	return PutDownTime;
}

TSubclassOf<APickupInstance_Weapon> AWeapon::GetPickupClass() const
{
	return PickupClass;
}

bool AWeapon::CanBeThrown() const
{
	return bCanBeThrown;
}

bool AWeapon::IsEquipping() const
{
	return CurrentState == EquippingState;
}

bool AWeapon::IsUnequipping() const
{
	return CurrentState == UnequippingState;
}
