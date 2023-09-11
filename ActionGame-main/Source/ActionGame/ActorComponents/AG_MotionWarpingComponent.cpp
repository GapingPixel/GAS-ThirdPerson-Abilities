// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponents/AG_MotionWarpingComponent.h"


UAG_MotionWarpingComponent::UAG_MotionWarpingComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
}

// Deprecated 5.1.1+
/*
void UAG_MotionWarpingComponent::SendWarpPointsToClients()
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		TArray<FMotionWarpingTargetByLocationAndRotation> WarpTargets;

		for (auto WarpTarget : WarpTargetMap)
		{
			FMotionWarpingTargetByLocationAndRotation MotionWarpingTarget(WarpTarget.Key, WarpTarget.Value.GetLocation(), WarpTarget.Value.GetRotation());

			WarpTargets.Add(MotionWarpingTarget);
		}

		MulticastSyncWarpPoints(WarpTargets);
	}
}

void UAG_MotionWarpingComponent::MulticastSyncWarpPoints_Implementation(const TArray<FMotionWarpingTargetByLocationAndRotation>& Targets)
{
	for (const FMotionWarpingTargetByLocationAndRotation& Target : Targets)
	{
		AddOrUpdateWarpTargetFromLocationAndRotation(Target.Name, Target.Location, FRotator(Target.Rotation));
	}
}
*/