// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InteractableComponent.h"

#include "BaseFPS.h"
#include "Character/BaseFPSCharacter.h"

// Sets default values for this component's properties
UInteractableComponent::UInteractableComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;

	InteractableHoldTime = 2.0f;
	InteractableDistance = 2000.0f; // 2 meters
	InteractableNameText = FText::FromString("Interactable");
	InteractableVerbText = FText::FromString("Use with");
	bAllowMultipleInteractors = false;

	InteractableComponentType = EInteractableComponentType::ICT_Basic;
	
	SetActive(true);
}


// Called when the game starts
void UInteractableComponent::BeginPlay()
{
	Super::BeginPlay();

	// caching value for optimization
	InteractableDistanceSquared = FMath::Square(InteractableDistance);
	
}

void UInteractableComponent::BeginInteract(ABaseFPSCharacter* Character)
{
	if (IsActive() && IsValid(Character) && !Character->IsDead())
	{
		if (CanInteract(Character))
		{
			Interactors.AddUnique(Character);
			OnBeginInteract.Broadcast(Character);
		}
	}
}

void UInteractableComponent::EndInteract(ABaseFPSCharacter* Character)
{
	if (IsActive() && IsValid(Character))
	{
		Interactors.Remove(Character);
		OnEndInteract.Broadcast(Character);
	}
}

void UInteractableComponent::Interact(ABaseFPSCharacter* Character)
{
	if (IsActive() && IsValid(Character) && !Character->IsDead())
	{
		OnInteract.Broadcast(Character);
		Interactors.Remove(Character);
	}
}

void UInteractableComponent::Deactivate()
{
	for (int32 i = 0; i < Interactors.Num(); i++)
	{
		if (ABaseFPSCharacter* Interactor = Interactors[i])
		{
			EndFocus(Interactor);
			EndInteract(Interactor);
		}
	}
	Interactors.Empty(); // clear array
	
	Super::Deactivate();
}

bool UInteractableComponent::CanInteract(ABaseFPSCharacter* Character) const
{
	const bool bPlayerAlreadyInteracting = !bAllowMultipleInteractors && Interactors.Num() > 0;
	return !bPlayerAlreadyInteracting && IsActive() && IsValid(Character);
}

/************************************************************************/
/* Accessors                                                            */
/************************************************************************/

/* -------------- Setters  -------------- */

void UInteractableComponent::SetInteractableHoldTime(float InInteractableHoldTime)
{
	InteractableHoldTime = InInteractableHoldTime;
}

void UInteractableComponent::SetInteractableDistance(float InInteractableDistance)
{
	InteractableDistance = InInteractableDistance;
}

void UInteractableComponent::SetInteractableNameText(FText InInteractableNameText)
{
	InteractableNameText = InInteractableNameText;
}

void UInteractableComponent::SetInteractableVerbText(FText InInteractableVerbText)
{
	InteractableVerbText = InInteractableVerbText;
}

void UInteractableComponent::SetInteractableIcon(UTexture2D* InInteractableIcon)
{
	InteractableIcon = InInteractableIcon;
}

/* -------------- Getters -------------- */


FText UInteractableComponent::GetInteractableInputEventText() const
{
	return FMath::IsNearlyZero(InteractableHoldTime) ?
		EInteractableInputTypeText::PRESS : EInteractableInputTypeText::HOLD;
}

float UInteractableComponent::GetInteractableDistance() const
{
	return InteractableDistance;
}

float UInteractableComponent::GetInteractableDistanceSquared() const
{
	return InteractableDistanceSquared;
}

float UInteractableComponent::GetInteractableHoldTime() const
{
	return InteractableHoldTime;
}

FText UInteractableComponent::GetInteractableNameText() const
{
	return InteractableNameText;
}

FText UInteractableComponent::GetInteractionVerbText() const
{
	return InteractableVerbText;
}

UTexture2D* UInteractableComponent::GetInteractableIcon() const
{
	return InteractableIcon;
}

EInteractableComponentType UInteractableComponent::GetInteractableComponentType()
{
	return InteractableComponentType;
}



