// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BaseFPSHUD.generated.h"

class ABaseFPSCharacter;
class UCommonActivatableWidget;
class UBaseFPSHUDLayout;
/**
 * 
 */
UCLASS()
class BASEFPS_API ABaseFPSHUD : public AHUD
{
	GENERATED_BODY()

	ABaseFPSHUD();
	
	//~ Begin AHUD interface
	virtual void PostInitializeComponents() override;
	virtual void ShowHUD() override;
	virtual void Destroyed() override;
	//~ End AHUD interface

	/************************************************************************/
	/* Possessed Pawn                                                       */
	/************************************************************************/

	/** the currently possessed pawn as a character, null if spectating */
	UPROPERTY(Transient)
	TObjectPtr<ABaseFPSCharacter> PossessedCharacter;
	
	/**
	 * handles HUD refresh on Pawn change
	 */
	UFUNCTION()
	virtual void OnPawnChanged(APawn* OldPawn, APawn* NewPawn);
	
	void BindToCharacter(TObjectPtr<ABaseFPSCharacter> InChar);
	void UnbindFromCharacter(TObjectPtr<ABaseFPSCharacter> InChar);

	void OnInventoryUpdated() const;
	void OnEquipNewWeapon() const;
	void OnFocusChanged(class UInteractableComponent* Interactable) const;
	void OnAmmoUpdated(class AWeapon* Weapon) const;
	
	/************************************************************************/
	/* HUD Layout                                                           */
	/************************************************************************/
protected:
	/** pointer to the currently active HUD layout */
	UPROPERTY(BlueprintReadOnly, Category="HUD")
	TObjectPtr<UBaseFPSHUDLayout> PawnHUDLayout;
	
	/** The HUD layout all players use when possessing a pawn in-game */
	UPROPERTY(EditDefaultsOnly, Category="HUD")
	TSubclassOf<UBaseFPSHUDLayout> PawnHUDLayoutClass; // set post-load

	/************************************************************************/
	/* Escape Menu                                                          */
	/************************************************************************/
protected:
	/** pointer to the currently on-screen escape menu, set to NULL if not open */
	UPROPERTY(Transient)
	TObjectPtr<UCommonActivatableWidget> EscapeMenu;

	/** The escape menu class for this HUD */
	UPROPERTY(EditDefaultsOnly, Category="HUD")
	TSubclassOf<UCommonActivatableWidget> EscapeMenuClass;

public:
	void ToggleInGameMenu();
	
	/************************************************************************/
	/* Delegate Handles                                                     */
	/************************************************************************/
protected:
	FDelegateHandle OnEquippedNewWeaponHandle;
	FDelegateHandle OnInventoryUpdatedHandle;
	FDelegateHandle OnFocusChangedHandle;
	FDelegateHandle OnAmmoUpdatedHandle;
	
};
