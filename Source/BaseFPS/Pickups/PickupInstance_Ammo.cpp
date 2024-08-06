// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/PickupInstance_Ammo.h"

#include "Components/InteractableComponent.h"
#include "Components/SphereComponent.h"

APickupInstance_Ammo::APickupInstance_Ammo(const FObjectInitializer& ObjectInitializer)
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

	InteractableComponent->SetInteractableNameText(FText::FromString("Ammo"));
	InteractableComponent->SetInteractableIcon(AmmoIcon);
	
	RespawnTime = 30.0f;
}

/************************************************************************/
/* Editor Only                                                          */
/************************************************************************/
#if WITH_EDITOR
void APickupInstance_Ammo::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (const FProperty* PropertyChanged = PropertyChangedEvent.Property)
	{
		if (PropertyChanged->GetName() == FString(TEXT("AmmoIcon")))
		{
			InteractableComponent->SetInteractableIcon(AmmoIcon);
		}
	}
}
#endif


/************************************************************************/
/* Visuals                                                              */
/************************************************************************/

UMeshComponent* APickupInstance_Ammo::GetEditorMeshTemplate() const
{
	return Mesh;
}
