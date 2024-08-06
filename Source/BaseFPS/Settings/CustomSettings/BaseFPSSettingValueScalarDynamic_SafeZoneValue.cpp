// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseFPSSettingValueScalarDynamic_SafeZoneValue.h"

#include "DataSource/GameSettingDataSourceDynamic.h"
#include "Player/BaseFPSLocalPlayer.h"
#include "Settings/BaseFPSSettingRegistry.h"
#include "Widgets/Layout/SSafeZone.h"

#define LOCTEXT_NAMESPACE "BaseFPSSettings"

UBaseFPSSettingAction_SafeZoneEditor::UBaseFPSSettingAction_SafeZoneEditor()
{
	SafeZoneValueSetting = NewObject<UBaseFPSSettingValueScalarDynamic_SafeZoneValue>();
	SafeZoneValueSetting->SetDevName(TEXT("SafeZoneValue"));
	SafeZoneValueSetting->SetDisplayName(LOCTEXT("SafeZoneValue_Name", "Safe Zone Value"));
	SafeZoneValueSetting->SetDescriptionRichText(LOCTEXT("SafeZoneValue_Description", "The safezone area percentage."));
	SafeZoneValueSetting->SetDefaultValue(0.0f);
	SafeZoneValueSetting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetSafeZone));
	SafeZoneValueSetting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetSafeZone));
	SafeZoneValueSetting->SetDisplayFormat([](double SourceValue, double NormalizedValue){ return FText::AsNumber(SourceValue); });
	SafeZoneValueSetting->SetSettingParent(this);
}

TArray<UGameSetting*> UBaseFPSSettingAction_SafeZoneEditor::GetChildSettings()
{
	return { SafeZoneValueSetting };
}


void UBaseFPSSettingValueScalarDynamic_SafeZoneValue::ResetToDefault()
{
	Super::ResetToDefault();
	SSafeZone::SetGlobalSafeZoneScale(TOptional<float>(DefaultValue.Get(0.0f)));
}

void UBaseFPSSettingValueScalarDynamic_SafeZoneValue::RestoreToInitial()
{
	Super::RestoreToInitial();
	SSafeZone::SetGlobalSafeZoneScale(TOptional<float>(InitialValue));
}


#undef LOCTEXT_NAMESPACE
