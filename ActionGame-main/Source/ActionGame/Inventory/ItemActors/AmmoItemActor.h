// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/ItemActor.h"
#include "AmmoItemActor.generated.h"

UCLASS()
class ACTIONGAME_API AAmmoItemActor : public AItemActor
{
	GENERATED_BODY()
	
public:

	const UAmmoItemStaticData* GetAmmoItemStaticData() const;

protected:

	UPROPERTY()
	UMeshComponent* MeshComponent = nullptr;

	virtual void InitInternal() override;

};
