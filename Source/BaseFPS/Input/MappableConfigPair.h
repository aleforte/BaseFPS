// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonInputTypeEnum.h"

#include "MappableConfigPair.generated.h"

class UPlayerMappableInputConfig;

/** A container to organize loaded player mappable configs to their CommonUI input type */
USTRUCT(BlueprintType)
struct FMappableConfigPair
{
	GENERATED_BODY()

public:
	FMappableConfigPair() = default;
	FMappableConfigPair(const UPlayerMappableInputConfig* InConfig, ECommonInputType InType, const bool InIsActive)
		: Config(InConfig)
		, Type(InType)
		, bIsActive(InIsActive)
	{}

	/** The player mappable input config that should be applied to the Enhanced Input subsystem */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	TObjectPtr<const UPlayerMappableInputConfig> Config = nullptr;

	/** The type of device that this mapping config should be applied to */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	ECommonInputType Type = ECommonInputType::Count;

	/** If this config is currently active. A config is marked as active when it's owning GFA is active */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	bool bIsActive = false;
};

// FMappableConfigPair Omitted (Looks to be only relevant for GameFeatures & Lyra's "Experience" framework).

