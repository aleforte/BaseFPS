// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseFPSSettingValueDiscrete_PerfStat.h"

#include "CommonUIVisibilitySubsystem.h"
#include "Performance/PerformanceSettings.h"
#include "Settings/BaseFPSSettingsLocal.h"
#include "Internationalization/Internationalization.h"

#define LOCTEXT_NAMESPACE "BaseFPSSettings"

//////////////////////////////////////////////////////////////////////

class FGameSettingEditCondition_PerfStatAllowed : public FGameSettingEditCondition
{
public:
	static TSharedRef<FGameSettingEditCondition_PerfStatAllowed> Get(EDisplayablePerformanceStat Stat)
	{
		return MakeShared<FGameSettingEditCondition_PerfStatAllowed>(Stat);
	}

	FGameSettingEditCondition_PerfStatAllowed(EDisplayablePerformanceStat Stat)
		: AssociatedStat(Stat)
	{
	}

	//~Begin FGameSettingEditCondition interface
	virtual void GatherEditState(const ULocalPlayer* InLocalPlayer, FGameSettingEditableState& InOutEditState) const override
	{
		const FGameplayTagContainer& VisibilityTags = UCommonUIVisibilitySubsystem::GetChecked(InLocalPlayer)->GetVisibilityTags();

		bool bCanShowStat = false;
		for (const FPerformanceStatGroup& Group : GetDefault<UPerformanceSettings>()->UserFacingPerformanceStats) //@TODO: Move this stuff to per-platform instead of doing vis queries too?
		{
			if (Group.AllowedStats.Contains(AssociatedStat))
			{
				const bool bShowGroup = (Group.VisibilityQuery.IsEmpty() || Group.VisibilityQuery.Matches(VisibilityTags));
				if (bShowGroup)
				{
					bCanShowStat = true;
					break;
				}
			}
		}

		if (!bCanShowStat)
		{
			InOutEditState.Hide(TEXT("Stat is not listed in UPerformanceSettings or is suppressed by current platform traits"));
		}
	}
	//~End FGameSettingEditCondition interface

private:
	EDisplayablePerformanceStat AssociatedStat;
};

//////////////////////////////////////////////////////////////////////

UBaseFPSSettingValueDiscrete_PerfStat::UBaseFPSSettingValueDiscrete_PerfStat()
{
}

void UBaseFPSSettingValueDiscrete_PerfStat::SetStat(EDisplayablePerformanceStat InStat)
{
	StatToDisplay = InStat;
	SetDevName(FName(*FString::Printf(TEXT("PerfStat_%d"), (int32)StatToDisplay)));
	AddEditCondition(FGameSettingEditCondition_PerfStatAllowed::Get(StatToDisplay));
}

void UBaseFPSSettingValueDiscrete_PerfStat::AddMode(FText&& Label, EStatDisplayMode Mode)
{
	Options.Emplace(MoveTemp(Label));
	DisplayModes.Add(Mode);
}

void UBaseFPSSettingValueDiscrete_PerfStat::OnInitialized()
{
	Super::OnInitialized();

	AddMode(LOCTEXT("PerfStatDisplayMode_None", "None"), EStatDisplayMode::Hidden);
	AddMode(LOCTEXT("PerfStatDisplayMode_TextOnly", "Text Only"), EStatDisplayMode::TextOnly);
	AddMode(LOCTEXT("PerfStatDisplayMode_GraphOnly", "Graph Only"), EStatDisplayMode::GraphOnly);
	AddMode(LOCTEXT("PerfStatDisplayMode_TextAndGraph", "Text and Graph"), EStatDisplayMode::TextAndGraph);
}

void UBaseFPSSettingValueDiscrete_PerfStat::StoreInitial()
{
	const UBaseFPSSettingsLocal* Settings = UBaseFPSSettingsLocal::Get();
	InitialMode = Settings->GetPerfStatDisplayState(StatToDisplay);
}

void UBaseFPSSettingValueDiscrete_PerfStat::ResetToDefault()
{
	UBaseFPSSettingsLocal* Settings = UBaseFPSSettingsLocal::Get();
	Settings->SetPerfStatDisplayState(StatToDisplay, EStatDisplayMode::Hidden);
	NotifySettingChanged(EGameSettingChangeReason::ResetToDefault);
}

void UBaseFPSSettingValueDiscrete_PerfStat::RestoreToInitial()
{
	UBaseFPSSettingsLocal* Settings = UBaseFPSSettingsLocal::Get();
	Settings->SetPerfStatDisplayState(StatToDisplay, InitialMode);
	NotifySettingChanged(EGameSettingChangeReason::RestoreToInitial);
}

void UBaseFPSSettingValueDiscrete_PerfStat::SetDiscreteOptionByIndex(int32 Index)
{
	if (DisplayModes.IsValidIndex(Index))
	{
		const EStatDisplayMode DisplayMode = DisplayModes[Index];

		UBaseFPSSettingsLocal* Settings = UBaseFPSSettingsLocal::Get();
		Settings->SetPerfStatDisplayState(StatToDisplay, DisplayMode);
	}
	NotifySettingChanged(EGameSettingChangeReason::Change);
}

int32 UBaseFPSSettingValueDiscrete_PerfStat::GetDiscreteOptionIndex() const
{
	const UBaseFPSSettingsLocal* Settings = UBaseFPSSettingsLocal::Get();
	return DisplayModes.Find(Settings->GetPerfStatDisplayState(StatToDisplay));
}

TArray<FText> UBaseFPSSettingValueDiscrete_PerfStat::GetDiscreteOptions() const
{
	return Options;
}

#undef LOCTEXT_NAMESPACE


