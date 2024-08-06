// Fill out your copyright notice in the Description page of Project Settings.


#include "Performance/PerformanceSettings.h"

//////////////////////////////////////////////////////////////////////

UPlatformSpecificRenderingSettings::UPlatformSpecificRenderingSettings()
{
}

const UPlatformSpecificRenderingSettings* UPlatformSpecificRenderingSettings::Get()
{
	UPlatformSpecificRenderingSettings* Result = UPlatformSettingsManager::Get().GetSettingsForPlatform<ThisClass>();
	check(Result);
	return Result;
}

//////////////////////////////////////////////////////////////////////

UPerformanceSettings::UPerformanceSettings()
{
	PerPlatformSettings.Initialize(UPlatformSpecificRenderingSettings::StaticClass());

	CategoryName = TEXT("Game");

	DesktopFrameRateLimits.Append({ 30, 60, 120, 144, 160, 165, 180, 200, 240, 360 });

	// Default to all stats are allowed
	FPerformanceStatGroup& StatGroup = UserFacingPerformanceStats.AddDefaulted_GetRef();
	for (EDisplayablePerformanceStat PerfStat : TEnumRange<EDisplayablePerformanceStat>())
	{
		StatGroup.AllowedStats.Add(PerfStat);
	}
}