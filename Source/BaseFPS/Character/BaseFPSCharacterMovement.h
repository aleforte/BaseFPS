// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BaseFPSCharacterMovement.generated.h"

/**
 * 
 */
UCLASS()
class BASEFPS_API UBaseFPSCharacterMovement : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:

	virtual void SetReplicatedAcceleration(FRotator MovementRotation, uint8 CompressedAccel);
};
