// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/BaseFPSLocalPlayer.h"

#include "Settings/BaseFPSSettings.h"
#include "Settings/BaseFPSSettingsLocal.h"
#include "Settings/BaseFPSSettingsShared.h"
#include "PlayerMappableInputConfig.h"

UBaseFPSLocalPlayer::UBaseFPSLocalPlayer()
{
}

void UBaseFPSLocalPlayer::PostInitProperties()
{
	Super::PostInitProperties();

	if (const UPlayerMappableInputConfig* InputConfig = GetDefault<UBaseFPSSettings>()->GetDefaultInputConfig().LoadSynchronous())
	{
		if (UBaseFPSSettingsLocal* LocalSettings = GetLocalSettings())
		{
			LocalSettings->RegisterInputConfig(ECommonInputType::MouseAndKeyboard, InputConfig, true);
		}
	}
}

/*static*/ UBaseFPSSettingsLocal* UBaseFPSLocalPlayer::GetLocalSettings()
{
	return UBaseFPSSettingsLocal::Get();
}

UBaseFPSSettingsShared* UBaseFPSLocalPlayer::GetSharedSettings() const
{
	if (!SharedSettings)
	{
		SharedSettings = UBaseFPSSettingsShared::LoadOrCreateSettings(this);
	}

	return SharedSettings;
}