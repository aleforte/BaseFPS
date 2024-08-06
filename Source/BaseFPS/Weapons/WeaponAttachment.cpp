// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/WeaponAttachment.h"

#include "BaseFPS.h"
#include "Weapon.h"
#include "Character/BaseFPSCharacter.h"

// Weapon attach points for {@code ABaseFPSCharacter}
static FName Mesh3PGripPoint = "GripPoint_3P";
static FName Mesh3PHipHolster = "Holster_Hip";
static FName Mesh3PBackHolster = "Holster_Back";

// Weapon attach point for muzzle/particle effects
static FName MuzzleAttachPoint = "MuzzleAttachPoint";

// Sets default values
AWeaponAttachment::AWeaponAttachment(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;
	
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	Mesh = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("Mesh3P"));
	Mesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	Mesh->bReceivesDecals = false;
	Mesh->CastShadow = true;
	Mesh->SetCollisionObjectType(ECC_WorldDynamic);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetCollisionResponseToChannels(ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	Mesh->SetOwnerNoSee(true);
	Mesh->SetHiddenInGame(true);
	RootComponent = Mesh;

	CharacterOwner = nullptr;
	
	EquippedAttachSocket = Mesh3PGripPoint;
	HolsterAttachSocket = Mesh3PBackHolster;
}

// Called when the game starts or when spawned
void AWeaponAttachment::BeginPlay()
{
	Super::BeginPlay();

	CharacterOwner = Cast<ABaseFPSCharacter>(Owner);
}

void AWeaponAttachment::Destroyed()
{
	DetachFromOwner();
	Super::Destroyed();
}

void AWeaponAttachment::AttachToOwnerEquipped()
{
	if (!CharacterOwner)
	{
		CharacterOwner = Cast<ABaseFPSCharacter>(Owner);
	}

	Mesh->AttachToComponent(CharacterOwner->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, EquippedAttachSocket);
	Mesh->bRecentlyRendered = CharacterOwner->GetMesh()->bRecentlyRendered;
}

void AWeaponAttachment::AttachToOwnerHolstered()
{
	if (!CharacterOwner)
	{
		CharacterOwner = Cast<ABaseFPSCharacter>(Owner);
	}

	Mesh->AttachToComponent(CharacterOwner->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, HolsterAttachSocket);
	Mesh->bRecentlyRendered = CharacterOwner->GetMesh()->bRecentlyRendered;
}

void AWeaponAttachment::DetachFromOwner()
{
	DetachFromActor(FDetachmentTransformRules::KeepRelativeTransform);
}

void AWeaponAttachment::PlayFiringEffects(uint8 FireMode)
{
	UE_LOG(LogTemp, Log, TEXT("Firing!!!!!!!!"));
	if (CharacterOwner)
	{
		CharacterOwner->PlayAnimMontage(FireAnim);
	}
}

void AWeaponAttachment::SpawnTrailEffect(uint8 FireMode, const FVector& EndPoint)
{
	if (!CharacterOwner->IsLocallyControlled())
	{
		const FVector StartPoint = GetMuzzleLocation();
		DrawDebugLine(GetWorld(), StartPoint, EndPoint, FColor::Green, false, 1.0f, 0, 0.3f);	
	}
}

void AWeaponAttachment::SpawnImpactEffects(uint8 FireMode, const FVector& EndPoint)
{
	if (!CharacterOwner->IsLocallyControlled())
	{
		FHitResult Hit = AWeapon::GetImpactEffectHit(CharacterOwner, GetMuzzleLocation(), EndPoint);
		if (AActor* HitActor = Hit.GetActor())
		{
			const bool bHitCharacter = HitActor->IsA(ABaseFPSCharacter::StaticClass());
			DrawDebugPoint(GetWorld(), EndPoint, bHitCharacter ? 10.f : 6.f,  bHitCharacter ? FColor::Red : FColor::Green, false, 0.8f);		
		}
	}
}

FVector AWeaponAttachment::GetMuzzleLocation() const
{
	return Mesh->GetSocketLocation(MuzzleAttachPoint);
}

FVector AWeaponAttachment::GetMuzzleDirection() const
{
	return Mesh->GetSocketRotation(MuzzleAttachPoint).Vector();
}


TArray<FName> AWeaponAttachment::GetHolsterAttachPoints() const
{
	return { Mesh3PBackHolster, Mesh3PHipHolster };
}

UAnimMontage* AWeaponAttachment::GetFireAnim() const
{
	return FireAnim;
}

UAnimMontage* AWeaponAttachment::GetBringUpAnim() const
{
	return BringUpAnim;
}

UAnimMontage* AWeaponAttachment::GetPutDownAnim() const
{
	return PutDownAnim;
}

UAnimMontage* AWeaponAttachment::GetReloadAnim() const
{
	return ReloadAnim;
}

