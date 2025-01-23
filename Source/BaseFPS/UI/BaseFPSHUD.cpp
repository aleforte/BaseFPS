// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BaseFPSHUD.h"

#include "InteractionCard.h"
#include "InventoryBar.h"
#include "AmmoCounter.h"
#include "BaseFPS.h"
#include "CommonUIExtensions.h"
#include "InventoryBarSlot.h"
#include "Character/BaseFPSCharacter.h"
#include "Components/Image.h"
#include "Player/BaseFPSPlayerController.h"
#include "UI/BaseFPSHUDLayout.h"
#include "Weapons/Weapon.h"

ABaseFPSHUD::ABaseFPSHUD()
{
	// Structure to hold one-time initialization
	struct FConstructorStatics
	{
		ConstructorHelpers::FClassFinder<UBaseFPSHUDLayout> HUDLayoutClass;
		ConstructorHelpers::FClassFinder<UCommonActivatableWidget> EscapeMenuClass;
		ConstructorHelpers::FClassFinder<UCommonActivatableWidget> ScoreboardClass;
		FConstructorStatics()
			: HUDLayoutClass(TEXT("/Game/UI/HUD/W_DefaultHUDLayout"))
			, EscapeMenuClass(TEXT("/Game/UI/HUD/W_InGameMenu"))
			, ScoreboardClass(TEXT("/Game/UI/Shared/Widgets/LoadingScreen/W_LoadingScreen"))
		{}
	};
	static FConstructorStatics ConstructorStatics;
	
	PawnHUDLayoutClass = ConstructorStatics.HUDLayoutClass.Class;
	EscapeMenuClass = ConstructorStatics.EscapeMenuClass.Class;
	ScoreboardClass = ConstructorStatics.ScoreboardClass.Class;
}

void ABaseFPSHUD::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (PlayerOwner)
	{
		PawnHUDLayout = Cast<UBaseFPSHUDLayout>(UCommonUIExtensions::PushContentToLayer_ForPlayer(PlayerOwner->GetLocalPlayer(), FrontendTags::TAG_UI_LAYER_GAME, PawnHUDLayoutClass));
		if (PawnHUDLayout)
		{
			PlayerOwner->OnPossessedPawnChanged.AddDynamic(this, &ThisClass::OnPawnChanged);
			PawnHUDLayout->SetVisibility(bShowHUD ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		}
	}
}

void ABaseFPSHUD::ShowHUD()
{
	Super::ShowHUD();
	if (PawnHUDLayout)
	{
		PawnHUDLayout->SetVisibility(bShowHUD ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);		
	}
}

void ABaseFPSHUD::Destroyed()
{
	if (PlayerOwner)
	{
		PlayerOwner->OnPossessedPawnChanged.RemoveAll(this);
		if (PawnHUDLayout)
		{
			UCommonUIExtensions::PopContentFromLayer(PawnHUDLayout);
			PawnHUDLayout = nullptr;
		}
	}
	Super::Destroyed();
}

void ABaseFPSHUD::OnPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	if (ABaseFPSCharacter* OldChar = Cast<ABaseFPSCharacter>(OldPawn))
	{
		UnbindFromCharacter(OldChar);
	}

	if (ABaseFPSCharacter* NewChar = Cast<ABaseFPSCharacter>(NewPawn))
	{
		BindToCharacter(NewChar);
	}
	
	// refresh Pawn HUD widgets
	OnInventoryUpdated();
	OnEquipNewWeapon();
	OnFocusChanged(PossessedCharacter ? PossessedCharacter->GetInteractableInFocus() : nullptr);

	if (PawnHUDLayout)
	{
		UImage* Crosshair = PawnHUDLayout->GetAimPoint();
		Crosshair->SetVisibility(PossessedCharacter ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}

void ABaseFPSHUD::BindToCharacter(TObjectPtr<ABaseFPSCharacter> InChar)
{
	if (ensureMsgf(InChar != nullptr, TEXT("Attempted to bind to invalid character")))
	{
		PossessedCharacter = InChar;
		OnInventoryUpdatedHandle = InChar->OnInventoryUpdated.AddUObject(this, &ThisClass::OnInventoryUpdated);
		OnEquippedNewWeaponHandle = InChar->OnEquippedNewWeapon.AddUObject(this, &ThisClass::OnEquipNewWeapon);
		OnFocusChangedHandle = InChar->OnFocusChanged.AddUObject(this, &ThisClass::OnFocusChanged);
		OnAmmoUpdatedHandle = InChar->OnAmmoUpdated.AddUObject(this, &ThisClass::OnAmmoUpdated);
	}
}

void ABaseFPSHUD::UnbindFromCharacter(TObjectPtr<ABaseFPSCharacter> InChar)
{
	if (ensureMsgf(InChar == PossessedCharacter, TEXT("Attempted to unbind from unpossessed character!")))
	{
		PossessedCharacter->OnInventoryUpdated.Remove(OnInventoryUpdatedHandle);
		PossessedCharacter->OnEquippedNewWeapon.Remove(OnEquippedNewWeaponHandle);
		PossessedCharacter->OnFocusChanged.Remove(OnFocusChangedHandle);
		PossessedCharacter->OnAmmoUpdated.Remove(OnAmmoUpdatedHandle);
		PossessedCharacter = nullptr;
	}
}

void ABaseFPSHUD::OnInventoryUpdated() const
{
	if (PawnHUDLayout)
	{
		TObjectPtr<UInventoryBar> InventoryBar = PawnHUDLayout->GetInventoryBar();
		if (PossessedCharacter)
		{
			InventoryBar->SetDisplayedInventory(PossessedCharacter->GetInventory());
			if (InventoryBar->ContainsItems())
			{
				InventoryBar->SetVisibility(ESlateVisibility::HitTestInvisible);
				InventoryBar->SetActiveItem(PossessedCharacter->GetEquippedWeapon());
			}
			else
			{
				InventoryBar->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
		else
		{
			InventoryBar->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void ABaseFPSHUD::OnEquipNewWeapon() const
{
	if (PawnHUDLayout)
	{
		TObjectPtr<UAmmoCounter> AmmoCounter = PawnHUDLayout->GetAmmoCounter();
		TObjectPtr<UInventoryBar> InventoryBar = PawnHUDLayout->GetInventoryBar();
		if (PossessedCharacter)
		{
			if (TObjectPtr<AWeapon> EquippedWeapon = PossessedCharacter->GetEquippedWeapon())
			{
				AmmoCounter->SetVisibility(ESlateVisibility::HitTestInvisible);
				AmmoCounter->SetDisplayedWeapon(EquippedWeapon);
				if (InventoryBar->ContainsItem(EquippedWeapon))
				{
					// If this is a new/pending pickup item, then OnInventoryUpdated() will refresh active card
					InventoryBar->SetActiveItem(EquippedWeapon);
				}
			}
			else
			{
				AmmoCounter->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
		else
		{
			AmmoCounter->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void ABaseFPSHUD::OnFocusChanged(UInteractableComponent* Interactable) const
{
	if (PawnHUDLayout && PlayerOwner)
	{
		TObjectPtr<UInteractionCard> InteractionCard = PawnHUDLayout->GetInteractionCard();
		if (PossessedCharacter)
		{
			InteractionCard->SetVisibility(ESlateVisibility::HitTestInvisible);
			if (Interactable)
			{
				FText InteractKeyText = Cast<ABaseFPSPlayerController>(PlayerOwner)->GetNameTextForInteractActionKey();
				InteractionCard->ShowPromptForInteractable(&InteractKeyText, Interactable);			
			}
			else
			{
				InteractionCard->SetVisibility(ESlateVisibility::Collapsed);	
			}
		}
		else
		{
			InteractionCard->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void ABaseFPSHUD::OnAmmoUpdated(AWeapon* Weapon) const
{
	if (PawnHUDLayout)
	{
		TObjectPtr<UAmmoCounter> AmmoCounter = PawnHUDLayout->GetAmmoCounter();
		TObjectPtr<UInventoryBar> InventoryBar = PawnHUDLayout->GetInventoryBar();
		if (TObjectPtr<UInventoryBarSlot> Slot = InventoryBar->GetInventoryBarSlot(Weapon))
		{
			int32 TotalAmmo = Weapon->GetCurrentAmmoInClip() + Weapon->GetCurrentReserveAmmo();
			Slot->SetAmmoCount(TotalAmmo);
		}
		if (PossessedCharacter)
		{
			if (PossessedCharacter->IsEquipped(Weapon))
			{
				AmmoCounter->SetCurrentAmmoInClipText(Weapon->GetCurrentAmmoInClip());
				AmmoCounter->SetCurrentAmmoReserveText(Weapon->GetCurrentReserveAmmo());
			}
		}
	}
}

void ABaseFPSHUD::ToggleInGameMenu()
{
	if (EscapeMenu)
	{
		UCommonUIExtensions::PopContentFromLayer(EscapeMenu);
		EscapeMenu = nullptr; // always clear cached widgets when removed from screen
	}
	else if (EscapeMenuClass && PlayerOwner)
	{
		EscapeMenu = UCommonUIExtensions::PushContentToLayer_ForPlayer(PlayerOwner->GetLocalPlayer(), FrontendTags::TAG_UI_LAYER_GAME_MENU, EscapeMenuClass);
	}
}

void ABaseFPSHUD::ShowScoreboard()
{
	if (ScoreboardClass && PlayerOwner)
	{
		Scoreboard = UCommonUIExtensions::PushContentToLayer_ForPlayer(PlayerOwner->GetLocalPlayer(), FrontendTags::TAG_UI_LAYER_GAME_MENU, ScoreboardClass);
	}
}

void ABaseFPSHUD::HideScoreboard()
{
	if (Scoreboard)
	{
		UCommonUIExtensions::PopContentFromLayer(Scoreboard);
		Scoreboard = nullptr; // always clear cached widgets when removed from screen
	}
}
