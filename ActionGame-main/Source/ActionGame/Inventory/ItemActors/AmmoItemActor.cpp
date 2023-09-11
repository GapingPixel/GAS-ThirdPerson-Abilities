// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/ItemActors/AmmoItemActor.h"

#include "Inventory/InventoryItemInstance.h"
#include "ActionGameTypes.h"

const UAmmoItemStaticData* AAmmoItemActor::GetAmmoItemStaticData() const
{
	return ItemInstance ? Cast<UAmmoItemStaticData>(ItemInstance->GetItemStaticData()) : nullptr;
}

void AAmmoItemActor::InitInternal()
{
	Super::InitInternal();

	if (const UAmmoItemStaticData* WeaponData = GetAmmoItemStaticData())
	{
		if (WeaponData->StaticMesh)
		{
			UStaticMeshComponent* StaticComp = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass(), TEXT("MeshComponent"));
			StaticComp->RegisterComponent();
			StaticComp->SetStaticMesh(WeaponData->StaticMesh);
			StaticComp->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);

			MeshComponent = StaticComp;
		}
	}
}