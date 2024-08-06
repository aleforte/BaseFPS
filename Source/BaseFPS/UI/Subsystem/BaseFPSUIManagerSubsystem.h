// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameUIManagerSubsystem.h"
#include "BaseFPSUIManagerSubsystem.generated.h"

class UCommonActivatableWidget;
/**
 * 
 */
UCLASS()
class BASEFPS_API UBaseFPSUIManagerSubsystem : public UGameUIManagerSubsystem
{
	GENERATED_BODY()

public:
	UBaseFPSUIManagerSubsystem() {}

	//~Begin UGameUIManagerSystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void BeginDestroy() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	//~End UGameUIManagerSystem interface

private:	
	/** Resolves issue where ActivatableWidgets get stuck in "active" state on level transition */
	virtual void OnWorldBeginTearDown(UWorld* InWorld);	
};
