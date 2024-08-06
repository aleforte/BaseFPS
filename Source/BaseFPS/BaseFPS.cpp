// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseFPS.h"

DEFINE_LOG_CATEGORY(LogBaseFPS)

namespace FrontendTags
{
	UE_DEFINE_GAMEPLAY_TAG( TAG_UI_LAYER_GAME, "UI.Layer.Game" );
	UE_DEFINE_GAMEPLAY_TAG( TAG_UI_LAYER_GAME_MENU, "UI.Layer.GameMenu" );
	UE_DEFINE_GAMEPLAY_TAG( TAG_UI_LAYER_MENU, "UI.Layer.Menu" );
	UE_DEFINE_GAMEPLAY_TAG( TAG_UI_LAYER_MODAL, "UI.Layer.Modal" );
}

FCollisionResponseParams WorldResponseParams = []()
{
	FCollisionResponseParams Result(ECR_Overlap);
	Result.CollisionResponse.WorldStatic = ECR_Block;
	Result.CollisionResponse.WorldDynamic = ECR_Block;
	return Result;
}();

void RemoveComponentTree(USceneComponent* Comp)
{
	if (Comp)
	{
		Comp->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		
		// Checking for registration (when an actor is modified, the editor reloads the actor and unregisters all
		// components in the process, causing our runtime components to be unregistered when this function is called) 
		if (Comp->IsRegistered())
		{
			Comp->UnregisterComponent();	
		}

		TArray<USceneComponent*> Children;
		Comp->GetChildrenComponents(true, Children);
		for (USceneComponent* Child : Children)
		{
			// ditto here
			if (Child->IsRegistered())
			{
				Child->UnregisterComponent();			
			}
			Child->DestroyComponent(false);
		}
		Comp->DestroyComponent(false);
	}
}

void SetVisibilityForMeshTree(UMeshComponent* Mesh, bool bIsVisible)
{
	Mesh->SetHiddenInGame(!bIsVisible);

	TArray<USceneComponent*> Children;
	Mesh->GetChildrenComponents(true, Children);
	for (USceneComponent* Child : Children)
	{
		if (UMeshComponent* ChildMesh = Cast<UMeshComponent>(Child))
		{
			SetVisibilityForMeshTree(ChildMesh, bIsVisible);
		}
	}
}

float GetScaledAnimDuration(UAnimMontage* AnimMontage)
{
	return AnimMontage ? ( AnimMontage->GetPlayLength() / AnimMontage->RateScale ) : 0.f;
}
