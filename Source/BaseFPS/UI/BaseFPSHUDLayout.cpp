// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BaseFPSHUDLayout.h"

UInventoryBar* UBaseFPSHUDLayout::GetInventoryBar() const
{
	return InventoryBar;
}

UAmmoCounter* UBaseFPSHUDLayout::GetAmmoCounter() const
{
	return AmmoCounter;
}

UInteractionCard* UBaseFPSHUDLayout::GetInteractionCard() const
{
	return InteractionCard;
}

UImage* UBaseFPSHUDLayout::GetAimPoint() const
{
	return AimPoint;
}
