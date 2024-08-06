// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "InteractableComponent.generated.h"

class ABaseFPSCharacter;

#define LOCTEXT_NAMESPACE "BaseFPS"

namespace EInteractableInputTypeText
{
	const static FText PRESS = NSLOCTEXT("InteractionInputEvent", "Press", "Press");
	const static FText HOLD = NSLOCTEXT("InteractionInputEvent", "Hold", "Hold");
}

/**
 * The type of interactable this component represents, i.e. pickup, health station, etc.
 * (useful for opening/closing interaction cards on the HUD)
 */
UENUM(BlueprintType)
enum EInteractableComponentType
{
	ICT_Basic = 0	UMETA(DisplayName = "Basic"),
	ICT_Pickup = 1  UMETA(DisplayName = "Pickup")
};

/**
 * Attach this component to Actors to make them interactable in-game
 *
 * Important Note:
 * To make the actor detectable by players, actor's "Interactable" channel (ECC_GameTraceChannel1) must be set to "Overlap"
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), HideCategories=(Variable, Sockets, Tags, AssetUserData, Collision, Replication))
class BASEFPS_API UInteractableComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UInteractableComponent(const FObjectInitializer& ObjectInitializer);
	
protected:
	/** the name that will come up when the player looks at the interactable */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interactable")
	FText InteractableNameText;

	/** the verb that describes how the interaction works, i.e. "pickup" for a chair, "eat" for food, etc... */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interactable")
	FText InteractableVerbText;

	/** the default icon displayed on HUD when player approaches the interactable */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interactable")
	UTexture2D* InteractableIcon;

	/** the time the player must hold the interact key to interact with this object */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interactable")
	float InteractableHoldTime;

	/** The max distance the player can be away from this actor before you can interact */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interactable")
	float InteractableDistance;
	UPROPERTY(Transient)
	float InteractableDistanceSquared; // cached version of distance squared for optimization
	
	/** whether we allow multiple players to interact with the item, or just one at any given time */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interactable")
	bool bAllowMultipleInteractors;
	
	/** the interaction type that this component represents */
	UPROPERTY(EditAnywhere, Category="Interactable")
	TEnumAsByte<EInteractableComponentType> InteractableComponentType;
	
protected:
	/** list of players interacting with this component on server (on client, will just be local player) */
	UPROPERTY(Transient)
	TArray<ABaseFPSCharacter*> Interactors;

public:
	/************************************************************************/
	/* Delegates                                                            */
	/************************************************************************/

	/** [local + server] called when the player presses the interact key while focusing on this interactable actor */
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnBeginInteractSignature, ABaseFPSCharacter*);
	FOnBeginInteractSignature OnBeginInteract;

	/** [local & server] called when the player releases the interact key, stops looking at actor, or gets too far away */
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnEndInteractSignature, ABaseFPSCharacter*);
	FOnEndInteractSignature OnEndInteract;

	/** [local & server] called when the player has interacted with the item for the required amount of time */
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnInteractSignature, ABaseFPSCharacter*);
	FOnInteractSignature OnInteract;
	
	protected:
	void BeginPlay() override;
	
public:
	/** called on the client when the player's interaction check begins/ends focus on this item */
	void BeginFocus(ABaseFPSCharacter* Character) {}
	void EndFocus(ABaseFPSCharacter* Character) {}

	/** called on the client when the player begins/ends interacting with this items */
	void BeginInteract(ABaseFPSCharacter* Character);
	void EndInteract(ABaseFPSCharacter* Character);

	virtual void Interact(ABaseFPSCharacter* Character);

	void Deactivate() override;
	virtual bool CanInteract(ABaseFPSCharacter* Character) const;
	
	/************************************************************************/
	/* Accessors                                                            */
	/************************************************************************/

	/* -------------- Setters -------------- */

	UFUNCTION(BlueprintCallable, Category="Interactable")
	inline void SetInteractableHoldTime(float InInteractableHoldTime);

	UFUNCTION(BlueprintCallable, Category="Interactable")
	inline void SetInteractableDistance(float InInteractableDistance);

	UFUNCTION(BlueprintCallable, Category="Interactable")
	inline void SetInteractableNameText(FText InInteractableNameText);

	UFUNCTION(BlueprintCallable, Category="Interactable")
	inline void SetInteractableVerbText(FText InInteractableVerbText);

	UFUNCTION(BlueprintCallable, Category="Interactable")
	inline void SetInteractableIcon(UTexture2D* InInteractableIcon);
	
	/* -------------- Getters -------------- */
public:
	/** the input event required to interact, hold or press */
	FText GetInteractableInputEventText() const;

	UFUNCTION(BlueprintPure, Category="Interactable")
	inline float GetInteractableDistance() const;
	
	UFUNCTION(BlueprintPure, Category="Interactable")
	inline float GetInteractableDistanceSquared() const;
	
	UFUNCTION(BlueprintPure, Category="Interactable")
	inline float GetInteractableHoldTime() const;
	
	UFUNCTION(BlueprintPure, Category="Interactable")
	virtual inline FText GetInteractableNameText() const;

	UFUNCTION(BlueprintPure, Category="Interactable")
	inline FText GetInteractionVerbText() const;

	UFUNCTION(BlueprintPure, Category="Interactable")
	inline UTexture2D* GetInteractableIcon() const;

	UFUNCTION(BlueprintPure, Category="Interactable")
	inline EInteractableComponentType GetInteractableComponentType();
};

#undef LOCTEXT_NAMESPACE