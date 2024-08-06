// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/BaseFPSCharacterMovement.h"

void UBaseFPSCharacterMovement::SetReplicatedAcceleration(FRotator MovementRotation, uint8 CompressedAccel)
{
	FVector CurrentDir = MovementRotation.Vector();
	FVector SideDir = (CurrentDir ^ FVector::UpVector).GetSafeNormal();

	FVector AccelDir(0.f);
	if (CompressedAccel & 1)
	{
		AccelDir += CurrentDir;
	}
	else if (CompressedAccel & 2)
	{
		AccelDir -= CurrentDir;
	}
	if (CompressedAccel & 4)
	{
		AccelDir += SideDir;
	}
	else if (CompressedAccel & 8)
	{
		AccelDir -= SideDir;
	}

	Acceleration = GetMaxAcceleration() * AccelDir.GetSafeNormal();
}
