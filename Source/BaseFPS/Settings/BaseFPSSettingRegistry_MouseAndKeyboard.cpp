// Fill out your copyright notice in the Description page of Project Settings.


#include "EnhancedActionKeyMapping.h"
#include "GameSettingCollection.h"
#include "GameSettingValueDiscreteDynamic.h"
#include "GameSettingValueScalarDynamic.h"
#include "PlayerMappableInputConfig.h"
#include "BaseFPSSettingRegistry.h"
#include "BaseFPSSettingsLocal.h"
#include "BaseFPSSettingsShared.h"
#include "CommonInputBaseTypes.h"

#include "Input/MappableConfigPair.h"
#include "Player/BaseFPSLocalPlayer.h"
#include "CustomSettings/BaseFPSSettingKeyboardInput.h"
#include "DataSource/GameSettingDataSourceDynamic.h"
#include "EditCondition/WhenCondition.h"

#define LOCTEXT_NAMESPACE "BaseFPS"

UGameSettingCollection* UBaseFPSSettingRegistry::InitializeMouseAndKeyboardSettings(UBaseFPSLocalPlayer* InLocalPlayer)
{
	UGameSettingCollection* Screen = NewObject<UGameSettingCollection>();
	Screen->SetDevName(TEXT("MouseAndKeyboardCollection"));
	Screen->SetDisplayName(LOCTEXT("MouseAndKeyboardCollection_Name", "Mouse & Keyboard"));
	Screen->Initialize(InLocalPlayer);

	const TSharedRef<FWhenCondition> WhenPlatformSupportsMouseAndKeyboard = MakeShared<FWhenCondition>(
	[](const ULocalPlayer*, FGameSettingEditableState& InOutEditState)
	{
		const UCommonInputPlatformSettings* PlatformInput = UPlatformSettingsManager::Get().GetSettingsForPlatform<UCommonInputPlatformSettings>();
		if (!PlatformInput->SupportsInputType(ECommonInputType::MouseAndKeyboard))
		{
			InOutEditState.Kill(TEXT("Platform does not support mouse and keyboard"));
		}
	});

	/************************************************************************/
	/* Mouse Sensitivity                                                    */
	/************************************************************************/
	{
		UGameSettingCollection* Sensitivity = NewObject<UGameSettingCollection>();
		Sensitivity->SetDevName(TEXT("MouseSensitivityCollection"));
		Sensitivity->SetDisplayName(LOCTEXT("MouseSensitivityCollection_Name", "Sensitivity"));
		Screen->AddSetting(Sensitivity);

		//----------------------------------------------------------------------------------
		{
			UGameSettingValueScalarDynamic* Setting = NewObject<UGameSettingValueScalarDynamic>();
			Setting->SetDevName(TEXT("MouseSensitivityYaw"));
			Setting->SetDisplayName(LOCTEXT("MouseSensitivityYaw_Name", "X-Axis Sensitivity"));
			Setting->SetDescriptionRichText(LOCTEXT("MouseSensitivityYaw_Description", "Sets the sensitivity of the mouse's horizontal (x) axis. With higher settings the camera will move faster when looking left and right with the mouse."));

			Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetMouseSensitivityX));
			Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetMouseSensitivityX));
			Setting->SetDefaultValue(GetDefault<UBaseFPSSettingsShared>()->GetMouseSensitivityX());
			Setting->SetDisplayFormat(UGameSettingValueScalarDynamic::RawTwoDecimals);
			Setting->SetSourceRangeAndStep(TRange<double>(0, 10), 0.05);
			Setting->SetMinimumLimit(0.00);

			Setting->AddEditCondition(WhenPlatformSupportsMouseAndKeyboard);

			Sensitivity->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		{
			UGameSettingValueScalarDynamic* Setting = NewObject<UGameSettingValueScalarDynamic>();
			Setting->SetDevName(TEXT("MouseSensitivityPitch"));
			Setting->SetDisplayName(LOCTEXT("MouseSensitivityPitch_Name", "Y-Axis Sensitivity"));
			Setting->SetDescriptionRichText(LOCTEXT("MouseSensitivityPitch_Description", "Sets the sensitivity of the mouse's vertical (y) axis. With higher settings the camera will move faster when looking up and down with the mouse."));

			Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetMouseSensitivityY));
			Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetMouseSensitivityY));
			Setting->SetDefaultValue(GetDefault<UBaseFPSSettingsShared>()->GetMouseSensitivityY());
			Setting->SetDisplayFormat(UGameSettingValueScalarDynamic::RawTwoDecimals);
			Setting->SetSourceRangeAndStep(TRange<double>(0, 10), 0.05);
			Setting->SetMinimumLimit(0.00);

			Setting->AddEditCondition(WhenPlatformSupportsMouseAndKeyboard);

			Sensitivity->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		{
			UGameSettingValueScalarDynamic* Setting = NewObject<UGameSettingValueScalarDynamic>();
			Setting->SetDevName(TEXT("MouseTargetingMultiplier"));
			Setting->SetDisplayName(LOCTEXT("MouseTargetingMultiplier_Name", "Targeting Sensitivity"));
			Setting->SetDescriptionRichText(LOCTEXT("MouseTargetingMultiplier_Description", "Sets the modifier for reducing mouse sensitivity when targeting. 100% will have no slow down when targeting. Lower settings will have more slow down when targeting."));

			Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetTargetingMultiplier));
			Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetTargetingMultiplier));
			Setting->SetDefaultValue(GetDefault<UBaseFPSSettingsShared>()->GetTargetingMultiplier());
			Setting->SetDisplayFormat(UGameSettingValueScalarDynamic::RawTwoDecimals);
			Setting->SetSourceRangeAndStep(TRange<double>(0, 10), 0.05);
			Setting->SetMinimumLimit(0.00);

			Setting->AddEditCondition(WhenPlatformSupportsMouseAndKeyboard);

			Sensitivity->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		{
			UGameSettingValueDiscreteDynamic_Bool* Setting = NewObject<UGameSettingValueDiscreteDynamic_Bool>();
			Setting->SetDevName(TEXT("InvertVerticalAxis"));
			Setting->SetDisplayName(LOCTEXT("InvertVerticalAxis_Name", "Invert Vertical Axis"));
			Setting->SetDescriptionRichText(LOCTEXT("InvertVerticalAxis_Description", "Enable the inversion of the vertical look axis."));

			Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetInvertVerticalAxis));
			Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetInvertVerticalAxis));
			Setting->SetDefaultValue(GetDefault<UBaseFPSSettingsShared>()->GetInvertVerticalAxis());

			Setting->AddEditCondition(WhenPlatformSupportsMouseAndKeyboard);

			Sensitivity->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		{
			UGameSettingValueDiscreteDynamic_Bool* Setting = NewObject<UGameSettingValueDiscreteDynamic_Bool>();
			Setting->SetDevName(TEXT("InvertHorizontalAxis"));
			Setting->SetDisplayName(LOCTEXT("InvertHorizontalAxis_Name", "Invert Horizontal Axis"));
			Setting->SetDescriptionRichText(LOCTEXT("InvertHorizontalAxis_Description", "Enable the inversion of the Horizontal look axis."));

			Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetInvertHorizontalAxis));
			Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetInvertHorizontalAxis));
			Setting->SetDefaultValue(GetDefault<UBaseFPSSettingsShared>()->GetInvertHorizontalAxis());

			Setting->AddEditCondition(WhenPlatformSupportsMouseAndKeyboard);

			Sensitivity->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
	}

	/************************************************************************/
	/* Bindings for Mouse & Keyboard - Automatically Generated              */
	/************************************************************************/
	{
		UGameSettingCollection* KeyBinding = NewObject<UGameSettingCollection>();
		KeyBinding->SetDevName(TEXT("KeyBindingCollection"));
		KeyBinding->SetDisplayName(LOCTEXT("KeyBindingCollection_Name", "Keyboard & Mouse"));
		Screen->AddSetting(KeyBinding);
		static TSet<FName> AddedSettings;
		AddedSettings.Reset();

		//----------------------------------------------------------------------------------
		{
			const TArray<FMappableConfigPair>& RegisteredConfigs = InLocalPlayer->GetLocalSettings()->GetAllRegisteredInputConfigs();	
			const TMap<FName, FKey>& CustomKeyMap = InLocalPlayer->GetLocalSettings()->GetCustomPlayerInputConfig();
			
			for (const FMappableConfigPair& InputConfigPair : RegisteredConfigs)
			{
				if (InputConfigPair.Type != ECommonInputType::MouseAndKeyboard)
				{
					continue;
				}
				
				TArray<FEnhancedActionKeyMapping> ConfigMappings = InputConfigPair.Config->GetPlayerMappableKeys();
				if (ConfigMappings.IsEmpty())
				{
					UE_LOG(LogBaseFPSSettingRegistry, Warning, TEXT("PlayerMappableInputConfig '%s' has no player mappable keys in it! Skipping it in the setting registry..."), *InputConfigPair.Config->GetConfigName().ToString());
					continue;
				}
				
				UGameSettingCollection* ConfigSettingCollection = NewObject<UGameSettingCollection>();
				ConfigSettingCollection->SetDevName(InputConfigPair.Config->GetConfigName());
				ConfigSettingCollection->SetDisplayName(InputConfigPair.Config->GetDisplayName());
				Screen->AddSetting(ConfigSettingCollection);
				
				// Add each player mappable key to the settings screen!
				for (FEnhancedActionKeyMapping& Mapping : ConfigMappings)
				{
					UBaseFPSSettingKeyboardInput* ExistingSetting = nullptr;

					// Make sure that we cannot add two settings with the same FName for saving purposes
					if (AddedSettings.Contains(Mapping.PlayerMappableOptions.Name))
					{
						UE_LOG(LogBaseFPSSettingRegistry, Warning, TEXT("A setting with the name '%s' from config '%s' has already been added! Please remove duplicate name."), *Mapping.PlayerMappableOptions.Name.ToString(), *InputConfigPair.Config->GetConfigName().ToString());
						continue;
					}
					
					for (UGameSetting* Setting : ConfigSettingCollection->GetChildSettings())
					{
						UBaseFPSSettingKeyboardInput* KeyboardSetting = Cast<UBaseFPSSettingKeyboardInput>(Setting);
						if (KeyboardSetting->GetSettingDisplayName().EqualToCaseIgnored(Mapping.PlayerMappableOptions.DisplayName))
						{
							ExistingSetting = KeyboardSetting;
							break;
						}
					}
					
					FEnhancedActionKeyMapping MappingSynthesized(Mapping);
					// If the player has bound a custom key to this action, then set it to that
					if (const FKey* PlayerBoundKey = CustomKeyMap.Find(Mapping.PlayerMappableOptions.Name))
					{
						MappingSynthesized.Key = *PlayerBoundKey;
					}

					if (MappingSynthesized.PlayerMappableOptions.Name != NAME_None && !MappingSynthesized.PlayerMappableOptions.DisplayName.IsEmpty())
					{
						// Create the settings widget and initialize it, adding it to this config's section
						UBaseFPSSettingKeyboardInput* InputBinding = ExistingSetting ? ExistingSetting : NewObject<UBaseFPSSettingKeyboardInput>();
						
						InputBinding->SetInputData(MappingSynthesized, InputConfigPair.Config, !ExistingSetting ? 0 : 1);
						InputBinding->AddEditCondition(WhenPlatformSupportsMouseAndKeyboard);
						
						if (!ExistingSetting)
						{
							ConfigSettingCollection->AddSetting(InputBinding);	
						}
						
						AddedSettings.Add(MappingSynthesized.PlayerMappableOptions.Name);
					}
					else
					{
						UE_LOG(LogBaseFPSSettingRegistry, Warning, TEXT("A setting with the name '%s' from config '%s' could not be added, one of its names is empty!"), *Mapping.PlayerMappableOptions.Name.ToString(), *InputConfigPair.Config->GetConfigName().ToString());
						ensure(false);
					}
				}
			}
		}
	}
	
	return Screen;
}

#undef LOCTEXT_NAMESPACE


