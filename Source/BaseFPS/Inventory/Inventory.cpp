// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Inventory.h"

#include "Character/BaseFPSCharacter.h"
#include "Net/UnrealNetwork.h"

AInventory::AInventory()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicatingMovement(false);

	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	CharacterOwner = nullptr;
}

void AInventory::BeginPlay()
{
	Super::BeginPlay();
	
}

void AInventory::Destroyed()
{
	Super::Destroyed();
	GetWorldTimerManager().ClearAllTimersForObject(this);
	UE_LOG(LogTemp, Error, TEXT("Destroying Inventory [%s](Owner=%s, bServer=%d)!"), *GetName(), *GetNameSafe(Owner), HasAuthority());
}

void AInventory::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DISABLE_REPLICATED_PRIVATE_PROPERTY(AActor, AttachmentReplication);
	DISABLE_REPLICATED_PRIVATE_PROPERTY(AActor, ReplicatedMovement);
}

void AInventory::OnRep_Owner()
{
	Super::OnRep_Owner();

	if (!CharacterOwner)
	{
		// if we reach here, then we failed to set owner via RPC because
		// owner was null on receipt on client, e.g. character still spawning
		OnAddedToInventory(Cast<ABaseFPSCharacter>(Owner));
	}
}

ABaseFPSCharacter* AInventory::GetCharacterOwner() const
{
	return CharacterOwner;
}

void AInventory::ClientOnAddedToInventory_Implementation(ABaseFPSCharacter* NewOwner)
{
	if (HasAuthority())
	{
		return;
	}
	
	if (NewOwner && !CharacterOwner)
	{
		OnAddedToInventory(NewOwner);
	}
}

void AInventory::ClientOnRemovedFromInventory_Implementation()
{
	if (HasAuthority())
	{
		return;
	}

	OnRemovedFromInventory();

}


void AInventory::OnAddedToInventory(ABaseFPSCharacter* NewOwner)
{
	SetOwner(NewOwner);
	SetInstigator(NewOwner);
	CharacterOwner = NewOwner;
	if (HasAuthority())
	{
		ClientOnAddedToInventory(NewOwner);			
	}
}

void AInventory::OnRemovedFromInventory()
{
	if (HasAuthority())
	{
		ClientOnRemovedFromInventory(); // must call before removing ownership			
	}
	SetInstigator(nullptr);
	SetOwner(nullptr);
	CharacterOwner = nullptr;
}

