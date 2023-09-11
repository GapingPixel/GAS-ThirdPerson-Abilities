// Fill out your copyright notice in the Description page of Project Settings.


#include "Volumes/AbilitySystemPhysicsVolume.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "DrawDebugHelpers.h"

AAbilitySystemPhysicsVolume::AAbilitySystemPhysicsVolume()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AAbilitySystemPhysicsVolume::ActorEnteredVolume(class AActor* Other)
{
	Super::ActorEnteredVolume(Other);

	if (!HasAuthority()) return;

	if (UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Other))
	{
		for (auto Ability : PermanentAbilitiesToGive)
		{
			AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(Ability));
		}

		EnteredActorsInfoMap.Add(Other);

		for (auto Ability : OngoingAbilitiesToGive)
		{
			FGameplayAbilitySpecHandle AbilityHandle = AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(Ability));

			EnteredActorsInfoMap[Other].AppliedAbilities.Add(AbilityHandle);
		}

		for (auto GameplayEffect : OngoingEffectsToApply)
		{
			FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();

			EffectContext.AddInstigator(Other, Other);

			FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(GameplayEffect, 1, EffectContext);
			if (SpecHandle.IsValid())
			{
				FActiveGameplayEffectHandle ActiveGEHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
				if (ActiveGEHandle.WasSuccessfullyApplied())
				{
					EnteredActorsInfoMap[Other].AppliedEffects.Add(ActiveGEHandle);
				}
			}
		}

		for (auto EventTag : GameplayEventsToSendOnEnter)
		{
			FGameplayEventData EventPayload;
			EventPayload.EventTag = EventTag;

			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Other, EventTag, EventPayload);
		}
	}
}

void AAbilitySystemPhysicsVolume::ActorLeavingVolume(class AActor* Other)
{
	Super::ActorLeavingVolume(Other);

	if (!HasAuthority()) return;

	if (UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Other))
	{
		if (EnteredActorsInfoMap.Find(Other))
		{
			for (auto GameplayEffectHandle : EnteredActorsInfoMap[Other].AppliedEffects)
			{
				AbilitySystemComponent->RemoveActiveGameplayEffect(GameplayEffectHandle);
			}

			for (auto GameplayAbilityHandle : EnteredActorsInfoMap[Other].AppliedAbilities)
			{
				AbilitySystemComponent->ClearAbility(GameplayAbilityHandle);
			}

			EnteredActorsInfoMap.Remove(Other);
		}

		for (auto GameplayEffect : OnExitEffectsToApply)
		{
			FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();

			EffectContext.AddInstigator(Other, Other);

			FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(GameplayEffect, 1, EffectContext);
			if (SpecHandle.IsValid())
			{
				FActiveGameplayEffectHandle ActiveGEHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}

		for (auto EventTag : GameplayEventsToSendOnExit)
		{
			FGameplayEventData EventPayload;
			EventPayload.EventTag = EventTag;

			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Other, EventTag, EventPayload);
		}
	}
}

void AAbilitySystemPhysicsVolume::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bDrawDebug)
	{
		DrawDebugBox(GetWorld(), GetActorLocation(), GetBounds().BoxExtent, FColor::Red, false, 0, 0, 5);
	}
}