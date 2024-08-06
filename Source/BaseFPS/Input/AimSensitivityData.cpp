// Fill out your copyright notice in the Description page of Project Settings.


#include "AimSensitivityData.h"

#include "Containers/Set.h"
#include "initializer_list"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AimSensitivityData)

UAimSensitivityData::UAimSensitivityData(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SensitivityMap =
	{
		{ EGamepadSensitivity::Slow,			0.5f },
		{ EGamepadSensitivity::SlowPlus,		0.75f },
		{ EGamepadSensitivity::SlowPlusPlus,	0.9f },
		{ EGamepadSensitivity::Normal,		1.0f },
		{ EGamepadSensitivity::NormalPlus,	1.1f },
		{ EGamepadSensitivity::NormalPlusPlus,1.25f },
		{ EGamepadSensitivity::Fast,			1.5f },
		{ EGamepadSensitivity::FastPlus,		1.75f },
		{ EGamepadSensitivity::FastPlusPlus,	2.0f },
		{ EGamepadSensitivity::Insane,		2.5f },
	};
}

const float UAimSensitivityData::SensitivtyEnumToFloat(const EGamepadSensitivity InSensitivity) const
{
	if (const float* Sens = SensitivityMap.Find(InSensitivity))
	{
		return *Sens;
	}

	return 1.0f;
}
