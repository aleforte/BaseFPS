// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/PickupInstance_Health.h"

#include "Components/InteractableComponent.h"
#include "Components/SphereComponent.h"

APickupInstance_Health::APickupInstance_Health(const FObjectInitializer& ObjectInitializer)
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

	InteractableComponent->SetInteractableNameText(FText::FromString("Health"));
	InteractableComponent->SetInteractableIcon(HealthIcon);
	
	RespawnTime = 30.0f;
}

/************************************************************************/
/* Visuals                                                              */
/************************************************************************/

UMeshComponent* APickupInstance_Health::GetEditorMeshTemplate() const
{
	return Mesh;
}
