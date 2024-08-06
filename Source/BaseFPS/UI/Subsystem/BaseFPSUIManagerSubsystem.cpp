// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Subsystem/BaseFPSUIManagerSubsystem.h"

#include "GameUIPolicy.h"
#include "Settings/BaseFPSSettings.h"

void UBaseFPSUIManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	if (const TSubclassOf<UGameUIPolicy> PolicyClass = GetDefault<UBaseFPSSettings>()->GetDefaultUIPolicyClass().LoadSynchronous())
	{
		SwitchToPolicy(NewObject<UGameUIPolicy>(this, PolicyClass));
	}
	FWorldDelegates::OnWorldBeginTearDown.AddUObject(this, &UBaseFPSUIManagerSubsystem::OnWorldBeginTearDown);
}

void UBaseFPSUIManagerSubsystem::BeginDestroy()
{
	Super::BeginDestroy();
	FWorldDelegates::OnWorldBeginTearDown.RemoveAll(this);
}

bool UBaseFPSUIManagerSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return Super::ShouldCreateSubsystem(Outer);
}

void UBaseFPSUIManagerSubsystem::OnWorldBeginTearDown(UWorld* InWorld)
{
	// iterator over local players and reset their input
	for (TArray<ULocalPlayer*>::TConstIterator It = GetGameInstance()->GetLocalPlayerIterator(); It; ++It)
	{
		ULocalPlayer* LocalPlayer = *It;
		if (LocalPlayer->PlayerController)
		{
			LocalPlayer->PlayerController->SetInputMode(FInputModeGameOnly()); // reset input for controller	
		}
	}
}
