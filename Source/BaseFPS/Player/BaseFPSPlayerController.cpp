// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/BaseFPSPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Character/BaseFPSCharacter.h"
#include "Settings/BaseFPSSettings.h"
#include "Settings/BaseFPSSettingsLocal.h"
#include "PlayerMappableInputConfig.h"
#include "UI/BaseFPSHUD.h"


ABaseFPSPlayerController::ABaseFPSPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Structure to hold one-time initialization
	struct FConstructorStatics
	{	
		ConstructorHelpers::FObjectFinder<UInputAction> ToggleMenuAction;
		ConstructorHelpers::FObjectFinder<UInputAction> ShowScoreboardAction;
		ConstructorHelpers::FObjectFinder<UInputAction> MoveAction;
		ConstructorHelpers::FObjectFinder<UInputAction> LookAction;
		ConstructorHelpers::FObjectFinder<UInputAction> JumpAction;
		ConstructorHelpers::FObjectFinder<UInputAction> CrouchAction;
		ConstructorHelpers::FObjectFinder<UInputAction> InteractAction;
		ConstructorHelpers::FObjectFinder<UInputAction> FireAction;
		ConstructorHelpers::FObjectFinder<UInputAction> AltFireAction;
		ConstructorHelpers::FObjectFinder<UInputAction> ReloadAction;
		ConstructorHelpers::FObjectFinder<UInputAction> NextWeaponAction;
		ConstructorHelpers::FObjectFinder<UInputAction> PrevWeaponAction;
		FConstructorStatics()
			: ToggleMenuAction(TEXT("/Game/Input/Actions/IA_ToggleMenu"))
			, ShowScoreboardAction(TEXT("/Game/Input/Actions/IA_ShowScoreboard"))
			, MoveAction(TEXT("/Game/Input/Actions/IA_Move"))
			, LookAction(TEXT("/Game/Input/Actions/IA_Look_Mouse"))
		    , JumpAction(TEXT("/Game/Input/Actions/IA_Jump"))
			, CrouchAction(TEXT("/Game/Input/Actions/IA_Crouch"))
			, InteractAction(TEXT("/Game/Input/Actions/IA_Interact"))
			, FireAction(TEXT("/Game/Input/Actions/IA_Fire"))
			, AltFireAction(TEXT("/Game/Input/Actions/IA_Fire_Alt"))
			, ReloadAction(TEXT("/Game/Input/Actions/IA_Reload"))
			, NextWeaponAction(TEXT("/Game/Input/Actions/IA_NextWeapon"))
			, PrevWeaponAction(TEXT("/Game/Input/Actions/IA_PrevWeapon"))
		{}
	};
	static FConstructorStatics ConstructorStatics;

	ToggleMenuAction = ConstructorStatics.ToggleMenuAction.Object;
	ShowScoreboardAction = ConstructorStatics.ShowScoreboardAction.Object;
	MoveAction = ConstructorStatics.MoveAction.Object;
	LookAction = ConstructorStatics.LookAction.Object;
	JumpAction = ConstructorStatics.JumpAction.Object;
	CrouchAction = ConstructorStatics.CrouchAction.Object;
	InteractAction = ConstructorStatics.InteractAction.Object;
	FireAction = ConstructorStatics.FireAction.Object;
	AltFireAction = ConstructorStatics.AltFireAction.Object;
	ReloadAction = ConstructorStatics.ReloadAction.Object;
	NextWeaponAction = ConstructorStatics.NextWeaponAction.Object;
	PrevWeaponAction = ConstructorStatics.PrevWeaponAction.Object;
}

ABaseFPSCharacter* ABaseFPSPlayerController::GetMyCharacter() const
{
	return MyCharacter;
}

void ABaseFPSPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
	{
		//Menus & Scoreboard
		EnhancedInputComponent->BindAction(ToggleMenuAction, ETriggerEvent::Triggered, this, &ThisClass::ToggleInGameMenu);
		EnhancedInputComponent->BindAction(ShowScoreboardAction, ETriggerEvent::Started, this, &ThisClass::ShowScoreboard);
		EnhancedInputComponent->BindAction(ShowScoreboardAction, ETriggerEvent::Completed, this, &ThisClass::HideScoreboard);
		
		//Moving & Looking
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Look);
		
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ThisClass::OnJump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ThisClass::OnJumpReleased);

		// Crouching
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &ThisClass::OnCrouch, false);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &ThisClass::OnCrouchReleased, false);

		// Interacting
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this,  &ThisClass::OnInteract);
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Completed, this,  &ThisClass::OnInteractReleased);
		
		// Actions
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ThisClass::OnFire);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &ThisClass::OnFireReleased);
		EnhancedInputComponent->BindAction(AltFireAction, ETriggerEvent::Started, this, &ThisClass::OnAltFire);
		EnhancedInputComponent->BindAction(AltFireAction, ETriggerEvent::Completed, this, &ThisClass::OnAltFireReleased);
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &ThisClass::OnReload);
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Completed, this, &ThisClass::OnReloadReleased);
		EnhancedInputComponent->BindAction(NextWeaponAction, ETriggerEvent::Triggered, this, &ThisClass::OnNextWeapon);
		EnhancedInputComponent->BindAction(PrevWeaponAction, ETriggerEvent::Triggered, this, &ThisClass::OnPrevWeapon);
	}	
}

void ABaseFPSPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController())
	{
		ApplyCustomPlayerKeyMappings();			
	}
}

void ABaseFPSPlayerController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);

	MyCharacter = Cast<ABaseFPSCharacter>(InPawn);
}

void ABaseFPSPlayerController::ApplyCustomPlayerKeyMappings() const
{
	if (UBaseFPSSettingsLocal* LocalSettings = UBaseFPSSettingsLocal::Get())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			Subsystem->ClearAllMappings();
				
			// registers input mapping context(s) associated with the mappable config
			Subsystem->AddPlayerMappableConfig(GetDefault<UBaseFPSSettings>()->GetDefaultInputConfig().LoadSynchronous());
	
			// registers custom key mappings [borrowed from LyraInputComponent::AddInputMappings]
			for (const TPair<FName, FKey>& Pair : LocalSettings->GetCustomPlayerInputConfig())
			{
				if (Pair.Key != NAME_None && Pair.Value.IsValid())
				{
					Subsystem->AddPlayerMappedKey(Pair.Key, Pair.Value);
				}
			}
		}	
	}
}

FText ABaseFPSPlayerController::GetNameTextForInteractActionKey(uint32 KeyIndex) const
{
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		TArray<FKey> Keys = Subsystem->QueryKeysMappedToAction(InteractAction);
		if (Keys.IsValidIndex(KeyIndex))
		{
			return Keys[KeyIndex].GetDisplayName();
		}
	}
	return FText::GetEmpty();
}

void ABaseFPSPlayerController::ToggleInGameMenu()
{
	if (ABaseFPSHUD* BaseFPSHUD = Cast<ABaseFPSHUD>(MyHUD))
	{
		BaseFPSHUD->ToggleInGameMenu();
	}
}

void ABaseFPSPlayerController::ShowScoreboard()
{
	if (ABaseFPSHUD* BaseFPSHUD = Cast<ABaseFPSHUD>(MyHUD))
	{
		BaseFPSHUD->ShowScoreboard();
	}
}

void ABaseFPSPlayerController::HideScoreboard()
{
	if (ABaseFPSHUD* BaseFPSHUD = Cast<ABaseFPSHUD>(MyHUD))
	{
		BaseFPSHUD->HideScoreboard();
	}
}

void ABaseFPSPlayerController::Move(const FInputActionValue& Value)
{
	if (MyCharacter && !IsMoveInputIgnored())
	{
		MyCharacter->Move(Value);
	}
}

void ABaseFPSPlayerController::Look(const FInputActionValue& Value)
{
	if (MyCharacter && !IsLookInputIgnored())
	{
		MyCharacter->Look(Value);
	}
}

void ABaseFPSPlayerController::OnJump()
{
	if (MyCharacter && !IsMoveInputIgnored())
	{
		MyCharacter->Jump();
	}
}

void ABaseFPSPlayerController::OnJumpReleased()
{
	if (MyCharacter)
	{
		MyCharacter->StopJumping();
	}
}

void ABaseFPSPlayerController::OnCrouch(bool bClientSimulation)
{
	if (MyCharacter && !IsMoveInputIgnored())
	{
		MyCharacter->Crouch(bClientSimulation);
	}
}

void ABaseFPSPlayerController::OnCrouchReleased(bool bClientSimulation)
{
	if (MyCharacter)
	{
		MyCharacter->UnCrouch(bClientSimulation);
	}
}

void ABaseFPSPlayerController::OnInteract()
{
	if (MyCharacter && !IsMoveInputIgnored())
	{
		MyCharacter->BeginInteract();
	}
}

void ABaseFPSPlayerController::OnInteractReleased()
{
	if (MyCharacter)
	{
		MyCharacter->EndInteract();
	}
}

void ABaseFPSPlayerController::OnFire()
{
	if (MyCharacter)
	{
		MyCharacter->StartFire();
	}
	else
	{
		ServerRestartPlayer();
	}
}

void ABaseFPSPlayerController::OnFireReleased()
{
	if (MyCharacter)
	{
		MyCharacter->StopFire();
	}
}

void ABaseFPSPlayerController::OnAltFire()
{
	if (MyCharacter && !IsMoveInputIgnored())
	{
		MyCharacter->StartAltFire();
	}
}

void ABaseFPSPlayerController::OnAltFireReleased()
{
	if (MyCharacter)
	{
		MyCharacter->StopAltFire();
	}
}

void ABaseFPSPlayerController::OnReload()
{
	if (MyCharacter && !IsMoveInputIgnored())
	{
		MyCharacter->StartReload();
	}
}

void ABaseFPSPlayerController::OnReloadReleased()
{
	if (MyCharacter)
	{
		MyCharacter->StopReload();
	}
}

void ABaseFPSPlayerController::OnNextWeapon()
{
	if (MyCharacter && !IsMoveInputIgnored())
	{
		MyCharacter->NextWeapon();
	}
}

void ABaseFPSPlayerController::OnPrevWeapon()
{
	if (MyCharacter && !IsMoveInputIgnored())
	{
		MyCharacter->PrevWeapon();
	}
}
