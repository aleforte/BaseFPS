// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseFPSSettingRegistry.h"
#include "GameSettingCollection.h"
#include "GameSettingValueDiscreteDynamic.h"
#include "GameSetting.h"
#include "GameSettingAction.h"
#include "GameSettingValueScalarDynamic.h"
#include "NativeGameplayTags.h"
#include "BaseFPSSettingsLocal.h"
#include "BaseFPSSettingsShared.h"
#include "Performance/PerformanceSettings.h"
#include "GenericPlatform/GenericWindow.h"
#include "Player/BaseFPSLocalPlayer.h"
#include "CustomSettings/BaseFPSSettingValueDiscrete_OverallQuality.h"
#include "CustomSettings/BaseFPSSettingValueDiscrete_Resolution.h"
#include "CustomSettings/BaseFPSSettingValueScalarDynamic_SafeZoneValue.h"
#include "DataSource/GameSettingDataSourceDynamic.h"
#include "EditCondition/WhenCondition.h"
#include "EditCondition/WhenPlatformHasTrait.h"
#include "EditCondition/WhenPlayingAsPrimaryPlayer.h"
#include "Performance/PerformanceSettings.h"

#define LOCTEXT_NAMESPACE "BaseFPS"

UE_DEFINE_GAMEPLAY_TAG_STATIC(GameSettings_Action_EditSafeZone, "GameSettings.Action.EditSafeZone");
UE_DEFINE_GAMEPLAY_TAG_STATIC(GameSettings_Action_EditBrightness, "GameSettings.Action.EditBrightness");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Platform_Trait_SupportsWindowedMode, "Platform.Trait.SupportsWindowedMode");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Platform_Trait_NeedsBrightnessAdjustment, "Platform.Trait.NeedsBrightnessAdjustment");

//////////////////////////////////////////////////////////////////////

// Checks the platform-specific value for bSupportsGranularVideoQualitySettings
class FGameSettingEditCondition_VideoQuality : public FGameSettingEditCondition
{
public:
	FGameSettingEditCondition_VideoQuality(const FString& InDisableString)
		: DisableString(InDisableString)
	{
	}

	virtual void GatherEditState(const ULocalPlayer* InLocalPlayer, FGameSettingEditableState& InOutEditState) const override
	{
		// commented for now, not needed unless other Platforms are supported
		if (!UPlatformSpecificRenderingSettings::Get()->bSupportsGranularVideoQualitySettings)
		{
			InOutEditState.Kill(DisableString);
		}
	}

	virtual void SettingChanged(const ULocalPlayer* LocalPlayer, UGameSetting* Setting, EGameSettingChangeReason Reason) const override
	{
		// TODO for now this applies the setting immediately
		const UBaseFPSLocalPlayer* LC = CastChecked<UBaseFPSLocalPlayer>(LocalPlayer);
		LC->GetLocalSettings()->ApplyScalabilitySettings();
	}

private:
	FString DisableString;
};

//////////////////////////////////////////////////////////////////////


UGameSettingCollection* UBaseFPSSettingRegistry::InitializeVideoSettings(UBaseFPSLocalPlayer* InLocalPlayer)
{
	UGameSettingCollection* Screen = NewObject<UGameSettingCollection>();
	Screen->SetDevName(TEXT("VideoCollection"));
	Screen->SetDisplayName(LOCTEXT("VideoCollection_Name", "Video"));
	Screen->Initialize(InLocalPlayer);

	UGameSettingValueDiscreteDynamic_Enum* WindowModeSetting = nullptr;

	/************************************************************************/
	/* Display                                                              */
	/************************************************************************/
	{
		UGameSettingCollection* Display = NewObject<UGameSettingCollection>();
		Display->SetDevName("DisplayCollection");
		Display->SetDisplayName(LOCTEXT("DisplayCollection_Name", "Display"));
		Screen->AddSetting(Display);

		//----------------------------------------------------------------------------------
		{
			UGameSettingValueDiscreteDynamic_Enum* Setting = NewObject<UGameSettingValueDiscreteDynamic_Enum>();
			Setting->SetDevName(TEXT("WindowMode"));
			Setting->SetDisplayName(LOCTEXT("WindowMode_Name", "Window Mode"));
			Setting->SetDescriptionRichText(LOCTEXT("WindowMode_Description", "In Windowed mode you can interact with other windows more easily, and drag the edges of the window to set the size. In Windowed Fullscreen mode you can easily switch between applications. In Fullscreen mode you cannot interact with other windows as easily, but the game will run slightly faster."));

			Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetFullscreenMode));
			Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetFullscreenMode));
			Setting->AddEnumOption(EWindowMode::Fullscreen, LOCTEXT("WindowModeFullscreen", "Fullscreen"));
			Setting->AddEnumOption(EWindowMode::WindowedFullscreen, LOCTEXT("WindowModeWindowedFullscreen", "Windowed Fullscreen"));
			Setting->AddEnumOption(EWindowMode::Windowed, LOCTEXT("WindowModeWindowed", "Windowed"));

			Setting->AddEditCondition(FWhenPlatformHasTrait::KillIfMissing(TAG_Platform_Trait_SupportsWindowedMode, TEXT("Platform does not support window mode")));

			WindowModeSetting = Setting;

			Display->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		{
			UBaseFPSSettingValueDiscrete_Resolution* Setting = NewObject<UBaseFPSSettingValueDiscrete_Resolution>();
			Setting->SetDevName(TEXT("Resolution"));
			Setting->SetDisplayName(LOCTEXT("Resolution_Name", "Resolution"));
			Setting->SetDescriptionRichText(LOCTEXT("Resolution_Description", "Display Resolution determines the size of the window in Windowed mode. In Fullscreen mode, Display Resolution determines the graphics card output resolution, which can result in black bars depending on monitor and graphics card. Display Resolution is inactive in Windowed Fullscreen mode."));

			Setting->AddEditDependency(WindowModeSetting);
			Setting->AddEditCondition(FWhenPlatformHasTrait::KillIfMissing(TAG_Platform_Trait_SupportsWindowedMode, TEXT("Platform does not support window mode")));
			Setting->AddEditCondition(MakeShared<FWhenCondition>([WindowModeSetting](const ULocalPlayer*, FGameSettingEditableState& InOutEditState)
			{
				if (WindowModeSetting->GetValue<EWindowMode::Type>() == EWindowMode::WindowedFullscreen)
				{
					InOutEditState.Disable(LOCTEXT("ResolutionWindowedFullscreen_Disabled", "When the Window Mode is set to <strong>Windowed Fullscreen</>, the resolution must match the native desktop resolution."));
				}
			}));

			Display->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		{
			AddPerformanceStatPage(Display, InLocalPlayer);
		}
		//----------------------------------------------------------------------------------

		/************************************************************************/
		/* Graphics                                                             */
		/************************************************************************/
		{
			UGameSettingCollection* Graphics = NewObject<UGameSettingCollection>();
			Graphics->SetDevName(TEXT("GraphicsCollection"));
			Graphics->SetDisplayName(LOCTEXT("GraphicsCollection_Name", "Graphics"));
			Screen->AddSetting(Graphics);
			
			//----------------------------------------------------------------------------------
			{
				UGameSettingValueDiscreteDynamic_Enum* Setting = NewObject<UGameSettingValueDiscreteDynamic_Enum>();
				Setting->SetDevName(TEXT("ColorBlindMode"));
				Setting->SetDisplayName(LOCTEXT("ColorBlindMode_Name", "Color Blind Mode"));
				Setting->SetDescriptionRichText(LOCTEXT("ColorBlindMode_Description", "Using the provided images, test out the different color blind modes to find a color correction that works best for you."));
			
				Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetColorBlindMode));
				Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetColorBlindMode));
				Setting->SetDefaultValue(GetDefault<UBaseFPSSettingsShared>()->GetColorBlindMode());
				Setting->AddEnumOption(EColorBlindMode::Off, LOCTEXT("ColorBlindRotatorSettingOff", "Off"));
				Setting->AddEnumOption(EColorBlindMode::Deuteranope, LOCTEXT("ColorBlindRotatorSettingDeuteranope", "Deuteranope"));
				Setting->AddEnumOption(EColorBlindMode::Protanope, LOCTEXT("ColorBlindRotatorSettingProtanope", "Protanope"));
				Setting->AddEnumOption(EColorBlindMode::Tritanope, LOCTEXT("ColorBlindRotatorSettingTritanope", "Tritanope"));

				Setting->AddEditCondition(FWhenPlayingAsPrimaryPlayer::Get());

				Graphics->AddSetting(Setting);
			}
			//----------------------------------------------------------------------------------
			{
				UGameSettingValueDiscreteDynamic_Number* Setting = NewObject<UGameSettingValueDiscreteDynamic_Number>();
				Setting->SetDevName(TEXT("ColorBlindStrength"));
				Setting->SetDisplayName(LOCTEXT("ColorBlindStrength_Name", "Color Blind Strength"));
				Setting->SetDescriptionRichText(LOCTEXT("ColorBlindStrength_Description", "Using the provided images, test out the different strengths to find a color correction that works best for you."));

				Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetColorBlindStrength));
				Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetColorBlindStrength));
				Setting->SetDefaultValue(GetDefault<UBaseFPSSettingsShared>()->GetColorBlindStrength());
				for (int32 Index = 0; Index <= 10; Index++)
				{
					Setting->AddOption(Index, FText::AsNumber(Index));
				}

				Setting->AddEditCondition(FWhenPlayingAsPrimaryPlayer::Get());

				Graphics->AddSetting(Setting);
			}
			//----------------------------------------------------------------------------------
			{
				UGameSettingValueScalarDynamic* Setting = NewObject<UGameSettingValueScalarDynamic>();
				Setting->SetDevName(TEXT("Brightness"));
				Setting->SetDisplayName(LOCTEXT("Brightness_Name", "Brightness"));
				Setting->SetDescriptionRichText(LOCTEXT("Brightness_Description", "Adjusts the brightness."));

				Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetDisplayGamma));
				Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetDisplayGamma));
				Setting->SetDefaultValue(2.2);
				Setting->SetDisplayFormat([](double SourceValue, double NormalizedValue) {
					return FText::Format(LOCTEXT("BrightnessFormat", "{0}%"), (int32)FMath::GetMappedRangeValueClamped(FVector2D(0, 1), FVector2D(50, 150), NormalizedValue));
				});
				Setting->SetSourceRangeAndStep(TRange<double>(1.7, 2.7), 0.01);

				Setting->AddEditCondition(FWhenPlayingAsPrimaryPlayer::Get());
				Setting->AddEditCondition(FWhenPlatformHasTrait::KillIfMissing(TAG_Platform_Trait_NeedsBrightnessAdjustment, TEXT("Platform does not require brightness adjustment.")));

				Graphics->AddSetting(Setting);
			}
			//----------------------------------------------------------------------------------
			{
				UBaseFPSSettingAction_SafeZoneEditor* Setting = NewObject<UBaseFPSSettingAction_SafeZoneEditor>();
				Setting->SetDevName(TEXT("SafeZone"));
				Setting->SetDisplayName(LOCTEXT("SafeZone_Name", "Safe Zone"));
				Setting->SetDescriptionRichText(LOCTEXT("SafeZone_Description", "Set the UI safe zone for the platform."));
				Setting->SetActionText(LOCTEXT("SafeZone_Action", "Set Safe Zone"));
				Setting->SetNamedAction(GameSettings_Action_EditSafeZone);
			
				Setting->AddEditCondition(FWhenPlayingAsPrimaryPlayer::Get());
				Setting->AddEditCondition(MakeShared<FWhenCondition>([](const ULocalPlayer*, FGameSettingEditableState& InOutEditState) {
					FDisplayMetrics Metrics;
					FSlateApplication::Get().GetCachedDisplayMetrics(Metrics);
					if (Metrics.TitleSafePaddingSize.Size() == 0)
					{
						InOutEditState.Kill(TEXT("Platform does not have any TitleSafePaddingSize configured in the display metrics."));
					}
				}));
			
				Graphics->AddSetting(Setting);
			}
			//----------------------------------------------------------------------------------
		}
	}


	/************************************************************************/
	/* Graphics Quality                                                     */
	/************************************************************************/
	{
		UGameSettingCollection* GraphicsQuality = NewObject<UGameSettingCollection>();
		GraphicsQuality->SetDevName(TEXT("GraphicsQuality"));
		GraphicsQuality->SetDisplayName(LOCTEXT("GraphicsQuality_Name", "Graphics Quality"));
		Screen->AddSetting(GraphicsQuality);

		UGameSetting* AutoSetQuality = nullptr;
		UGameSetting* GraphicsQualityPresets = nullptr;

		//----------------------------------------------------------------------------------
		{
			UGameSettingAction* Setting = NewObject<UGameSettingAction>();
			Setting->SetDevName(TEXT("AutoSetQuality"));
			Setting->SetDisplayName(LOCTEXT("AutoSetQuality_Name", "Auto-Set Quality"));
			Setting->SetDescriptionRichText(LOCTEXT("AutoSetQuality_Description", "Automatically configure the graphics quality options based on a benchmark of the hardware."));

			Setting->SetDoesActionDirtySettings(true);
			Setting->SetActionText(LOCTEXT("AutoSetQuality_Action", "Auto-Set"));
			Setting->SetCustomAction([](ULocalPlayer* LocalPlayer)
			{
				const UBaseFPSLocalPlayer* LC = CastChecked<UBaseFPSLocalPlayer>(LocalPlayer);
				// We don't save state until users apply the settings.
				constexpr bool bImmediatelySaveState = false;
				LC->GetLocalSettings()->RunAutoBenchmark(bImmediatelySaveState);
			});

			Setting->AddEditCondition(MakeShared<FWhenCondition>([](const ULocalPlayer* LocalPlayer, FGameSettingEditableState& InOutEditState)
			{
				const UBaseFPSLocalPlayer* LC = CastChecked<UBaseFPSLocalPlayer>(LocalPlayer);
				const bool bCanBenchmark = LC->GetLocalSettings()->CanRunAutoBenchmark();

				if (!bCanBenchmark)
				{
					InOutEditState.Kill(TEXT("Auto quality not supported"));
				}
			}));

			GraphicsQuality->AddSetting(Setting);

			AutoSetQuality = Setting;
		}
		//----------------------------------------------------------------------------------
		{
			UBaseFPSSettingValueDiscrete_OverallQuality* Setting = NewObject<UBaseFPSSettingValueDiscrete_OverallQuality>();
			Setting->SetDevName(TEXT("GraphicsQualityPresets"));
			Setting->SetDisplayName(LOCTEXT("GraphicsQualityPresets_Name", "Quality Presets"));
			Setting->SetDescriptionRichText(LOCTEXT("GraphicsQualityPresets_Description", "Quality Preset allows you to adjust multiple video options at once. Try a few options to see what fits your preference and device's performance."));

			Setting->AddEditDependency(AutoSetQuality);
			GraphicsQuality->AddSetting(Setting);

			GraphicsQualityPresets = Setting;
		}
				//----------------------------------------------------------------------------------
		{
			UGameSettingValueScalarDynamic* Setting = NewObject<UGameSettingValueScalarDynamic>();
			Setting->SetDevName(TEXT("ResolutionScale"));
			Setting->SetDisplayName(LOCTEXT("ResolutionScale_Name", "3D Resolution"));
			Setting->SetDescriptionRichText(LOCTEXT("ResolutionScale_Description", "3D resolution determines the resolution that objects are rendered in game, but does not affect the main menu.  Lower resolutions can significantly increase frame rate."));

			Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetResolutionScaleNormalized));
			Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetResolutionScaleNormalized));
			Setting->SetDisplayFormat(UGameSettingValueScalarDynamic::ZeroToOnePercent);

			Setting->AddEditDependency(AutoSetQuality);
			Setting->AddEditDependency(GraphicsQualityPresets);
			Setting->AddEditCondition(MakeShared<FGameSettingEditCondition_VideoQuality>(TEXT("Platform does not support 3D Resolution")));

			// When this setting changes, it can GraphicsQualityPresets to be set to custom, or a particular preset.
			GraphicsQualityPresets->AddEditDependency(Setting);
			GraphicsQuality->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		{
			UGameSettingValueDiscreteDynamic_Number* Setting = NewObject<UGameSettingValueDiscreteDynamic_Number>();
			Setting->SetDevName(TEXT("GlobalIlluminationQuality"));
			Setting->SetDisplayName(LOCTEXT("GlobalIlluminationQuality_Name", "Global Illumination"));
			Setting->SetDescriptionRichText(LOCTEXT("GlobalIlluminationQuality_Description", "Global Illumination controls the quality of dynamically calculated indirect lighting bounces, sky shadowing and Ambient Occlusion. Settings of 'High' and above use more accurate ray tracing methods to solve lighting, but can reduce performance."));

			Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetGlobalIlluminationQuality));
			Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetGlobalIlluminationQuality));
			Setting->AddOption(0, LOCTEXT("VisualEffectQualityLow", "Low"));
			Setting->AddOption(1, LOCTEXT("VisualEffectQualityMedium", "Medium"));
			Setting->AddOption(2, LOCTEXT("VisualEffectQualityHigh", "High"));
			Setting->AddOption(3, LOCTEXT("VisualEffectQualityEpic", "Epic"));

			Setting->AddEditDependency(AutoSetQuality);
			Setting->AddEditDependency(GraphicsQualityPresets);
			Setting->AddEditCondition(MakeShared<FGameSettingEditCondition_VideoQuality>(TEXT("Platform does not support GlobalIlluminationQuality")));

			// When this setting changes, it can GraphicsQualityPresets to be set to custom, or a particular preset.
			GraphicsQualityPresets->AddEditDependency(Setting);

			GraphicsQuality->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		{
			UGameSettingValueDiscreteDynamic_Number* Setting = NewObject<UGameSettingValueDiscreteDynamic_Number>();
			Setting->SetDevName(TEXT("Shadows"));
			Setting->SetDisplayName(LOCTEXT("Shadows_Name", "Shadows"));
			Setting->SetDescriptionRichText(LOCTEXT("Shadows_Description", "Shadow quality determines the resolution and view distance of dynamic shadows. Shadows improve visual quality and give better depth perception, but can reduce performance."));

			Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetShadowQuality));
			Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetShadowQuality));
			Setting->AddOption(0, LOCTEXT("ShadowLow", "Off"));
			Setting->AddOption(1, LOCTEXT("ShadowMedium", "Medium"));
			Setting->AddOption(2, LOCTEXT("ShadowHigh", "High"));
			Setting->AddOption(3, LOCTEXT("ShadowEpic", "Epic"));

			Setting->AddEditDependency(AutoSetQuality);
			Setting->AddEditDependency(GraphicsQualityPresets);
			Setting->AddEditCondition(MakeShared<FGameSettingEditCondition_VideoQuality>(TEXT("Platform does not support Shadows")));

			// When this setting changes, it can GraphicsQualityPresets to be set to custom, or a particular preset.
			GraphicsQualityPresets->AddEditDependency(Setting);

			GraphicsQuality->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		{
			UGameSettingValueDiscreteDynamic_Number* Setting = NewObject<UGameSettingValueDiscreteDynamic_Number>();
			Setting->SetDevName(TEXT("AntiAliasing"));
			Setting->SetDisplayName(LOCTEXT("AntiAliasing_Name", "Anti-Aliasing"));
			Setting->SetDescriptionRichText(LOCTEXT("AntiAliasing_Description", "Anti-Aliasing reduces jaggy artifacts along geometry edges. Increasing this setting will make edges look smoother, but can reduce performance. Higher settings mean more anti-aliasing."));

			Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetAntiAliasingQuality));
			Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetAntiAliasingQuality));
			Setting->AddOption(0, LOCTEXT("AntiAliasingLow", "Off"));
			Setting->AddOption(1, LOCTEXT("AntiAliasingMedium", "Medium"));
			Setting->AddOption(2, LOCTEXT("AntiAliasingHigh", "High"));
			Setting->AddOption(3, LOCTEXT("AntiAliasingEpic", "Epic"));

			Setting->AddEditDependency(AutoSetQuality);
			Setting->AddEditDependency(GraphicsQualityPresets);
			Setting->AddEditCondition(MakeShared<FGameSettingEditCondition_VideoQuality>(TEXT("Platform does not support Anti-Aliasing")));

			// When this setting changes, it can GraphicsQualityPresets to be set to custom, or a particular preset.
			GraphicsQualityPresets->AddEditDependency(Setting);

			GraphicsQuality->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		{
			UGameSettingValueDiscreteDynamic_Number* Setting = NewObject<UGameSettingValueDiscreteDynamic_Number>();
			Setting->SetDevName(TEXT("ViewDistance"));
			Setting->SetDisplayName(LOCTEXT("ViewDistance_Name", "View Distance"));
			Setting->SetDescriptionRichText(LOCTEXT("ViewDistance_Description", "View distance determines how far away objects are culled for performance."));

			Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetViewDistanceQuality));
			Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetViewDistanceQuality));
			Setting->AddOption(0, LOCTEXT("ViewDistanceNear", "Near"));
			Setting->AddOption(1, LOCTEXT("ViewDistanceMedium", "Medium"));
			Setting->AddOption(2, LOCTEXT("ViewDistanceFar", "Far"));
			Setting->AddOption(3, LOCTEXT("ViewDistanceEpic", "Epic"));

			Setting->AddEditDependency(AutoSetQuality);
			Setting->AddEditDependency(GraphicsQualityPresets);
			Setting->AddEditCondition(MakeShared<FGameSettingEditCondition_VideoQuality>(TEXT("Platform does not support View Distance")));

			// When this setting changes, it can GraphicsQualityPresets to be set to custom, or a particular preset.
			GraphicsQualityPresets->AddEditDependency(Setting);

			GraphicsQuality->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		{
			UGameSettingValueDiscreteDynamic_Number* Setting = NewObject<UGameSettingValueDiscreteDynamic_Number>();
			Setting->SetDevName(TEXT("TextureQuality"));
			Setting->SetDisplayName(LOCTEXT("TextureQuality_Name", "Textures"));

			Setting->SetDescriptionRichText(LOCTEXT("TextureQuality_Description", "Texture quality determines the resolution of textures in game. Increasing this setting will make objects more detailed, but can reduce performance."));

			Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetTextureQuality));
			Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetTextureQuality));
			Setting->AddOption(0, LOCTEXT("TextureQualityLow", "Low"));
			Setting->AddOption(1, LOCTEXT("TextureQualityMedium", "Medium"));
			Setting->AddOption(2, LOCTEXT("TextureQualityHigh", "High"));
			Setting->AddOption(3, LOCTEXT("TextureQualityEpic", "Epic"));

			Setting->AddEditDependency(AutoSetQuality);
			Setting->AddEditDependency(GraphicsQualityPresets);
			Setting->AddEditCondition(MakeShared<FGameSettingEditCondition_VideoQuality>(TEXT("Platform does not support Texture quality")));

			// When this setting changes, it can GraphicsQualityPresets to be set to custom, or a particular preset.
			GraphicsQualityPresets->AddEditDependency(Setting);

			GraphicsQuality->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		{
			UGameSettingValueDiscreteDynamic_Number* Setting = NewObject<UGameSettingValueDiscreteDynamic_Number>();
			Setting->SetDevName(TEXT("VisualEffectQuality"));
			Setting->SetDisplayName(LOCTEXT("VisualEffectQuality_Name", "Effects"));
			Setting->SetDescriptionRichText(LOCTEXT("VisualEffectQuality_Description", "Effects determines the quality of visual effects and lighting in game. Increasing this setting will increase the quality of visual effects, but can reduce performance."));

			Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetVisualEffectQuality));
			Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetVisualEffectQuality));
			Setting->AddOption(0, LOCTEXT("VisualEffectQualityLow", "Low"));
			Setting->AddOption(1, LOCTEXT("VisualEffectQualityMedium", "Medium"));
			Setting->AddOption(2, LOCTEXT("VisualEffectQualityHigh", "High"));
			Setting->AddOption(3, LOCTEXT("VisualEffectQualityEpic", "Epic"));

			Setting->AddEditDependency(AutoSetQuality);
			Setting->AddEditDependency(GraphicsQualityPresets);
			Setting->AddEditCondition(MakeShared<FGameSettingEditCondition_VideoQuality>(TEXT("Platform does not support VisualEffectQuality")));

			// When this setting changes, it can GraphicsQualityPresets to be set to custom, or a particular preset.
			GraphicsQualityPresets->AddEditDependency(Setting);

			GraphicsQuality->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		{
			UGameSettingValueDiscreteDynamic_Number* Setting = NewObject<UGameSettingValueDiscreteDynamic_Number>();
			Setting->SetDevName(TEXT("ReflectionQuality"));
			Setting->SetDisplayName(LOCTEXT("ReflectionQuality_Name", "Reflections"));
			Setting->SetDescriptionRichText(LOCTEXT("ReflectionQuality_Description", "Reflection quality determines the resolution and accuracy of reflections.  Settings of 'High' and above use more accurate ray tracing methods to solve reflections, but can reduce performance."));

			Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetReflectionQuality));
			Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetReflectionQuality));
			Setting->AddOption(0, LOCTEXT("VisualEffectQualityLow", "Low"));
			Setting->AddOption(1, LOCTEXT("VisualEffectQualityMedium", "Medium"));
			Setting->AddOption(2, LOCTEXT("VisualEffectQualityHigh", "High"));
			Setting->AddOption(3, LOCTEXT("VisualEffectQualityEpic", "Epic"));

			Setting->AddEditDependency(AutoSetQuality);
			Setting->AddEditDependency(GraphicsQualityPresets);
			Setting->AddEditCondition(MakeShared<FGameSettingEditCondition_VideoQuality>(TEXT("Platform does not support ReflectionQuality")));

			// When this setting changes, it can GraphicsQualityPresets to be set to custom, or a particular preset.
			GraphicsQualityPresets->AddEditDependency(Setting);

			GraphicsQuality->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		{
			UGameSettingValueDiscreteDynamic_Number* Setting = NewObject<UGameSettingValueDiscreteDynamic_Number>();
			Setting->SetDevName(TEXT("PostProcessingQuality"));
			Setting->SetDisplayName(LOCTEXT("PostProcessingQuality_Name", "Post Processing"));
			Setting->SetDescriptionRichText(LOCTEXT("PostProcessingQuality_Description", "Post Processing effects include Motion Blur, Depth of Field and Bloom. Increasing this setting improves the quality of post process effects, but can reduce performance."));  

			Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetPostProcessingQuality));
			Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetPostProcessingQuality));
			Setting->AddOption(0, LOCTEXT("PostProcessingQualityLow", "Low"));
			Setting->AddOption(1, LOCTEXT("PostProcessingQualityMedium", "Medium"));
			Setting->AddOption(2, LOCTEXT("PostProcessingQualityHigh", "High"));
			Setting->AddOption(3, LOCTEXT("PostProcessingQualityEpic", "Epic"));

			Setting->AddEditDependency(AutoSetQuality);
			Setting->AddEditDependency(GraphicsQualityPresets);
			Setting->AddEditCondition(MakeShared<FGameSettingEditCondition_VideoQuality>(TEXT("Platform does not support PostProcessingQuality")));

			// When this setting changes, it can GraphicsQualityPresets to be set to custom, or a particular preset.
			GraphicsQualityPresets->AddEditDependency(Setting);

			GraphicsQuality->AddSetting(Setting);
		}
	}

	/************************************************************************/
	/* Advanced Graphics                                                    */
	/************************************************************************/
	{
		UGameSettingCollection* AdvancedGraphics = NewObject<UGameSettingCollection>();
		AdvancedGraphics->SetDevName(TEXT("AdvancedGraphics"));
		AdvancedGraphics->SetDisplayName(LOCTEXT("AdvancedGraphics_Name", "Advanced Graphics"));
		Screen->AddSetting(AdvancedGraphics);

		//----------------------------------------------------------------------------------
		{
			UGameSettingValueDiscreteDynamic_Bool* Setting = NewObject<UGameSettingValueDiscreteDynamic_Bool>();
			Setting->SetDevName(TEXT("VerticalSync"));
			Setting->SetDisplayName(LOCTEXT("VerticalSync_Name", "Vertical Sync"));
			Setting->SetDescriptionRichText(LOCTEXT("VerticalSync_Description", "Enabling Vertical Sync eliminates screen tearing by always rendering and presenting a full frame. Disabling Vertical Sync can give higher frame rate and better input response, but can result in horizontal screen tearing."));

			Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(IsVSyncEnabled));
			Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetVSyncEnabled));
			Setting->SetDefaultValue(false);

			Setting->AddEditDependency(WindowModeSetting);
			Setting->AddEditCondition(MakeShared<FWhenCondition>([WindowModeSetting](const ULocalPlayer*, FGameSettingEditableState& InOutEditState) {
				if (WindowModeSetting->GetValue<EWindowMode::Type>() != EWindowMode::Fullscreen)
				{
					InOutEditState.Disable(LOCTEXT("FullscreenNeededForVSync", "This feature only works if 'Window Mode' is set to 'Fullscreen'."));
				}
			}));

			AdvancedGraphics->AddSetting(Setting);
		}
	}
	
	return Screen;
}

void AddFrameRateOptions(UGameSettingValueDiscreteDynamic_Number* Setting)
{
	const FText FPSFormat = LOCTEXT("FPSFormat", "{0} FPS");
	for (int32 Rate : GetDefault<UPerformanceSettings>()->DesktopFrameRateLimits)
	{
		Setting->AddOption((float)Rate, FText::Format(FPSFormat, Rate));
	}
	Setting->AddOption(0.0f, LOCTEXT("UnlimitedFPS", "Unlimited"));
}

void UBaseFPSSettingRegistry::InitializeVideoSettings_FrameRates(UGameSettingCollection* Screen, UBaseFPSLocalPlayer* InLocalPlayer)
{
		//----------------------------------------------------------------------------------
	{
		UGameSettingValueDiscreteDynamic_Number* Setting = NewObject<UGameSettingValueDiscreteDynamic_Number>();
		Setting->SetDevName(TEXT("FrameRateLimit_OnBattery"));
		Setting->SetDisplayName(LOCTEXT("FrameRateLimit_OnBattery_Name", "Frame Rate Limit (On Battery)"));
		Setting->SetDescriptionRichText(LOCTEXT("FrameRateLimit_OnBattery_Description", "Frame rate limit when running on battery. Set this lower for a more consistent frame rate or higher for the best experience on faster machines. You may need to disable Vsync to reach high frame rates."));

		Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetFrameRateLimit_OnBattery));
		Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetFrameRateLimit_OnBattery));
		Setting->SetDefaultValue(GetDefault<UBaseFPSSettingsLocal>()->GetFrameRateLimit_OnBattery());

		AddFrameRateOptions(Setting);

		Screen->AddSetting(Setting);
	}
	//----------------------------------------------------------------------------------
	{
		UGameSettingValueDiscreteDynamic_Number* Setting = NewObject<UGameSettingValueDiscreteDynamic_Number>();
		Setting->SetDevName(TEXT("FrameRateLimit_InMenu"));
		Setting->SetDisplayName(LOCTEXT("FrameRateLimit_InMenu_Name", "Frame Rate Limit (Menu)"));
		Setting->SetDescriptionRichText(LOCTEXT("FrameRateLimit_InMenu_Description", "Frame rate limit when in the menu. Set this lower for a more consistent frame rate or higher for the best experience on faster machines. You may need to disable Vsync to reach high frame rates."));

		Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetFrameRateLimit_InMenu));
		Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetFrameRateLimit_InMenu));
		Setting->SetDefaultValue(GetDefault<UBaseFPSSettingsLocal>()->GetFrameRateLimit_InMenu());

		AddFrameRateOptions(Setting);

		Screen->AddSetting(Setting);
	}
	//----------------------------------------------------------------------------------
	{
		UGameSettingValueDiscreteDynamic_Number* Setting = NewObject<UGameSettingValueDiscreteDynamic_Number>();
		Setting->SetDevName(TEXT("FrameRateLimit_WhenBackgrounded"));
		Setting->SetDisplayName(LOCTEXT("FrameRateLimit_WhenBackgrounded_Name", "Frame Rate Limit (Background)"));
		Setting->SetDescriptionRichText(LOCTEXT("FrameRateLimit_WhenBackgrounded_Description", "Frame rate limit when in the background. Set this lower for a more consistent frame rate or higher for the best experience on faster machines. You may need to disable Vsync to reach high frame rates."));

		Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetFrameRateLimit_WhenBackgrounded));
		Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetFrameRateLimit_WhenBackgrounded));
		Setting->SetDefaultValue(GetDefault<UBaseFPSSettingsLocal>()->GetFrameRateLimit_WhenBackgrounded());

		AddFrameRateOptions(Setting);

		Screen->AddSetting(Setting);
	}
	//----------------------------------------------------------------------------------
	{
		UGameSettingValueDiscreteDynamic_Number* Setting = NewObject<UGameSettingValueDiscreteDynamic_Number>();
		Setting->SetDevName(TEXT("FrameRateLimit_Always"));
		Setting->SetDisplayName(LOCTEXT("FrameRateLimit_Always_Name", "Frame Rate Limit"));
		Setting->SetDescriptionRichText(LOCTEXT("FrameRateLimit_Always_Description", "Frame rate limit sets the highest frame rate that is allowed. Set this lower for a more consistent frame rate or higher for the best experience on faster machines. You may need to disable Vsync to reach high frame rates."));

		Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetFrameRateLimit_Always));
		Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetFrameRateLimit_Always));
		Setting->SetDefaultValue(GetDefault<UBaseFPSSettingsLocal>()->GetFrameRateLimit_Always());

		AddFrameRateOptions(Setting);

		Screen->AddSetting(Setting);
	}
}

#undef LOCTEXT_NAMESPACE


