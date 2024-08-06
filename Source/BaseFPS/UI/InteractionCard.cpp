// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InteractionCard.h"

#include "BaseFPSHUDLayout.h"
#include "CommonTextBlock.h"
#include "Character/BaseFPSCharacter.h"
#include "Components/Image.h"
#include "Components/InteractableComponent.h"
#include "Player/BaseFPSPlayerController.h"

static FName TextureParam = "TextureMask";

FText UInteractionCard::BuildFormattedDisplayText(const FText* InteractKeyName, const UInteractableComponent* Interactable)
{
	FFormatNamedArguments Args;
	Args.Add("InputEvent", Interactable->GetInteractableInputEventText());
	Args.Add("InputKey", *InteractKeyName);
	Args.Add("InteractionVerb", Interactable->GetInteractionVerbText());
	Args.Add("InteractableName", Interactable->GetInteractableNameText());

	return FText::Format(
			NSLOCTEXT("BaseFPS", "InteractionCardFormat", "{InputEvent} {InputKey} to {InteractionVerb} {InteractableName}"),
			Args);
}

void UInteractionCard::ShowPromptForInteractable(const FText* InteractKeyName, const UInteractableComponent* Interactable)
{
	if (Interactable)
	{
		if (UTexture2D* Texture = Interactable->GetInteractableIcon())
		{
			InteractableIcon->GetDynamicMaterial()->SetTextureParameterValue(TextureParam, Texture);
			InteractableIcon->SetVisibility(ESlateVisibility::HitTestInvisible);

			// setting icon size
			const float Ratio = (float)Texture->GetSizeX() / (float)Texture->GetSizeY();
			const int32 Width = FMath::Min(Ratio * DesiredIconHeight, MaxIconWidth);
			InteractableIcon->Brush.SetImageSize(FVector2d(Width, DesiredIconHeight));
		}
		else
		{
			InteractableIcon->SetVisibility(ESlateVisibility::Hidden);
		}

		FText FormattedText = BuildFormattedDisplayText(InteractKeyName, Interactable);
		InteractableText->SetText(FormattedText);
	}
}

