// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonInputBaseTypes.h"
#include "Scalability.h"
#include "GameFramework/GameUserSettings.h"
#include "Performance/PerformanceStatTypes.h"
#include "BaseFPSSettingsLocal.generated.h"

class UBaseFPSLocalPlayer;
class USoundControlBus;
class USoundControlBusMix;
struct FMappableConfigPair;

/**
 * UBaseFPSSettingsLocal
 */
UCLASS()
class BASEFPS_API UBaseFPSSettingsLocal : public UGameUserSettings
{
	GENERATED_BODY()

public:
	UBaseFPSSettingsLocal();

	static UBaseFPSSettingsLocal* Get();

	//~Begin UObject interface
	virtual void BeginDestroy() override;
	//~End UObject interface

	//~Begin UGameUserSettings interface
	virtual void SetToDefaults() override;
	virtual void LoadSettings(bool bForceReload) override;
	virtual void ConfirmVideoMode() override;
	virtual float GetEffectiveFrameRateLimit() override;
	virtual void ResetToCurrentSettings() override;
	virtual void ApplyNonResolutionSettings() override;
	virtual int32 GetOverallScalabilityLevel() const override;
	virtual void SetOverallScalabilityLevel(int32 Value) override;
	//~End UGameUserSettings interface

	/************************************************************************/
	/* Frontend State                                                       */
	/************************************************************************/
public:
	void SetShouldUseFrontendPerformanceSettings(bool bInFrontEnd);
protected:
	bool ShouldUseFrontendPerformanceSettings() const;
private:
	bool bInFrontEndForPerformancePurposes = false;

	/************************************************************************/
	/* Performance Stats                                                    */
	/************************************************************************/
	public:
	/** Returns the display mode for the specified performance stat */
	EStatDisplayMode GetPerfStatDisplayState(EDisplayablePerformanceStat Stat) const;
	
	/** Sets the display mode for the specified performance stat */
	void SetPerfStatDisplayState(EDisplayablePerformanceStat Stat, EStatDisplayMode DisplayMode);

	/** Fired when the display state for a performance stat has changed, or the settings are applied */
	DECLARE_EVENT(UBaseFPSSettingsLocal, FPerfStatSettingsChanged);
	FPerfStatSettingsChanged& OnPerfStatDisplayStateChanged() { return PerfStatSettingsChangedEvent; }

private:
	// List of stats to display in the HUD
	UPROPERTY(Config)
	TMap<EDisplayablePerformanceStat, EStatDisplayMode> DisplayStatList;

	// Event for display stat widget containers to bind to
	FPerfStatSettingsChanged PerfStatSettingsChangedEvent;

	/************************************************************************/
	/* Brightness/Gamma                                                     */
	/************************************************************************/
public:
	UFUNCTION()
	float GetDisplayGamma() const;
	UFUNCTION()
	void SetDisplayGamma(float InGamma);
	
private:
	void ApplyDisplayGamma();
	
	UPROPERTY(Config)
	float DisplayGamma = 2.2;

	/************************************************************************/
	/* Display                                                              */
	/************************************************************************/
public:
	UFUNCTION()
	float GetFrameRateLimit_OnBattery() const;
	UFUNCTION()
	void SetFrameRateLimit_OnBattery(float NewLimitFPS);

	UFUNCTION()
	float GetFrameRateLimit_InMenu() const;
	UFUNCTION()
	void SetFrameRateLimit_InMenu(float NewLimitFPS);

	UFUNCTION()
	float GetFrameRateLimit_WhenBackgrounded() const;
	UFUNCTION()
	void SetFrameRateLimit_WhenBackgrounded(float NewLimitFPS);

	UFUNCTION()
	float GetFrameRateLimit_Always() const;
	UFUNCTION()
	void SetFrameRateLimit_Always(float NewLimitFPS);

protected:
	void UpdateEffectiveFrameRateLimit();
	
private:
	UPROPERTY(Config)
	float FrameRateLimit_OnBattery;
	UPROPERTY(Config)
	float FrameRateLimit_InMenu;
	UPROPERTY(Config)
	float FrameRateLimit_WhenBackgrounded;

	/************************************************************************/
	/* Display - Console Quality Presets                                    */
	/************************************************************************/
protected:
	/** Updates device profiles, FPS mode etc for the current game mode */
	void UpdateGameModeDeviceProfileAndFps();
	
	void UpdateDesktopFramePacing();

	void UpdateDynamicResFrameTime(float TargetFPS);

private:
	UPROPERTY(Transient)
	FString DesiredUserChosenDeviceProfileSuffix;

	UPROPERTY(Transient)
	FString CurrentAppliedDeviceProfileOverrideSuffix;

	UPROPERTY(config)
	FString UserChosenDeviceProfileSuffix;

	/************************************************************************/
	/* Graphics Quality                                                     */
	/************************************************************************/
public:
	/** Returns true if this platform can run the auto benchmark */
	UFUNCTION(BlueprintCallable, Category="Settings")
	bool CanRunAutoBenchmark() const;
	
	/** Run the auto benchmark, optionally saving right away */
	UFUNCTION(BlueprintCallable, Category="Settings")
	void RunAutoBenchmark(bool bSaveImmediately);

	/** Apply just the quality scalability settings */
	void ApplyScalabilitySettings();

	/************************************************************************/
	/* Audio - Volume                                                       */
	/************************************************************************/
public:
	DECLARE_EVENT_OneParam(UBaseFPSSettingsLocal, FAudioDeviceChanged, const FString& /*DeviceId*/);
	FAudioDeviceChanged OnAudioOutputDeviceChanged;

public:
	/** Returns if we're using headphone mode (HRTF) **/
	UFUNCTION()
	bool IsHeadphoneModeEnabled() const;

	/** Enables or disables headphone mode (HRTF) - NOTE this setting will be overruled if au.DisableBinauralSpatialization is set */
	UFUNCTION()
	void SetHeadphoneModeEnabled(bool bEnabled);

	/** Returns if we can enable/disable headphone mode (i.e., if it's not forced on or off by the platform) */
	UFUNCTION()
	bool CanModifyHeadphoneModeEnabled() const;

public:
	/** Whether we *want* to use headphone mode (HRTF); may or may not actually be applied **/
	UPROPERTY(Transient)
	bool bDesiredHeadphoneMode;

private:
	/** Whether to use headphone mode (HRTF) **/
	UPROPERTY(config)
	bool bUseHeadphoneMode;

public:
	/** Returns if we're using High Dynamic Range Audio mode (HDR Audio) **/
	UFUNCTION()
	bool IsHDRAudioModeEnabled() const;

	/** Enables or disables High Dynamic Range Audio mode (HDR Audio) */
	UFUNCTION()
	void SetHDRAudioModeEnabled(bool bEnabled);

	/** Whether to use High Dynamic Range Audio mode (HDR Audio) **/
	UPROPERTY(config)
	bool bUseHDRAudioMode;
	
public:
	UFUNCTION()
	float GetOverallVolume() const;
	UFUNCTION()
	void SetOverallVolume(float InVolume);

	UFUNCTION()
	float GetMusicVolume() const;
	UFUNCTION()
	void SetMusicVolume(float InVolume);

	UFUNCTION()
	float GetSoundFXVolume() const;
	UFUNCTION()
	void SetSoundFXVolume(float InVolume);

	UFUNCTION()
	float GetDialogueVolume() const;
	UFUNCTION()
	void SetDialogueVolume(float InVolume);

	UFUNCTION()
	float GetVoiceChatVolume() const;
	UFUNCTION()
	void SetVoiceChatVolume(float InVolume);
	
private:
	void LoadUserControlBusMix();
	
	void SetVolumeForControlBus(USoundControlBus* InSoundControlBus, float InVolume);
	
	UPROPERTY(Config)
	float OverallVolume = 1.0f;
	UPROPERTY(Config)
	float MusicVolume = 1.0f;
	UPROPERTY(Config)
	float SoundFXVolume = 1.0f;
	UPROPERTY(Config)
	float DialogueVolume = 1.0f;
	UPROPERTY(Config)
	float VoiceChatVolume = 1.0f;

	UPROPERTY(Transient)
	TMap<FName/*SoundClassName*/, TObjectPtr<USoundControlBus>> ControlBusMap;

	UPROPERTY(Transient)
	TObjectPtr<USoundControlBusMix> ControlBusMix = nullptr;

	UPROPERTY(Transient)
	bool bSoundControlBusMixLoaded;

	/************************************************************************/
	/* Audio - Sound                                                        */
	/************************************************************************/
public:
	/** Returns the user's audio device id */
	UFUNCTION()
	FString GetAudioOutputDeviceId() const { return AudioOutputDeviceId; }

	/** Sets the user's audio device by id */
	UFUNCTION()
	void SetAudioOutputDeviceId(const FString& InAudioOutputDeviceId);

private:
	UPROPERTY(Config)
	FString AudioOutputDeviceId;
	
	void SetVolumeForSoundClass(FName ChannelName, float InVolume); // not implemented

	/************************************************************************/
	/* Safezone                                                             */
	/************************************************************************/
public:
	UFUNCTION()
	bool IsSafeZoneSet() const { return SafeZoneScale != -1; }
	UFUNCTION()
	float GetSafeZone() const { return SafeZoneScale >= 0 ? SafeZoneScale : 0; }
	UFUNCTION()
	void SetSafeZone(float Value) { SafeZoneScale = Value; ApplySafeZoneScale(); }

	void ApplySafeZoneScale();
private:

	UPROPERTY(Config)
	float SafeZoneScale = -1;

	/************************************************************************/
	/* Keybindings                                                          */
	/************************************************************************/
public:

	// Sets the controller representation to use, a single platform might support multiple kinds of controllers.  For
	// example, Win64 games could be played with both an XBox or Playstation controller.
	UFUNCTION()
	void SetControllerPlatform(const FName InControllerPlatform);
	UFUNCTION()
	FName GetControllerPlatform() const;

	DECLARE_EVENT_OneParam(UBaseFPSSettingsLocal, FInputConfigDelegate, const FMappableConfigPair& /*Config*/)
	
	/** Delegate called when a new input config has been registered */
	FInputConfigDelegate OnInputConfigRegistered;

	/** Delegate called when a registered input config has been activated */
	FInputConfigDelegate OnInputConfigActivated;
	
	/** Delegate called when a registered input config has been deactivate */
	FInputConfigDelegate OnInputConfigDeactivated;
	
	/** Register the given input config with the settings to make it available to the player. */
	void RegisterInputConfig(ECommonInputType Type, const UPlayerMappableInputConfig* NewConfig, const bool bIsActive);
	
	/** Unregister the given input config. Returns the number of configs removed. */
	int32 UnregisterInputConfig(const UPlayerMappableInputConfig* ConfigToRemove);

	/** Get an input config with a certain name. If the config doesn't exist then nullptr will be returned. */
	UFUNCTION(BlueprintCallable)
	const UPlayerMappableInputConfig* GetInputConfigByName(FName ConfigName) const;

	/** Get all currently registered input configs */
	const TArray<FMappableConfigPair>& GetAllRegisteredInputConfigs() const { return RegisteredInputConfigs; }

	/**
	 * Get all registered input configs that match the input type.
	 * 
	 * @param Type		The type of config to get, ECommonInputType::Count will include all configs.
	 * @param OutArray	Array to be populated with the current registered input configs that match the type
	 */
	void GetRegisteredInputConfigsOfType(ECommonInputType Type, OUT TArray<FMappableConfigPair>& OutArray) const;
	
	/**
	 * Returns the display name of any actions with that key bound to it
	 * 
	 * @param InKey The key to check for current mappings of
	 * @param OutActionNames Array to store display names of actions of bound keys
	 */
	void GetAllMappingNamesFromKey(const FKey InKey, TArray<FName>& OutActionNames);

	/**
 * Maps the given keyboard setting to the new key
 * 
 * @param MappingName	The name of the FPlayerMappableKeyOptions that you would like to change
 * @param NewKey		The new key to bind this option to
 * @param LocalPlayer   local player to reset the keybinding on
 */
	void AddOrUpdateCustomKeyboardBindings(const FName MappingName, const FKey NewKey, UBaseFPSLocalPlayer* LocalPlayer);

	/**
	 * Resets keybinding to its default value in its input mapping context 
	 * 
	 * @param MappingName	The name of the FPlayerMappableKeyOptions that you would like to change
	 * @param LocalPlayer   local player to reset the keybinding on
	 */
	void ResetKeybindingToDefault(const FName MappingName, UBaseFPSLocalPlayer* LocalPlayer);

	/** Resets all keybindings to their default value in their input mapping context
	 * @param LocalPlayer   local player to reset the keybinding on
	 */
	void ResetKeybindingsToDefault(UBaseFPSLocalPlayer* LocalPlayer);

	const TMap<FName, FKey>& GetCustomPlayerInputConfig() const { return CustomKeyboardConfig; }

private:

	/**
	 * The name of the controller the player is using.  This is maps to the name of a UCommonInputBaseControllerData
	 * that is available on this current platform.  The gamepad data are registered per platform, you'll find them
	 * in <Platform>Game.ini files listed under +ControllerData=...
	 */
	UPROPERTY(Config)
	FName ControllerPlatform;

	UPROPERTY(Config)
	FName ControllerPreset = TEXT("Default");
	
	/** The name of the current input config that the user has selected. */
	UPROPERTY(Config)
	FName InputConfigName = TEXT("Default");
	
	/**
	 * Array of currently registered input configs. This is populated by game feature plugins
	 * 
	 * @see UGameFeatureAction_AddInputConfig
	 */
	UPROPERTY(VisibleAnywhere)
	TArray<FMappableConfigPair> RegisteredInputConfigs;

	/** Array of custom key mappings that have been set by the player. Empty by default. */
	UPROPERTY(Config)
	TMap<FName, FKey> CustomKeyboardConfig;
	
private:
	void OnAppActivationStateChanged(bool bIsActive);
	
private:
	FDelegateHandle OnApplicationActivationStateChangedHandle;	
};
