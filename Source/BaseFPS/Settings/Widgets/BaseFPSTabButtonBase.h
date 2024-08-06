// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseFPSTabListWidgetBase.h"
#include "UI/Foundation/BaseFPSButtonBase.h"
#include "BaseFPSTabButtonBase.generated.h"

class UCommonLazyImage;
class UObject;
struct FFrame;
struct FSlateBrush;

UCLASS(Abstract, Blueprintable, meta = (DisableNativeTick))
class BASEFPS_API UBaseFPSTabButtonBase : public UBaseFPSButtonBase, public IBaseFPSTabButtonInterface
{
	GENERATED_BODY()

public:
	void SetIconFromLazyObject(TSoftObjectPtr<UObject> LazyObject);
	void SetIconBrush(const FSlateBrush& Brush);

protected:

	UFUNCTION()
	virtual void SetTabLabelInfo_Implementation(const FTabDescriptor& TabLabelInfo) override;

private:

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonLazyImage> LazyImage_Icon;
};
