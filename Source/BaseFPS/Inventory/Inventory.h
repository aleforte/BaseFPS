// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Inventory.generated.h"

class ABaseFPSCharacter;

/**
 * Represents an Inventory item/actor stored in a player's inventory
 */
UCLASS(Abstract)
class BASEFPS_API AInventory : public AActor
{
	GENERATED_BODY()
	
public:	
	AInventory();

protected:
	//~ Begin AActor interface
	virtual void BeginPlay() override;
	virtual void Destroyed() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_Owner() override;
	//~ End AActor interface

protected:
	/** Pointer to the @code Owner cast to a Character */
	UPROPERTY(Transient)
	TObjectPtr<ABaseFPSCharacter> CharacterOwner;
	
public:
	UFUNCTION(BlueprintPure, Category="Inventory")
	ABaseFPSCharacter* GetCharacterOwner() const;
	
	/************************************************************************/
	/* Inventory Actions                                                    */
	/************************************************************************/
	
protected:
	/** [client]
	 * client-side call when this item has been added to player's inventory
	 * (This is used to expedite creation of actor on client's side, happens faster than relying on replication)
	 */
	UFUNCTION(Client, Reliable)
	void ClientOnAddedToInventory(ABaseFPSCharacter* NewOwner);
	
	/** [client]
	 * client-side call when this item has been removed from player's inventory
	 * (This is used because replication stops when ownership is removed, clients need to be notified for cleanup)
	 */
	UFUNCTION(Client, Reliable)
	void ClientOnRemovedFromInventory();
	
public:
	/** [server] item was added to pawn's inventory */
	virtual void OnAddedToInventory(ABaseFPSCharacter* NewOwner);

	/** [server] item was removed from pawn's inventory */
	virtual void OnRemovedFromInventory();
};
