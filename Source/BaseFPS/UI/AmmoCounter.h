// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "AmmoCounter.generated.h"

class UImage;
class UCommonTextBlock;
class AWeapon;
/**
 * 
 */
UCLASS(ClassGroup = UI)
class BASEFPS_API UAmmoCounter : public UCommonUserWidget
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> CurrentAmmoInClip;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> CurrentAmmoReserve;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UImage> AmmoIcon;

public:
	void SetDisplayedWeapon(const AWeapon* EquippedWeapon) const;
	void SetCurrentAmmoInClipText(int32 InAmount) const;
	void SetCurrentAmmoReserveText(int32 InAmount) const;
};
