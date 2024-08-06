// Fill out your copyright notice in the Description page of Project Settings.


#include "Settings/BaseFPSSettingsLocal.h"

#include "AudioModulationStatics.h"
#include "CommonInputSubsystem.h"
#include "SoundControlBus.h"
#include "Sound/SoundClass.h"
#include "CommonInputTypeEnum.h"
#include "CommonUISettings.h"
#include "EnhancedActionKeyMapping.h"
#include "EnhancedInputSubsystems.h"
#include "ICommonUIModule.h"
#include "NativeGameplayTags.h"
#include "PlayerMappableInputConfig.h"
#include "SoundControlBusMix.h"
#include "Audio/AudioMixEffectsSubsystem.h"
#include "Audio/BaseFPSAudioSettings.h"
#include "Player/BaseFPSLocalPlayer.h"
#include "DeviceProfiles/DeviceProfileManager.h"
#include "Input/MappableConfigPair.h"
#include "Performance/PerformanceSettings.h"
#include "Widgets/Layout/SSafeZone.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(BaseFPSSettingsLocal)

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Platform_Trait_BinauralSettingControlledByOS, "Platform.Trait.BinauralSettingControlledByOS");

//////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
static TAutoConsoleVariable<bool> CVarApplyFrameRateSettingsInPIE(TEXT("BaseFPS.Settings.ApplyFrameRateSettingsInPIE"),
	false,
	TEXT("Should we apply frame rate settings in PIE?"),
	ECVF_Default);

static TAutoConsoleVariable<bool> CVarApplyFrontEndPerformanceOptionsInPIE(TEXT("BaseFPS.Settings.ApplyFrontEndPerformanceOptionsInPIE"),
	false,
	TEXT("Do we apply front-end specific performance options in PIE?"),
	ECVF_Default);

static TAutoConsoleVariable<bool> CVarApplyDeviceProfilesInPIE(TEXT("BaseFPS.Settings.ApplyDeviceProfilesInPIE"),
	false,
	TEXT("Should we apply experience/platform emulated device profiles in PIE?"),
	ECVF_Default);
#endif

//////////////////////////////////////////////////////////////////////

namespace BaseFPSSettingsHelpers
{
	bool HasPlatformTrait(FGameplayTag Tag)
	{
		return ICommonUIModule::GetSettings().GetPlatformTraits().HasTag(Tag);
	}
}

UBaseFPSSettingsLocal::UBaseFPSSettingsLocal()
{
	if (!HasAnyFlags(RF_ClassDefaultObject) && FSlateApplication::IsInitialized())
	{
		OnApplicationActivationStateChangedHandle = FSlateApplication::Get().OnApplicationActivationStateChanged().AddUObject(this, &ThisClass::OnAppActivationStateChanged);
	}

	SetToDefaults();
}

UBaseFPSSettingsLocal* UBaseFPSSettingsLocal::Get()
{
	return GEngine ? CastChecked<UBaseFPSSettingsLocal>(GEngine->GetGameUserSettings()) : nullptr;
}

void UBaseFPSSettingsLocal::BeginDestroy()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().OnApplicationActivationStateChanged().Remove(OnApplicationActivationStateChangedHandle);
	}

	Super::BeginDestroy();
}

void UBaseFPSSettingsLocal::SetToDefaults()
{
	Super::SetToDefaults();

	bUseHeadphoneMode = false;
	bUseHDRAudioMode = false;
	bSoundControlBusMixLoaded = false;
	
	FrameRateLimit_InMenu = 144.0f;
	FrameRateLimit_WhenBackgrounded = 30.0f;
	FrameRateLimit_OnBattery = 60.0f;
}

void UBaseFPSSettingsLocal::LoadSettings(bool bForceReload)
{
	Super::LoadSettings(bForceReload);
	
	// Console platforms use rhi.SyncInterval to limit framerate
	const UPlatformSpecificRenderingSettings* PlatformSettings = UPlatformSpecificRenderingSettings::Get();
	if (PlatformSettings->FramePacingMode == EFramePacingMode::ConsoleStyle)
	{
		FrameRateLimit = 0.0f;
	}
}

void UBaseFPSSettingsLocal::ConfirmVideoMode()
{
	Super::ConfirmVideoMode();
}

// Combines two limits, always taking the minimum of the two (with special handling for values of <= 0 meaning unlimited)
float CombineFrameRateLimits(float Limit1, float Limit2)
{
	if (Limit1 <= 0.0f)
	{
		return Limit2;
	}
	else if (Limit2 <= 0.0f)
	{
		return Limit1;
	}
	else
	{
		return FMath::Min(Limit1, Limit2);
	}
}

float UBaseFPSSettingsLocal::GetEffectiveFrameRateLimit()
{
	const UPlatformSpecificRenderingSettings* PlatformSettings = UPlatformSpecificRenderingSettings::Get();

#if WITH_EDITOR
	if (GIsEditor && !CVarApplyFrameRateSettingsInPIE.GetValueOnGameThread())
	{
		return Super::GetEffectiveFrameRateLimit();
	}
#endif

	if (PlatformSettings->FramePacingMode == EFramePacingMode::ConsoleStyle)
	{
		return 0.0f;
	}
	
	float EffectiveFrameRateLimit = Super::GetEffectiveFrameRateLimit();


	if (PlatformSettings->FramePacingMode == EFramePacingMode::DesktopStyle)
	{
		if (FPlatformMisc::IsRunningOnBattery())
		{
			EffectiveFrameRateLimit = CombineFrameRateLimits(EffectiveFrameRateLimit, FrameRateLimit_OnBattery);
		}

		if (FSlateApplication::IsInitialized() && !FSlateApplication::Get().IsActive())
		{
			EffectiveFrameRateLimit = CombineFrameRateLimits(EffectiveFrameRateLimit, FrameRateLimit_WhenBackgrounded);
		}
	}

	return EffectiveFrameRateLimit;
}

void UBaseFPSSettingsLocal::ResetToCurrentSettings()
{
	Super::ResetToCurrentSettings();
}

void UBaseFPSSettingsLocal::ApplyNonResolutionSettings()
{
	Super::ApplyNonResolutionSettings();
	
	// Check if Control Bus Mix references have been loaded,
	// Might be false if applying non resolution settings without touching any of the setters from UI
	if (!bSoundControlBusMixLoaded)
	{
		LoadUserControlBusMix();
	}
	
	// In this section, update each Control Bus to the currently cached UI settings
	{
		if (TObjectPtr<USoundControlBus>* ControlBusDblPtr = ControlBusMap.Find(TEXT("Overall")))
		{
			if (USoundControlBus* ControlBusPtr = *ControlBusDblPtr)
			{
				SetVolumeForControlBus(ControlBusPtr, OverallVolume);
			}
		}

		if (TObjectPtr<USoundControlBus>* ControlBusDblPtr = ControlBusMap.Find(TEXT("Music")))
		{
			if (USoundControlBus* ControlBusPtr = *ControlBusDblPtr)
			{
				SetVolumeForControlBus(ControlBusPtr, MusicVolume);
			}
		}

		if (TObjectPtr<USoundControlBus>* ControlBusDblPtr = ControlBusMap.Find(TEXT("SoundFX")))
		{
			if (USoundControlBus* ControlBusPtr = *ControlBusDblPtr)
			{
				SetVolumeForControlBus(ControlBusPtr, SoundFXVolume);
			}
		}

		if (TObjectPtr<USoundControlBus>* ControlBusDblPtr = ControlBusMap.Find(TEXT("Dialogue")))
		{
			if (USoundControlBus* ControlBusPtr = *ControlBusDblPtr)
			{
				SetVolumeForControlBus(ControlBusPtr, DialogueVolume);
			}
		}

		if (TObjectPtr<USoundControlBus>* ControlBusDblPtr = ControlBusMap.Find(TEXT("VoiceChat")))
		{
			if (USoundControlBus* ControlBusPtr = *ControlBusDblPtr)
			{
				SetVolumeForControlBus(ControlBusPtr, VoiceChatVolume);
			}
		}
	}

	if (UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(GetTypedOuter<ULocalPlayer>()))
	{
		InputSubsystem->SetGamepadInputType(ControllerPlatform);
	}

	if (bUseHeadphoneMode != bDesiredHeadphoneMode)
	{
		SetHeadphoneModeEnabled(bDesiredHeadphoneMode);
	}

	if (FApp::CanEverRender())
	{
		ApplyDisplayGamma();
		ApplySafeZoneScale();
		UpdateGameModeDeviceProfileAndFps();
	}

	PerfStatSettingsChangedEvent.Broadcast();
}

int32 UBaseFPSSettingsLocal::GetOverallScalabilityLevel() const
{
	return Super::GetOverallScalabilityLevel();
}

void UBaseFPSSettingsLocal::SetOverallScalabilityLevel(int32 Value)
{
	Super::SetOverallScalabilityLevel(Value);
}

void UBaseFPSSettingsLocal::SetShouldUseFrontendPerformanceSettings(bool bInFrontEnd)
{
	bInFrontEndForPerformancePurposes = bInFrontEnd;
	UpdateEffectiveFrameRateLimit();
}

bool UBaseFPSSettingsLocal::ShouldUseFrontendPerformanceSettings() const
{
#if WITH_EDITOR
	if (GIsEditor && !CVarApplyFrontEndPerformanceOptionsInPIE.GetValueOnGameThread())
	{
		return false;
	}
#endif

	return bInFrontEndForPerformancePurposes;
}

EStatDisplayMode UBaseFPSSettingsLocal::GetPerfStatDisplayState(EDisplayablePerformanceStat Stat) const
{
	if (const EStatDisplayMode* pMode = DisplayStatList.Find(Stat))
	{
		return *pMode;
	}
	else
	{
		return EStatDisplayMode::Hidden;
	}
}

void UBaseFPSSettingsLocal::SetPerfStatDisplayState(EDisplayablePerformanceStat Stat,
	EStatDisplayMode DisplayMode)
{
	if (DisplayMode == EStatDisplayMode::Hidden)
	{
		DisplayStatList.Remove(Stat);
	}
	else
	{
		DisplayStatList.FindOrAdd(Stat) = DisplayMode;
	}
	PerfStatSettingsChangedEvent.Broadcast();
}

float UBaseFPSSettingsLocal::GetDisplayGamma() const
{
	return DisplayGamma;
}

void UBaseFPSSettingsLocal::SetDisplayGamma(float InGamma)
{
	DisplayGamma = InGamma;
	ApplyDisplayGamma();
}

void UBaseFPSSettingsLocal::ApplyDisplayGamma()
{
	if (GEngine)
	{
		GEngine->DisplayGamma = DisplayGamma;
	}
}

void UBaseFPSSettingsLocal::UpdateEffectiveFrameRateLimit()
{
	if (!IsRunningDedicatedServer())
	{
		SetFrameRateLimitCVar(GetEffectiveFrameRateLimit());
	}
}

void UBaseFPSSettingsLocal::UpdateGameModeDeviceProfileAndFps()
{
#if WITH_EDITOR
	if (GIsEditor && !CVarApplyDeviceProfilesInPIE.GetValueOnGameThread())
	{
		return;
	}
#endif
	
	// Omitting all "Platform specific" (i.e. Desktop vs Mobile vs Console) logic
	// Just including Desktop Frame pacing...
	UpdateDesktopFramePacing();
}

void UBaseFPSSettingsLocal::UpdateDesktopFramePacing()
{
	// For desktop the frame rate limit is handled by the parent class based on the value already
	// applied via UpdateEffectiveFrameRateLimit()
	// So this function is only doing 'second order' effects of desktop frame pacing preferences

	const float TargetFPS = GetEffectiveFrameRateLimit();
	const float ClampedFPS = (TargetFPS <= 0.0f) ? 60.0f : FMath::Clamp(TargetFPS, 30.0f, 60.0f);
	UpdateDynamicResFrameTime(ClampedFPS);
}

void UBaseFPSSettingsLocal::UpdateDynamicResFrameTime(float TargetFPS)
{
	static IConsoleVariable* CVarDyResFrameTimeBudget = IConsoleManager::Get().FindConsoleVariable(TEXT("r.DynamicRes.FrameTimeBudget"));
	if (CVarDyResFrameTimeBudget)
	{
		if (ensure(TargetFPS > 0.0f))
		{
			const float DyResFrameTimeBudget = 1000.0f / TargetFPS;
			CVarDyResFrameTimeBudget->Set(DyResFrameTimeBudget, ECVF_SetByGameSetting);
		}
	}
}


bool UBaseFPSSettingsLocal::CanRunAutoBenchmark() const
{
	return true; // PC only for now -- so defaulting to true
}

void UBaseFPSSettingsLocal::RunAutoBenchmark(bool bSaveImmediately)
{
	RunHardwareBenchmark();

	ApplyScalabilitySettings();
	
	if (bSaveImmediately)
	{
		SaveSettings();
	}
}

void UBaseFPSSettingsLocal::ApplyScalabilitySettings()
{
	Scalability::SetQualityLevels(ScalabilityQuality);
}

bool UBaseFPSSettingsLocal::IsHeadphoneModeEnabled() const
{
	return bUseHeadphoneMode;
}

void UBaseFPSSettingsLocal::SetHeadphoneModeEnabled(bool bEnabled)
{
	if (CanModifyHeadphoneModeEnabled())
	{
		static IConsoleVariable* BinauralSpatializationDisabledCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("au.DisableBinauralSpatialization"));
		if (BinauralSpatializationDisabledCVar)
		{
			BinauralSpatializationDisabledCVar->Set(!bEnabled, ECVF_SetByGameSetting);

			// Only save settings if the setting actually changed
			if (bUseHeadphoneMode != bEnabled)
			{
				bUseHeadphoneMode = bEnabled;
				SaveSettings();
			}
		}
	}
}

bool UBaseFPSSettingsLocal::CanModifyHeadphoneModeEnabled() const
{
	static IConsoleVariable* BinauralSpatializationDisabledCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("au.DisableBinauralSpatialization"));
	const bool bHRTFOptionAvailable = BinauralSpatializationDisabledCVar && ((BinauralSpatializationDisabledCVar->GetFlags() & EConsoleVariableFlags::ECVF_SetByMask) <= EConsoleVariableFlags::ECVF_SetByGameSetting);

	const bool bBinauralSettingControlledByOS = BaseFPSSettingsHelpers::HasPlatformTrait(TAG_Platform_Trait_BinauralSettingControlledByOS);

	return bHRTFOptionAvailable && !bBinauralSettingControlledByOS;
}

bool UBaseFPSSettingsLocal::IsHDRAudioModeEnabled() const
{
	return bUseHDRAudioMode;
}

void UBaseFPSSettingsLocal::SetHDRAudioModeEnabled(bool bEnabled)
{
	bUseHDRAudioMode = bEnabled;

	if (GEngine)
	{
		if (const UWorld* World = GEngine->GetCurrentPlayWorld())
		{
			if (UAudioMixEffectsSubsystem* AudioMixEffectsSubsystem = World->GetSubsystem<UAudioMixEffectsSubsystem>())
			{
				AudioMixEffectsSubsystem->ApplyDynamicRangeEffectsChains(bEnabled);
			}
		}
	}
}

float UBaseFPSSettingsLocal::GetOverallVolume() const
{
	return OverallVolume;
}

void UBaseFPSSettingsLocal::SetOverallVolume(float InVolume)
{
	// Cache the incoming volume value
	OverallVolume = InVolume;

	// Check to see if references to the control buses and control bus mixes have been loaded yet
	// Will likely need to be loaded if this function is the first time a setter has been called from the UI
	if (!bSoundControlBusMixLoaded)
	{
		LoadUserControlBusMix();
	}

	// Ensure it's been loaded before continuing
	ensureMsgf(bSoundControlBusMixLoaded, TEXT("UserControlBusMix Settings Failed to Load."));

	// Locate the locally cached bus and set the volume on it
	if (TObjectPtr<USoundControlBus>* ControlBusDblPtr = ControlBusMap.Find(TEXT("Overall")))
	{
		if (USoundControlBus* ControlBusPtr = *ControlBusDblPtr)
		{
			SetVolumeForControlBus(ControlBusPtr, OverallVolume);
		}
	}
}

float UBaseFPSSettingsLocal::GetMusicVolume() const
{
	return MusicVolume;
}

void UBaseFPSSettingsLocal::SetMusicVolume(float InVolume)
{
	// Cache the incoming volume value
	MusicVolume = InVolume;

	// Check to see if references to the control buses and control bus mixes have been loaded yet
	// Will likely need to be loaded if this function is the first time a setter has been called from the UI
	if (!bSoundControlBusMixLoaded)
	{
		LoadUserControlBusMix();
	}

	// Ensure it's been loaded before continuing
	ensureMsgf(bSoundControlBusMixLoaded, TEXT("UserControlBusMix Settings Failed to Load."));

	// Locate the locally cached bus and set the volume on it
	if (TObjectPtr<USoundControlBus>* ControlBusDblPtr = ControlBusMap.Find(TEXT("Music")))
	{
		if (USoundControlBus* ControlBusPtr = *ControlBusDblPtr)
		{
			SetVolumeForControlBus(ControlBusPtr, MusicVolume);
		}
	}
}

float UBaseFPSSettingsLocal::GetSoundFXVolume() const
{
	return SoundFXVolume;
}

void UBaseFPSSettingsLocal::SetSoundFXVolume(float InVolume)
{
	// Cache the incoming volume value
	SoundFXVolume = InVolume;

	// Check to see if references to the control buses and control bus mixes have been loaded yet
	// Will likely need to be loaded if this function is the first time a setter has been called from the UI
	if (!bSoundControlBusMixLoaded)
	{
		LoadUserControlBusMix();
	}

	// Ensure it's been loaded before continuing
	ensureMsgf(bSoundControlBusMixLoaded, TEXT("UserControlBusMix Settings Failed to Load."));

	// Locate the locally cached bus and set the volume on it
	if (TObjectPtr<USoundControlBus>* ControlBusDblPtr = ControlBusMap.Find(TEXT("SoundFX")))
	{
		if (USoundControlBus* ControlBusPtr = *ControlBusDblPtr)
		{
			SetVolumeForControlBus(ControlBusPtr, SoundFXVolume);
		}
	}
}

float UBaseFPSSettingsLocal::GetDialogueVolume() const
{
	return 	DialogueVolume;
}

void UBaseFPSSettingsLocal::SetDialogueVolume(float InVolume)
{
	// Cache the incoming volume value
	DialogueVolume = InVolume;

	// Check to see if references to the control buses and control bus mixes have been loaded yet
	// Will likely need to be loaded if this function is the first time a setter has been called from the UI
	if (!bSoundControlBusMixLoaded)
	{
		LoadUserControlBusMix();
	}

	// Ensure it's been loaded before continuing
	ensureMsgf(bSoundControlBusMixLoaded, TEXT("UserControlBusMix Settings Failed to Load."));

	// Locate the locally cached bus and set the volume on it
	if (TObjectPtr<USoundControlBus>* ControlBusDblPtr = ControlBusMap.Find(TEXT("Dialogue")))
	{
		if (USoundControlBus* ControlBusPtr = *ControlBusDblPtr)
		{
			SetVolumeForControlBus(ControlBusPtr, DialogueVolume);
		}
	}
}

float UBaseFPSSettingsLocal::GetVoiceChatVolume() const
{
	return VoiceChatVolume;
}

void UBaseFPSSettingsLocal::SetVoiceChatVolume(float InVolume)
{
	// Cache the incoming volume value
	VoiceChatVolume = InVolume;

	// Check to see if references to the control buses and control bus mixes have been loaded yet
	// Will likely need to be loaded if this function is the first time a setter has been called from the UI
	if (!bSoundControlBusMixLoaded)
	{
		LoadUserControlBusMix();
	}

	// Ensure it's been loaded before continuing
	ensureMsgf(bSoundControlBusMixLoaded, TEXT("UserControlBusMix Settings Failed to Load."));

	// Locate the locally cached bus and set the volume on it
	if (TObjectPtr<USoundControlBus>* ControlBusDblPtr = ControlBusMap.Find(TEXT("VoiceChat")))
	{
		if (USoundControlBus* ControlBusPtr = *ControlBusDblPtr)
		{
			SetVolumeForControlBus(ControlBusPtr, VoiceChatVolume);
		}
	}
}


void UBaseFPSSettingsLocal::LoadUserControlBusMix()
{
		if (GEngine)
	{
		if (const UWorld* World = GEngine->GetCurrentPlayWorld())
		{
			if (const UBaseFPSAudioSettings* AudioSettings = GetDefault<UBaseFPSAudioSettings>())
			{
				USoundControlBus* OverallControlBus = nullptr;
				USoundControlBus* MusicControlBus = nullptr;
				USoundControlBus* SoundFXControlBus = nullptr;
				USoundControlBus* DialogueControlBus = nullptr;
				USoundControlBus* VoiceChatControlBus = nullptr;
			
				ControlBusMap.Empty();
			
				if (UObject* ObjPath = AudioSettings->OverallVolumeControlBus.TryLoad())
				{
					if (USoundControlBus* SoundControlBus = Cast<USoundControlBus>(ObjPath))
					{
						OverallControlBus = SoundControlBus;
						ControlBusMap.Add(TEXT("Overall"), OverallControlBus);
					}
					else
					{
						ensureMsgf(SoundControlBus, TEXT("Overall Control Bus reference missing from BaseFPS Audio Settings."));
					}
				}
			
				if (UObject* ObjPath = AudioSettings->MusicVolumeControlBus.TryLoad())
				{
					if (USoundControlBus* SoundControlBus = Cast<USoundControlBus>(ObjPath))
					{
						MusicControlBus = SoundControlBus;
						ControlBusMap.Add(TEXT("Music"), MusicControlBus);
					}
					else
					{
						ensureMsgf(SoundControlBus, TEXT("Music Control Bus reference missing from BaseFPS Audio Settings."));
					}
				}
			
				if (UObject* ObjPath = AudioSettings->SoundFXVolumeControlBus.TryLoad())
				{
					if (USoundControlBus* SoundControlBus = Cast<USoundControlBus>(ObjPath))
					{
						SoundFXControlBus = SoundControlBus;
						ControlBusMap.Add(TEXT("SoundFX"), SoundFXControlBus);
					}
					else
					{
						ensureMsgf(SoundControlBus, TEXT("SoundFX Control Bus reference missing from BaseFPS Audio Settings."));
					}
				}
			
				if (UObject* ObjPath = AudioSettings->DialogueVolumeControlBus.TryLoad())
				{
					if (USoundControlBus* SoundControlBus = Cast<USoundControlBus>(ObjPath))
					{
						DialogueControlBus = SoundControlBus;
						ControlBusMap.Add(TEXT("Dialogue"), DialogueControlBus);
					}
					else
					{
						ensureMsgf(SoundControlBus, TEXT("Dialogue Control Bus reference missing from BaseFPS Audio Settings."));
					}
				}
			
				if (UObject* ObjPath = AudioSettings->VoiceChatVolumeControlBus.TryLoad())
				{
					if (USoundControlBus* SoundControlBus = Cast<USoundControlBus>(ObjPath))
					{
						VoiceChatControlBus = SoundControlBus;
						ControlBusMap.Add(TEXT("VoiceChat"), VoiceChatControlBus);
					}
					else
					{
						ensureMsgf(SoundControlBus, TEXT("VoiceChat Control Bus reference missing from BaseFPS Audio Settings."));
					}
				}
			
				if (UObject* ObjPath = AudioSettings->UserSettingsControlBusMix.TryLoad())
				{
					if (USoundControlBusMix* SoundControlBusMix = Cast<USoundControlBusMix>(ObjPath))
					{
						ControlBusMix = SoundControlBusMix;
			
						const FSoundControlBusMixStage OverallControlBusMixStage = UAudioModulationStatics::CreateBusMixStage(World, OverallControlBus, OverallVolume);
						const FSoundControlBusMixStage MusicControlBusMixStage = UAudioModulationStatics::CreateBusMixStage(World, MusicControlBus, MusicVolume);
						const FSoundControlBusMixStage SoundFXControlBusMixStage = UAudioModulationStatics::CreateBusMixStage(World, SoundFXControlBus, SoundFXVolume);
						const FSoundControlBusMixStage DialogueControlBusMixStage = UAudioModulationStatics::CreateBusMixStage(World, DialogueControlBus, DialogueVolume);
						const FSoundControlBusMixStage VoiceChatControlBusMixStage = UAudioModulationStatics::CreateBusMixStage(World, VoiceChatControlBus, VoiceChatVolume);
			
						TArray<FSoundControlBusMixStage> ControlBusMixStageArray;
						ControlBusMixStageArray.Add(OverallControlBusMixStage);
						ControlBusMixStageArray.Add(MusicControlBusMixStage);
						ControlBusMixStageArray.Add(SoundFXControlBusMixStage);
						ControlBusMixStageArray.Add(DialogueControlBusMixStage);
						ControlBusMixStageArray.Add(VoiceChatControlBusMixStage);
			
						UAudioModulationStatics::UpdateMix(World, ControlBusMix, ControlBusMixStageArray);
			
						bSoundControlBusMixLoaded = true;
					}
					else
					{
						ensureMsgf(SoundControlBusMix, TEXT("User Settings Control Bus Mix reference missing from BaseFPS Audio Settings."));
					}
				}
			}
		}
	}
}

void UBaseFPSSettingsLocal::SetVolumeForControlBus(USoundControlBus* InSoundControlBus, float InVolume)
{
	// Check to see if references to the control buses and control bus mixes have been loaded yet
	// Will likely need to be loaded if this function is the first time a setter has been called
	if (!bSoundControlBusMixLoaded)
	{
		LoadUserControlBusMix();
	}

	// Ensure it's been loaded before continuing
	ensureMsgf(bSoundControlBusMixLoaded, TEXT("UserControlBusMix Settings Failed to Load."));

	// Assuming everything has been loaded correctly, we retrieve the world and use AudioModulationStatics to update the Control Bus Volume values and
	// apply the settings to the cached User Control Bus Mix
	if (GEngine && InSoundControlBus && bSoundControlBusMixLoaded)
	{
		if (const UWorld* AudioWorld = GEngine->GetCurrentPlayWorld())
		{
			ensureMsgf(ControlBusMix, TEXT("Control Bus Mix failed to load."));

			// Create and set the Control Bus Mix Stage Parameters
			FSoundControlBusMixStage UpdatedControlBusMixStage;
			UpdatedControlBusMixStage.Bus = InSoundControlBus;
			UpdatedControlBusMixStage.Value.TargetValue = InVolume;
			UpdatedControlBusMixStage.Value.AttackTime = 0.01f;
			UpdatedControlBusMixStage.Value.ReleaseTime = 0.01f;

			// Add the Control Bus Mix Stage to an Array as the UpdateMix function requires
			TArray<FSoundControlBusMixStage> UpdatedMixStageArray;
			UpdatedMixStageArray.Add(UpdatedControlBusMixStage);

			// Modify the matching bus Mix Stage parameters on the User Control Bus Mix
			UAudioModulationStatics::UpdateMix(AudioWorld, ControlBusMix, UpdatedMixStageArray);
		}
	}
}

void UBaseFPSSettingsLocal::SetAudioOutputDeviceId(const FString& InAudioOutputDeviceId)
{
	AudioOutputDeviceId = InAudioOutputDeviceId;
	OnAudioOutputDeviceChanged.Broadcast(InAudioOutputDeviceId);
}

void UBaseFPSSettingsLocal::ApplySafeZoneScale()
{
	SSafeZone::SetGlobalSafeZoneScale(GetSafeZone());
}

void UBaseFPSSettingsLocal::SetControllerPlatform(const FName InControllerPlatform)
{
	if (ControllerPlatform != InControllerPlatform)
	{
		ControllerPlatform = InControllerPlatform;

		// Apply the change to the common input subsystem so that we refresh any input icons we're using.
		if (UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(GetTypedOuter<ULocalPlayer>()))
		{
			InputSubsystem->SetGamepadInputType(ControllerPlatform);
		}
	}
}

FName UBaseFPSSettingsLocal::GetControllerPlatform() const
{
	return ControllerPlatform;
}

void UBaseFPSSettingsLocal::RegisterInputConfig(ECommonInputType Type, const UPlayerMappableInputConfig* NewConfig,
                                              const bool bIsActive)
{
	if (NewConfig)
	{
		const int32 ExistingConfigIdx = RegisteredInputConfigs.IndexOfByPredicate([&NewConfig](const FMappableConfigPair& Pair) { return Pair.Config == NewConfig; });
		if (ExistingConfigIdx == INDEX_NONE)
		{
			const int32 NumAdded = RegisteredInputConfigs.Add(FMappableConfigPair(NewConfig, Type, bIsActive));
			if (NumAdded != INDEX_NONE)
			{
				OnInputConfigRegistered.Broadcast(RegisteredInputConfigs[NumAdded]);
			}
		}
	}
}

int32 UBaseFPSSettingsLocal::UnregisterInputConfig(const UPlayerMappableInputConfig* ConfigToRemove)
{
	if (ConfigToRemove)
	{
		const int32 Index = RegisteredInputConfigs.IndexOfByPredicate([&ConfigToRemove](const FMappableConfigPair& Pair) { return Pair.Config == ConfigToRemove; });
		if (Index != INDEX_NONE)
		{
			RegisteredInputConfigs.RemoveAt(Index);
			return 1;
		}
	}
	return INDEX_NONE;
}

const UPlayerMappableInputConfig* UBaseFPSSettingsLocal::GetInputConfigByName(FName ConfigName) const
{
	for (const FMappableConfigPair& Pair: RegisteredInputConfigs)
	{
		if (Pair.Config->GetConfigName() == ConfigName)
		{
			return Pair.Config;
		}
	}
	return nullptr;
}

void UBaseFPSSettingsLocal::GetRegisteredInputConfigsOfType(ECommonInputType Type,
	TArray<FMappableConfigPair>& OutArray) const
{
	OutArray.Empty();

	if (Type == ECommonInputType::Count)
	{
		OutArray = RegisteredInputConfigs;
		return;
	}

	for (const FMappableConfigPair& Pair : RegisteredInputConfigs)
	{
		if (Pair.Type == Type)
		{
			OutArray.Emplace(Pair);
		}	
	}
}

void UBaseFPSSettingsLocal::GetAllMappingNamesFromKey(const FKey InKey, TArray<FName>& OutActionNames)
{
	if (InKey == EKeys::Invalid)
	{
		return;
	}

	for (const FMappableConfigPair& Pair : RegisteredInputConfigs)
	{
		if (Pair.Type == ECommonInputType::MouseAndKeyboard)
		{
			for (const FEnhancedActionKeyMapping& Mapping : Pair.Config->GetPlayerMappableKeys())
			{
				FName MappingName(Mapping.PlayerMappableOptions.DisplayName.ToString());
				FName ActionName = Mapping.PlayerMappableOptions.Name;
				// make sure it isn't custom bound as well
				if (const FKey* MappingKey = CustomKeyboardConfig.Find(ActionName))
				{
					if (*MappingKey == InKey)
					{
						OutActionNames.Add(MappingName);
					}
				}
				else
				{
					if (Mapping.Key == InKey)
					{
						OutActionNames.Add(MappingName);
					}
				}
			}
		}
	}
	
}

void UBaseFPSSettingsLocal::AddOrUpdateCustomKeyboardBindings(const FName MappingName, const FKey NewKey,
	UBaseFPSLocalPlayer* LocalPlayer)
{
	if (MappingName == NAME_None)
	{
		return;
	}

	if (InputConfigName != TEXT("Custom"))
	{
		// Copy Presets.
		if (const UPlayerMappableInputConfig* DefaultConfig = GetInputConfigByName(TEXT("Default")))
		{
			for (FEnhancedActionKeyMapping& Mapping : DefaultConfig->GetPlayerMappableKeys())
			{
				// Make sure that the mapping has a valid name, its possible to have an empty name
				// if someone has marked a mapping as "Player Mappable" but deleted the default field value
				if (Mapping.PlayerMappableOptions.Name != NAME_None)
				{
					CustomKeyboardConfig.Add(Mapping.PlayerMappableOptions.Name, Mapping.Key);
				}
			}
		}

		InputConfigName = TEXT("Custom");
	}

	if (FKey* ExistingMapping = CustomKeyboardConfig.Find(MappingName))
	{
		// Change the key to the new one
		CustomKeyboardConfig[MappingName] = NewKey;
	}
	else
	{
		CustomKeyboardConfig.Add(MappingName, NewKey);
	}

	// Tell the enhanced input subsystem for this local player that we should remap some input! Woo :)
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
	{
		Subsystem->AddPlayerMappedKey(MappingName, NewKey);
	}
}

void UBaseFPSSettingsLocal::ResetKeybindingToDefault(const FName MappingName, UBaseFPSLocalPlayer* LocalPlayer)
{
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
	{
		Subsystem->RemovePlayerMappedKey(MappingName);
	}
}

void UBaseFPSSettingsLocal::ResetKeybindingsToDefault(UBaseFPSLocalPlayer* LocalPlayer)
{
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
	{
		Subsystem->RemoveAllPlayerMappedKeys();
	}
}

void UBaseFPSSettingsLocal::OnAppActivationStateChanged(bool bIsActive)
{
	// We might want to adjust the frame rate when the app loses/gains focus on multi-window platforms
	UpdateEffectiveFrameRateLimit();
}

float UBaseFPSSettingsLocal::GetFrameRateLimit_OnBattery() const
{
	return FrameRateLimit_OnBattery;
}

void UBaseFPSSettingsLocal::SetFrameRateLimit_OnBattery(float NewLimitFPS)
{
	FrameRateLimit_OnBattery = NewLimitFPS;
	UpdateEffectiveFrameRateLimit();
}

float UBaseFPSSettingsLocal::GetFrameRateLimit_InMenu() const
{
	return FrameRateLimit_InMenu;
}

void UBaseFPSSettingsLocal::SetFrameRateLimit_InMenu(float NewLimitFPS)
{
	FrameRateLimit_InMenu = NewLimitFPS;
	UpdateEffectiveFrameRateLimit();
}

float UBaseFPSSettingsLocal::GetFrameRateLimit_WhenBackgrounded() const
{
	return FrameRateLimit_WhenBackgrounded;
}

void UBaseFPSSettingsLocal::SetFrameRateLimit_WhenBackgrounded(float NewLimitFPS)
{
	FrameRateLimit_WhenBackgrounded = NewLimitFPS;
	UpdateEffectiveFrameRateLimit();
}

float UBaseFPSSettingsLocal::GetFrameRateLimit_Always() const
{
	return GetFrameRateLimit();
}

void UBaseFPSSettingsLocal::SetFrameRateLimit_Always(float NewLimitFPS)
{
	SetFrameRateLimit(NewLimitFPS);
	UpdateEffectiveFrameRateLimit();
}
