// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/PickupInstance_Shield.h"

#include "Components/InteractableComponent.h"
#include "Components/SphereComponent.h"

APickupInstance_Shield::APickupInstance_Shield(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Mesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("Mesh"));
	Mesh->SetComponentTickEnabled(false);
	Mesh->SetShouldUpdatePhysicsVolume(false);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	Mesh->SetGenerateOverlapEvents(false);
	Mesh->bReceivesDecals = false;
	Mesh->CastShadow = true;
	Mesh->SetHiddenInGame(false);

	RootComponent = Mesh;
	InteractCollision->SetupAttachment(Mesh);

	InteractableComponent->SetInteractableNameText(FText::FromString("Shield"));
	InteractableComponent->SetInteractableIcon(ShieldIcon);
	
	RespawnTime = 30.0f;
}

/************************************************************************/
/* Visuals                                                              */
/************************************************************************/

UMeshComponent* APickupInstance_Shield::GetEditorMeshTemplate() const
{
	return Mesh;
}