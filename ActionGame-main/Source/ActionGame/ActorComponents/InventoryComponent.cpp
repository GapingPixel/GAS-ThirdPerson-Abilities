// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponents/InventoryComponent.h"
#include "Net/UnrealNetwork.h"

#include "Inventory/InventoryList.h"
#include "Inventory/InventoryItemInstance.h"

#include "Abilities/GameplayAbilityTypes.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Actors/ItemActor.h"

#include "GameplayTagsManager.h"

#include "Engine/ActorChannel.h"

#include "AbilitySystemLog.h"

FGameplayTag UInventoryComponent::EquipItemActorTag;
FGameplayTag UInventoryComponent::DropItemTag;
FGameplayTag UInventoryComponent::EquipNextTag;
FGameplayTag UInventoryComponent::UnequipTag;

static TAutoConsoleVariable<int32> CVarShowInventory(
	TEXT("ShowDebugInventory"),
	0,
	TEXT("Draws debug info about inventory")
	TEXT(" 0: off/n")
	TEXT(" 1: on/n"),
	ECVF_Cheat
);

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bWantsInitializeComponent = true;
	SetIsReplicatedByDefault(true);

	static bool bHandledAddingTags = false;
	if (!bHandledAddingTags)
	{
		bHandledAddingTags = true;
		UGameplayTagsManager::Get().OnLastChanceToAddNativeTags().AddUObject(this, &UInventoryComponent::AddInventoryTags);
	}
}

int32 UInventoryComponent::GetInventoryTagCount(FGameplayTag Tag) const
{
	return InventoryTags.GetTagCount(Tag);
}

void UInventoryComponent::AddInventoryTagCount(FGameplayTag InTag, int32 CountDelta)
{
	InventoryTags.AddTagCount(InTag, CountDelta);
}

void UInventoryComponent::AddInventoryTags()
{
	UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();

	UInventoryComponent::EquipItemActorTag = TagsManager.AddNativeGameplayTag(TEXT("Event.Inventory.EquipItemActor"), TEXT("Equip item from item actor event"));
	UInventoryComponent::DropItemTag = TagsManager.AddNativeGameplayTag(TEXT("Event.Inventory.DropItem"), TEXT("Drop equipped item"));
	UInventoryComponent::EquipNextTag = TagsManager.AddNativeGameplayTag(TEXT("Event.Inventory.EquipNext"), TEXT("Try equip next item"));
	UInventoryComponent::UnequipTag = TagsManager.AddNativeGameplayTag(TEXT("Event.Inventory.Unequip"), TEXT("Unequip current item"));

	TagsManager.OnLastChanceToAddNativeTags().RemoveAll(this);
}

void UInventoryComponent::InitializeComponent()
{
	Super::InitializeComponent();

	if (GetOwner()->HasAuthority())
	{
		for (auto ItemClass : DefaultItems)
		{
			InventoryList.AddItem(ItemClass);
		}
	}

	if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner()))
	{
		ASC->GenericGameplayEventCallbacks.FindOrAdd(UInventoryComponent::EquipItemActorTag).AddUObject(this, &UInventoryComponent::GameplayEventCallback);
		ASC->GenericGameplayEventCallbacks.FindOrAdd(UInventoryComponent::DropItemTag).AddUObject(this, &UInventoryComponent::GameplayEventCallback);
		ASC->GenericGameplayEventCallbacks.FindOrAdd(UInventoryComponent::EquipNextTag).AddUObject(this, &UInventoryComponent::GameplayEventCallback);
		ASC->GenericGameplayEventCallbacks.FindOrAdd(UInventoryComponent::UnequipTag).AddUObject(this, &UInventoryComponent::GameplayEventCallback);

	}
}

void UInventoryComponent::AddItem(TSubclassOf<UItemStaticData> InItemStaticDataClass)
{
	if (GetOwner()->HasAuthority())
	{
		InventoryList.AddItem(InItemStaticDataClass);
	}
}

void UInventoryComponent::AddItemInstance(UInventoryItemInstance* InItemInstance)
{
	if (GetOwner()->HasAuthority())
	{
		TArray<UInventoryItemInstance*> Items = InventoryList.GetAllAvailableInstancesOfType(InItemInstance->ItemStaticDataClass);

		Algo::Sort(Items, [](UInventoryItemInstance* InA, UInventoryItemInstance* InB)
		{
			return InA->GetQuantity() < InB->GetQuantity();
		}
		);

		const int32 MaxItemStackCount = InItemInstance->GetItemStaticData()->MaxStackCount;

		int32 ItemsLeft = InItemInstance->GetQuantity();

		for (auto Item : Items)
		{
			const int32 EmptySlots = MaxItemStackCount - Item->GetQuantity();

			int32 SlotsToAdd = ItemsLeft;

			if (ItemsLeft > EmptySlots)
			{
				SlotsToAdd = EmptySlots;
			}

			ItemsLeft -= SlotsToAdd;

			Item->AddItems(SlotsToAdd);
			InItemInstance->AddItems(-SlotsToAdd);

			for (FGameplayTag InvTag : Item->GetItemStaticData()->InventoryTags)
			{
				InventoryTags.AddTagCount(InvTag, SlotsToAdd);
			}

			if (ItemsLeft <= 0)
			{
				ItemsLeft = 0;

				return;
			}
		}

		while (ItemsLeft > MaxItemStackCount)
		{
			AddItem(InItemInstance->GetItemStaticData()->GetClass());

			for (FGameplayTag InvTag : InItemInstance->GetItemStaticData()->InventoryTags)
			{
				InventoryTags.AddTagCount(InvTag, MaxItemStackCount);
			}

			ItemsLeft -= MaxItemStackCount;
			InItemInstance->AddItems(-MaxItemStackCount);
		}

		InventoryList.AddItem(InItemInstance);

		for (FGameplayTag InvTag : InItemInstance->GetItemStaticData()->InventoryTags)
		{
			InventoryTags.AddTagCount(InvTag, InItemInstance->GetQuantity());
		}

	}
}

void UInventoryComponent::RemoveItem(TSubclassOf<UItemStaticData> InItemStaticDataClass)
{
	if (GetOwner()->HasAuthority())
	{
		InventoryList.RemoveItem(InItemStaticDataClass);
	}
}

void UInventoryComponent::RemoveItemInstance(UInventoryItemInstance* InItemInstance)
{
	if (GetOwner()->HasAuthority())
	{
		InventoryList.RemoveItem(InItemInstance);

		for (FGameplayTag InvTag : InItemInstance->GetItemStaticData()->InventoryTags)
		{
			InventoryTags.AddTagCount(InvTag, -InItemInstance->GetQuantity());
		}
	}
}

void UInventoryComponent::RemoveItemWithInventoryTag(FGameplayTag Tag, int32 Count)
{
	if (GetOwner()->HasAuthority())
	{
		int32 CountLeft = Count;

		TArray<UInventoryItemInstance*> Items = GetAllInstancesWithTag(Tag);

		Algo::Sort(Items, [](UInventoryItemInstance* InA, UInventoryItemInstance* InB)
		{
			return InA->GetQuantity() < InB->GetQuantity();
		}
		);

		for (auto Item : Items)
		{
			int32 AvailableCount = Item->GetQuantity();
			int32 ItemsToRemove = CountLeft;

			if (ItemsToRemove >= AvailableCount)
			{
				ItemsToRemove = AvailableCount;

				RemoveItemInstance(Item);
			}
			else
			{
				Item->AddItems(-ItemsToRemove);

				for (FGameplayTag InvTag : Item->GetItemStaticData()->InventoryTags)
				{
					InventoryTags.AddTagCount(InvTag, -ItemsToRemove);
				}
			}

			CountLeft -= ItemsToRemove;
		}
	}
}

void UInventoryComponent::EquipItem(TSubclassOf<UItemStaticData> InItemStaticDataClass)
{
	if (GetOwner()->HasAuthority())
	{
		for (auto Item : InventoryList.GetItemsRef())
		{
			if (Item.ItemInstance->ItemStaticDataClass == InItemStaticDataClass)
			{
				Item.ItemInstance->OnEquipped(GetOwner());

				CurrentItem = Item.ItemInstance;

				break;
			}
		}
	}
}

void UInventoryComponent::EquipItemInstance(UInventoryItemInstance* InItemInstance)
{
	if (GetOwner()->HasAuthority())
	{
		for (auto Item : InventoryList.GetItemsRef())
		{
			if (Item.ItemInstance == InItemInstance)
			{
				Item.ItemInstance->OnEquipped(GetOwner());
				CurrentItem = Item.ItemInstance;
				break;
			}
		}
	}
}

void UInventoryComponent::EquipNext()
{
	TArray<FInventoryListItem>& Items = InventoryList.GetItemsRef();

	const bool bNoItems = Items.Num() == 0;
	const bool bOneAndEquipped = Items.Num() == 1 && CurrentItem;

	if(bNoItems || bOneAndEquipped) return;

	UInventoryItemInstance* TargetItem = CurrentItem;

	for (auto Item : Items)
	{
		if (Item.ItemInstance->GetItemStaticData()->bCanBeEquipped)
		{
			if (Item.ItemInstance != CurrentItem)
			{
				TargetItem = Item.ItemInstance;
				break;
			}
		}
	}

	if (CurrentItem)
	{
		if (TargetItem == CurrentItem)
		{
			return;
		}

		UnequipItem();
	}

	EquipItemInstance(TargetItem);
}

void UInventoryComponent::UnequipItem()
{
	if (GetOwner()->HasAuthority())
	{
		if (IsValid(CurrentItem))
		{
			CurrentItem->OnUnequipped(GetOwner());
			CurrentItem = nullptr;
		}
	}
}

void UInventoryComponent::DropItem()
{
	if (GetOwner()->HasAuthority())
	{
		if (IsValid(CurrentItem))
		{
			CurrentItem->OnDropped(GetOwner());
			RemoveItem(CurrentItem->ItemStaticDataClass);
			CurrentItem = nullptr;
		}
	}
}

UInventoryItemInstance* UInventoryComponent::GetEquippedItem() const
{
	return CurrentItem;
}

void UInventoryComponent::GameplayEventCallback(const FGameplayEventData* Payload)
{
	ENetRole NetRole = GetOwnerRole();

	if (NetRole == ROLE_Authority)
	{
		HandleGameplayEventInternal(*Payload);
	}
	else if (NetRole == ROLE_AutonomousProxy)
	{
		ServerHandleGameplayEvent(*Payload);
	}
}

TArray<UInventoryItemInstance*> UInventoryComponent::GetAllInstancesWithTag(FGameplayTag Tag)
{
	TArray<UInventoryItemInstance*> OutInstances;

	OutInstances = InventoryList.GetAllInstancesWithTag(Tag);

	return OutInstances;
}

void UInventoryComponent::HandleGameplayEventInternal(FGameplayEventData Payload)
{
	ENetRole NetRole = GetOwnerRole();

	if (NetRole == ROLE_Authority)
	{
		FGameplayTag EventTag = Payload.EventTag;

		if (EventTag == UInventoryComponent::EquipItemActorTag)
		{
			if (const UInventoryItemInstance* ItemInstance = Cast<UInventoryItemInstance>(Payload.OptionalObject))
			{
				AddItemInstance(const_cast<UInventoryItemInstance*>(ItemInstance));

				if (Payload.Instigator)
				{
					const_cast<AActor*>(Payload.Instigator.Get())->Destroy();
				}
			}
		}
		else if (EventTag == UInventoryComponent::EquipNextTag)
		{
			EquipNext();
		}
		else if (EventTag == UInventoryComponent::DropItemTag)
		{
			DropItem();
		}
		else if (EventTag == UInventoryComponent::UnequipTag)
		{
			UnequipItem();
		}
	}
}

void UInventoryComponent::ServerHandleGameplayEvent_Implementation(FGameplayEventData Payload)
{
	HandleGameplayEventInternal(Payload);
}

bool UInventoryComponent::ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	for (FInventoryListItem& Item : InventoryList.GetItemsRef())
	{
		UInventoryItemInstance* ItemInstance = Item.ItemInstance;

		if (IsValid(ItemInstance))
		{
			WroteSomething |= Channel->ReplicateSubobject(ItemInstance, *Bunch, *RepFlags);
		}
	}

	return WroteSomething;
}

// Called every frame
void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const bool bShowDebug = CVarShowInventory.GetValueOnGameThread() != 0;
	if (bShowDebug)
	{
		for (FInventoryListItem& Item : InventoryList.GetItemsRef())
		{
			UInventoryItemInstance* ItemInstance = Item.ItemInstance;
			const UItemStaticData* ItemStaticData = ItemInstance->GetItemStaticData();

			if (IsValid(ItemInstance) && IsValid(ItemStaticData))
			{
				GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Blue, FString::Printf(TEXT("Ttem: %s"), *ItemStaticData->Name.ToString()));
			}
		}

		const TArray<FFastArrayTagCounterRecord>& InventoryTagArray = InventoryTags.GetTagArray();

		for (auto TagRecord : InventoryTagArray)
		{
			GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Purple, FString::Printf(TEXT("Tag: %s %d"), *TagRecord.Tag.ToString(), TagRecord.Count));
		}
	}
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventoryComponent, InventoryList);
	DOREPLIFETIME(UInventoryComponent, CurrentItem);
	DOREPLIFETIME(UInventoryComponent, InventoryTags);
}