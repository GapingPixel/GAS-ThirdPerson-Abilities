// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerControllers/ActionGamePlayerController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "ActionGameGameMode.h"

void AActionGamePlayerController::RestartPlayerIn(float InTime)
{
	ChangeState(NAME_Spectating);

	GetWorld()->GetTimerManager().SetTimer(RestartPlayerTimerHandle, this, &AActionGamePlayerController::RestartPlayer, InTime, false);
}

void AActionGamePlayerController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);

	if (UAbilitySystemComponent* AbilityComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(aPawn))
	{
		DeathStateTagDelegate = AbilityComponent->RegisterGameplayTagEvent(FGameplayTag::RequestGameplayTag(TEXT("State.Dead")), EGameplayTagEventType::NewOrRemoved).AddUObject(this, &AActionGamePlayerController::OnPawnDeathStateChanged);
	}
}

void AActionGamePlayerController::OnUnPossess()
{
	Super::OnUnPossess();

	if (DeathStateTagDelegate.IsValid())
	{
		if (UAbilitySystemComponent* AbilityComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn()))
		{
			AbilityComponent->UnregisterGameplayTagEvent(DeathStateTagDelegate, FGameplayTag::RequestGameplayTag(TEXT("State.Dead")), EGameplayTagEventType::NewOrRemoved);
		}
	}
}

void AActionGamePlayerController::OnPawnDeathStateChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	if (NewCount > 0)
	{
		UWorld* World = GetWorld();

		AActionGameGameMode* GameMode = World ? Cast<AActionGameGameMode>(World->GetAuthGameMode()) : nullptr;

		if (GameMode)
		{
			GameMode->NotifyPlayerDied(this);
		}

		if (DeathStateTagDelegate.IsValid())
		{
			if (UAbilitySystemComponent* AbilityComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn()))
			{
				AbilityComponent->UnregisterGameplayTagEvent(DeathStateTagDelegate, FGameplayTag::RequestGameplayTag(TEXT("State.Dead")), EGameplayTagEventType::NewOrRemoved);
			}
		}
	}
}

void AActionGamePlayerController::RestartPlayer()
{
	UWorld* World = GetWorld();

	AActionGameGameMode* GameMode = World ? Cast<AActionGameGameMode>(World->GetAuthGameMode()) : nullptr;

	if (GameMode)
	{
		GameMode->RestartPlayer(this);
	}
}