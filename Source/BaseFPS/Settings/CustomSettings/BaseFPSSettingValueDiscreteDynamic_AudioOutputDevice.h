// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AudioMixerBlueprintLibrary.h"
#include "GameSettingValueDiscreteDynamic.h"
#include "BaseFPSSettingValueDiscreteDynamic_AudioOutputDevice.generated.h"

struct FFrame;

enum class EAudioDeviceChangedRole : uint8;

/**
 * 
 */
UCLASS()
class BASEFPS_API UBaseFPSSettingValueDiscreteDynamic_AudioOutputDevice : public UGameSettingValueDiscreteDynamic
{
	GENERATED_BODY()
public:

	virtual ~UBaseFPSSettingValueDiscreteDynamic_AudioOutputDevice() = default;

	/** UGameSettingValueDiscrete */
	virtual void SetDiscreteOptionByIndex(int32 Index) override;

protected:

	virtual void OnInitialized() override;

public:

	UFUNCTION()
	void OnAudioOutputDevicesObtained(const TArray<FAudioOutputDeviceInfo>& AvailableDevices);

	UFUNCTION()
	void OnCompletedDeviceSwap(const FSwapAudioOutputResult& SwapResult);

	UFUNCTION()
	void DeviceAddedOrRemoved(FString DeviceId);

	UFUNCTION()
	void DefaultDeviceChanged(EAudioDeviceChangedRole InRole, FString DeviceId);

protected:

	TArray<FAudioOutputDeviceInfo> OutputDevices;
	FString CurrentDeviceId;
	FString SystemDefaultDeviceId;
	int32 LastKnownGoodIndex = 0;
	bool bRequestDefault = false;

	FOnAudioOutputDevicesObtained DevicesObtainedCallback;
	FOnCompletedDeviceSwap DevicesSwappedCallback;
};
