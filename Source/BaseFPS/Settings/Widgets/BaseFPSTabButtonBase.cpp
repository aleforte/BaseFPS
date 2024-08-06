// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseFPSTabButtonBase.h"
#include "CommonLazyImage.h"

void UBaseFPSTabButtonBase::SetIconFromLazyObject(TSoftObjectPtr<> LazyObject)
{
	if (LazyImage_Icon)
	{
		LazyImage_Icon->SetBrushFromLazyDisplayAsset(LazyObject);
	}
}

void UBaseFPSTabButtonBase::SetIconBrush(const FSlateBrush& Brush)
{
	if (LazyImage_Icon)
	{
		LazyImage_Icon->SetBrush(Brush);
	}
}

void UBaseFPSTabButtonBase::SetTabLabelInfo_Implementation(const FTabDescriptor& TabLabelInfo)
{
	SetButtonText(TabLabelInfo.TabText);
	SetIconBrush(TabLabelInfo.IconBrush);
}
