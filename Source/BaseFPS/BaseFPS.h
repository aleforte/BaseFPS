// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine.h"
#include "NativeGameplayTags.h"

#define COLLISION_INTERACTABLE ECC_GameTraceChannel1
#define COLLISION_TRACE_WEAPON ECC_GameTraceChannel3

BASEFPS_API DECLARE_LOG_CATEGORY_EXTERN(LogBaseFPS, Log, All);

/**
 * Front-end labels used by the BaseGameUIPolicy (see Blueprint)
 */
namespace FrontendTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_UI_LAYER_GAME);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_UI_LAYER_GAME_MENU);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_UI_LAYER_MENU);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_UI_LAYER_MODAL);
}

/** handy response params for world-only checks */
extern FCollisionResponseParams WorldResponseParams;

/************************************************************************/
/* Mesh Helpers                                                         */
/************************************************************************/

extern void RemoveComponentTree(USceneComponent* Comp);
extern void SetVisibilityForMeshTree(UMeshComponent* Mesh, bool bIsVisible);

/************************************************************************/
/* Animation Helps                                                      */
/************************************************************************/

extern float GetScaledAnimDuration(UAnimMontage* AnimMontage);
