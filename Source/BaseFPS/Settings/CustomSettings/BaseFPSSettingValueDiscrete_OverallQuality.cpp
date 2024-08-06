// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseFPSSettingValueDiscrete_OverallQuality.h"

#include "Settings/BaseFPSSettingsLocal.h"
#include "Internationalization/Internationalization.h"

#define LOCTEXT_NAMESPACE "BaseFPSSettings"

UBaseFPSSettingValueDiscrete_OverallQuality::UBaseFPSSettingValueDiscrete_OverallQuality()
{
}

void UBaseFPSSettingValueDiscrete_OverallQuality::OnInitialized()
{
	Super::OnInitialized();

	// Removed Mobile-specific logic. Not needed
	Options.Add(LOCTEXT("VideoQualityOverall_Low", "Low"));
	Options.Add(LOCTEXT("VideoQualityOverall_Medium", "Medium"));
	Options.Add(LOCTEXT("VideoQualityOverall_High", "High"));
	Options.Add(LOCTEXT("VideoQualityOverall_Epic", "Epic"));

	OptionsWithCustom = Options;
	OptionsWithCustom.Add(LOCTEXT("VideoQualityOverall_Custom", "Custom"));
}

void UBaseFPSSettingValueDiscrete_OverallQuality::StoreInitial()
{
}

void UBaseFPSSettingValueDiscrete_OverallQuality::ResetToDefault()
{
}

void UBaseFPSSettingValueDiscrete_OverallQuality::RestoreToInitial()
{
}

void UBaseFPSSettingValueDiscrete_OverallQuality::SetDiscreteOptionByIndex(int32 Index)
{
	UGameUserSettings* UserSettings = CastChecked<UGameUserSettings>(GEngine->GetGameUserSettings());

	if (Index == GetCustomOptionIndex())
	{
		// Leave everything as is we're in a custom setup.
	}
	else
	{
		// Low / Medium / High / Epic
		UserSettings->SetOverallScalabilityLevel(Index);
	}

	NotifySettingChanged(EGameSettingChangeReason::Change);
}

int32 UBaseFPSSettingValueDiscrete_OverallQuality::GetDiscreteOptionIndex() const
{
	const int32 OverallQualityLevel = GetOverallQualityLevel();
	if (OverallQualityLevel == INDEX_NONE)
	{
		return GetCustomOptionIndex();
	}

	return OverallQualityLevel;
}

TArray<FText> UBaseFPSSettingValueDiscrete_OverallQuality::GetDiscreteOptions() const
{
	const int32 OverallQualityLevel = GetOverallQualityLevel();
	if (OverallQualityLevel == INDEX_NONE)
	{
		return OptionsWithCustom;
	}
	else
	{
		return Options;
	}
}

int32 UBaseFPSSettingValueDiscrete_OverallQuality::GetCustomOptionIndex() const
{
	return OptionsWithCustom.Num() - 1;
}

int32 UBaseFPSSettingValueDiscrete_OverallQuality::GetOverallQualityLevel() const
{
	const UGameUserSettings* UserSettings = CastChecked<const UGameUserSettings>(GEngine->GetGameUserSettings());
	return UserSettings->GetOverallScalabilityLevel();
}

#undef LOCTEXT_NAMESPACE
