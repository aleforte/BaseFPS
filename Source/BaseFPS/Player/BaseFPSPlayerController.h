// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "CommonPlayerController.h"
#include "BaseFPSPlayerController.generated.h"

class ABaseFPSCharacter;
class UInputMappingContext;
class UInputAction;
/**
 * 
 */
UCLASS()
class BASEFPS_API ABaseFPSPlayerController : public ACommonPlayerController
{
	GENERATED_BODY()

public:
	ABaseFPSPlayerController(const FObjectInitializer& ObjectInitializer);

protected:	
	//~ Begin PlayerController interface
	virtual void BeginPlay() override;
	virtual void SetPawn(APawn* InPawn) override;
	virtual void SetupInputComponent() override;
	//~ End PlayerController interface
	
	/************************************************************************/
	/* Input                                                                */
	/************************************************************************/
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input", meta=(AllowPrivateAccess = "true"))
	UInputAction* ToggleMenuAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input", meta=(AllowPrivateAccess = "true"))
	UInputAction* MoveAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input", meta=(AllowPrivateAccess = "true"))
	UInputAction* JumpAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input", meta=(AllowPrivateAccess = "true"))
	UInputAction* CrouchAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* InteractAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* FireAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* AltFireAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* ReloadAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* NextWeaponAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* PrevWeaponAction;
	
public:
	/** Applies custom key mappings from player settings */
	void ApplyCustomPlayerKeyMappings() const;

	/** returns FText of the currently mapped interact action key */
	FText GetNameTextForInteractActionKey(uint32 KeyIndex = 0) const;
	
	/************************************************************************/
	/* Character                                                            */
	/************************************************************************/
protected:
	UPROPERTY(BlueprintReadOnly, Category="Controller")
	TObjectPtr<ABaseFPSCharacter> MyCharacter;

public:
	UFUNCTION(BlueprintGetter, Category="Controller")
	ABaseFPSCharacter* GetMyCharacter() const;

	/* -------------- Character Actions -------------- */
protected:
	void ToggleInGameMenu();
	
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	
	void OnJump();
	void OnJumpReleased();
	
	void OnCrouch(bool bClientSimulation);
	void OnCrouchReleased(bool bClientSimulation);
	
	void OnInteract();
	void OnInteractReleased();

	void OnFire();
	void OnFireReleased();
	void OnAltFire();
	void OnAltFireReleased();

	void OnReload();
	void OnReloadReleased();

	void OnNextWeapon();
	void OnPrevWeapon();
	
};
