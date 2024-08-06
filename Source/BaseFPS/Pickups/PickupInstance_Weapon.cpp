// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/PickupInstance_Weapon.h"

#include "Components/InteractableComponent.h"
#include "Components/SphereComponent.h"
#include "Weapons/Weapon.h"

APickupInstance_Weapon::APickupInstance_Weapon(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Mesh = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("Mesh"));
	Mesh->SetComponentTickEnabled(false);
	Mesh->SetShouldUpdatePhysicsVolume(false);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	Mesh->SetGenerateOverlapEvents(false);
	Mesh->bReceivesDecals = false;
	Mesh->CastShadow = true;
	Mesh->SetHiddenInGame(false);
	Mesh->SetupAttachment(InteractCollision);
	
	InteractableComponent->SetInteractableNameText(FText::FromString("Weapon"));

	RespawnTime = 30.0f;
	bIsDropped = false;
	DroppedAmmoAmount = 0;
	DroppedPickupLifetime = 30.f;
}

void APickupInstance_Weapon::BeginPlay()
{
	Super::BeginPlay();
	
	// check for potentially overlapping pawns on spawn
	if (HasAuthority())
	{
		TSet<AActor*> OverlappingPawns;
		GetOverlappingActors(OverlappingPawns, ABaseFPSCharacter::StaticClass());
		for (AActor* OverlappingPawn : OverlappingPawns)
		{
			OnOverlap(CastChecked<ABaseFPSCharacter>(OverlappingPawn));
		}

		if (bIsDropped)
		{
			GetWorldTimerManager().SetTimer(DroppedTimerHandle, this, &APickupInstance_Weapon::OnDroppedPickupLifetimeExpired, DroppedPickupLifetime, false);
		}
	}
}

void APickupInstance_Weapon::Destroyed()
{
	GetWorldTimerManager().ClearAllTimersForObject(this);
	Super::Destroyed();
}

/************************************************************************/
/* Editor Only                                                          */
/************************************************************************/
#if WITH_EDITOR

void APickupInstance_Weapon::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (const FProperty* PropertyChanged = PropertyChangedEvent.Property)
	{
		if (PropertyChanged->GetName() == FString(TEXT("WeaponType")))
		{
			WeaponTypeUpdated();
		}
	}
}

void APickupInstance_Weapon::WeaponTypeUpdated() const
{
	if (WeaponType)
	{
		InteractableComponent->SetInteractableNameText(WeaponType.GetDefaultObject()->GetDisplayName());
		InteractableComponent->SetInteractableIcon(WeaponType.GetDefaultObject()->GetPrimaryIcon());
	}
	else
	{
		InteractableComponent->SetInteractableNameText(FText::FromString("Weapon"));
		InteractableComponent->SetInteractableIcon(nullptr);
	}
}

TSubclassOf<AWeapon> APickupInstance_Weapon::GetWeaponType() const
{
	return WeaponType;
}

void APickupInstance_Weapon::GiveTo(ABaseFPSCharacter* Character)
{
	Super::GiveTo(Character);
	
	if (Character)
	{
		if (AInventory* Inv = Character->GetInventoryOfType(WeaponType))
		{
			Cast<AWeapon>(Inv)->AddAmmoToReserve(30);
			return;
		}
		
		AWeapon* InvToAdd = ( DroppedWeapon != nullptr ) ? DroppedWeapon :
			GetWorld()->SpawnActor<AWeapon>(WeaponType, FVector::ZeroVector, FRotator::ZeroRotator);
		
		if (!Character->IsInventoryFull())
		{
			if (!Character->AddInventory(InvToAdd, true)) // if not successful
			{
				UE_LOG(LogTemp, Warning, TEXT("Failed to give pickup item (%s) to character (%s)"), *InvToAdd->GetName(), *Character->GetName());
				InvToAdd->Destroy();
			}
			return;
		}

		// if we're here, then the character's inventory is full. Force character to swap equipped with pickup
		if (AWeapon* Equipped = Character->GetEquippedWeapon())
		{
			if (Character->ReplaceInventory(Equipped, InvToAdd))
			{
				Equipped->DropFrom(Character->GetActorLocation());
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Failed to give pickup item (%s) to character (%s)"), *InvToAdd->GetName(), *Character->GetName());
				InvToAdd->Destroy();
			}
		}
	}
}

#endif

/************************************************************************/
/* Dropped Weapon                                                       */
/************************************************************************/

void APickupInstance_Weapon::SetDroppedWeapon(AWeapon* WeaponToDrop)
{
	bIsDropped = true;
	DroppedWeapon = WeaponToDrop;
	WeaponType = WeaponToDrop->GetClass();
	// DroppedAmmoAmount = WeaponToDrop->GetAmmoSupply(); TODO (aleforte) add ammo
}

void APickupInstance_Weapon::OnDroppedPickupLifetimeExpired()
{
	if (HasAuthority())
	{
		if (DroppedWeapon)
		{
			UE_LOG(LogTemp, Error, TEXT("OnDropppedPickupLifetimeExpired!!!!!! (Weapon=%s)"), *DroppedWeapon->GetName());
			DroppedWeapon->Destroy();
		}
		Destroy();
	}
}

/************************************************************************/
/* Visuals                                                              */
/************************************************************************/

UMeshComponent* APickupInstance_Weapon::GetEditorMeshTemplate() const
{
	return Mesh;
}
