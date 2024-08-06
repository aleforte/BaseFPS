// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonLocalPlayer.h"
#include "BaseFPSLocalPlayer.generated.h"

class UBaseFPSSettingsShared;
class UBaseFPSSettingsLocal;
/**
 * 
 */
UCLASS()
class BASEFPS_API UBaseFPSLocalPlayer : public UCommonLocalPlayer
{
	GENERATED_BODY()

public:
	UBaseFPSLocalPlayer();

private:
	UPROPERTY(Transient)
	mutable TObjectPtr<UBaseFPSSettingsShared> SharedSettings;

protected:
	//~UObject interface
	virtual void PostInitProperties() override;
	//~End of UObject interface

	/************************************************************************/
	/* Accessors                                                            */
	/************************************************************************/
public:
	UFUNCTION()
	static UBaseFPSSettingsLocal* GetLocalSettings();

	UFUNCTION()
	UBaseFPSSettingsShared* GetSharedSettings() const;
};
