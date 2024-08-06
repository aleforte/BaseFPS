// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/PickupInstance.h"

#include "BaseFPS.h"
#include "Character/BaseFPSCharacter.h"
#include "Components/InteractableComponent.h"
#include "Components/SphereComponent.h"

// Sets default values
APickupInstance::APickupInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bStartWithTickEnabled = false;
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	InteractCollision = ObjectInitializer.CreateDefaultSubobject<USphereComponent>(this, TEXT("Collision"));
	InteractCollision->SetComponentTickEnabled(false);
	InteractCollision->SetCollisionProfileName(FName(TEXT("Pickup")));
	InteractCollision->SetSphereRadius(48.f);
	InteractCollision->SetShouldUpdatePhysicsVolume(false);
	InteractCollision->SetGenerateOverlapEvents(true);
	InteractCollision->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	InteractCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECR_Overlap);
	InteractCollision->SetCollisionResponseToChannel(COLLISION_INTERACTABLE, ECR_Overlap);
	RootComponent = InteractCollision;

	InteractableComponent = ObjectInitializer.CreateDefaultSubobject<UInteractableComponent>(this, TEXT("Interactable"));
	InteractableComponent->SetInteractableHoldTime(0.4f);
	InteractableComponent->SetInteractableDistance(1600.0f);
	InteractableComponent->SetInteractableVerbText(FText::FromString("Pickup"));
}

void APickupInstance::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	InteractableComponent->OnInteract.AddUObject(this, &APickupInstance::OnInteract);
}

// Called when the game starts or when spawned
void APickupInstance::BeginPlay()
{
	Super::BeginPlay();
	PlayEffectsOnSpawn();
}

/************************************************************************/
/* Properties                                                           */
/************************************************************************/

float APickupInstance::GetRespawnTime() const
{
	return RespawnTime;
}

/************************************************************************/
/* Visuals                                                              */
/************************************************************************/

void APickupInstance::PlayEffectsOnGiveTo()
{
	// stub
}

void APickupInstance::PlayEffectsOnSpawn()
{
	// stub
}

UMeshComponent* APickupInstance::GetEditorMeshTemplate() const
{
	// stub
	return nullptr;  
}

/************************************************************************/
/* Pickup Interaction                                                   */
/************************************************************************/

void APickupInstance::OnInteract(ABaseFPSCharacter* Character)
{
	if (HasAuthority() && IsValid(this) && CanBePickedUpBy(Character))
	{
		GiveTo(Character);
		PlayEffectsOnGiveTo();
		Destroy();
	}
}

bool APickupInstance::CanBePickedUpBy(ABaseFPSCharacter* Character)
{
	return IsValid(Character) && Character->CanPickUp(this) &&
		/* trace to see if nothing between character & pickup */
		!GetWorld()->LineTraceTestByChannel(
			Character->GetActorLocation(),
			GetActorLocation(),
			ECC_Pawn,
			FCollisionQueryParams(),
			WorldResponseParams);
}

UShapeComponent* APickupInstance::GetEditorCollisionTemplate() const
{
	return InteractCollision;
}

void APickupInstance::GiveTo(ABaseFPSCharacter* Character)
{
	UE_LOG(LogTemp, Warning, TEXT("Give to %s"), *Character->GetName())
}

/************************************************************************/
/* Overlap                                                              */
/************************************************************************/

void APickupInstance::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (ABaseFPSCharacter* Character = Cast<ABaseFPSCharacter>(OtherActor))
	{
		OnOverlap(Character);
	}
}

void APickupInstance::OnOverlap(ABaseFPSCharacter* Character)
{
	if (HasAuthority() && IsValid(this))
	{
		UE_LOG(LogTemp, Log, TEXT("Overlapped with %s! (Actor=%s)"), *Character->GetName(), *GetName());		
	}
}


