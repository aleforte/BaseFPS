// Fill out your copyright notice in the Description page of Project Settings.


#include "Settings/BaseFPSSettingRegistry.h"

#include "Settings/BaseFPSSettingsShared.h"
#include "Settings/BaseFPSSettingsLocal.h"
#include "GameSettingCollection.h"
#include "Player/BaseFPSLocalPlayer.h"

#define LOCTEXT_NAMESPACE "BaseFPS"

DEFINE_LOG_CATEGORY(LogBaseFPSSettingRegistry);

UBaseFPSSettingRegistry::UBaseFPSSettingRegistry()
{
}


/*static*/ UBaseFPSSettingRegistry* UBaseFPSSettingRegistry::Get(UBaseFPSLocalPlayer* InLocalPlayer)
{
	UBaseFPSSettingRegistry* Registry = FindObject<UBaseFPSSettingRegistry>(Cast<UObject>(InLocalPlayer), TEXT("BaseFPSSettingRegistry"), true);
	if (Registry)
	{
		Registry = NewObject<UBaseFPSSettingRegistry>(Cast<UObject>(InLocalPlayer), TEXT("BaseFPSSettingRegistry"));
		Registry->Initialize(Cast<ULocalPlayer>(InLocalPlayer));
	}

	return Registry;
}


void UBaseFPSSettingRegistry::OnInitialize(ULocalPlayer* InLocalPlayer)
{
	UBaseFPSLocalPlayer* XLocalPlayer = Cast<UBaseFPSLocalPlayer>(InLocalPlayer);

	VideoSettings = InitializeVideoSettings(XLocalPlayer);
	InitializeVideoSettings_FrameRates(VideoSettings, XLocalPlayer);
	RegisterSetting(VideoSettings);

	AudioSettings = InitializeAudioSettings(XLocalPlayer);
	RegisterSetting(AudioSettings);

	GameplaySettings = InitializeGameplaySettings(XLocalPlayer);
	RegisterSetting(GameplaySettings);

	MouseAndKeyboardSettings = InitializeMouseAndKeyboardSettings(XLocalPlayer);
	RegisterSetting(MouseAndKeyboardSettings);

	GamepadSettings = InitializeGamepadSettings(XLocalPlayer);
	RegisterSetting(GamepadSettings);
}


bool UBaseFPSSettingRegistry::IsFinishedInitializing() const
{
	if (Super::IsFinishedInitializing())
	{
		if (const UBaseFPSLocalPlayer* LocalPlayer = Cast<UBaseFPSLocalPlayer>(OwningLocalPlayer))
		{
			if (LocalPlayer->GetSharedSettings() == nullptr)
			{
				return false;
			}
		}
		return true;
	}
	return false;
}

void UBaseFPSSettingRegistry::SaveChanges()
{
	Super::SaveChanges();

	if (UBaseFPSLocalPlayer* LocalPlayer = Cast<UBaseFPSLocalPlayer>(OwningLocalPlayer))
	{
		LocalPlayer->GetLocalSettings()->ApplySettings(false);

		LocalPlayer->GetSharedSettings()->ApplySettings();
		LocalPlayer->GetSharedSettings()->SaveSettings();
	}
}

#undef LOCTEXT_NAMESPACE