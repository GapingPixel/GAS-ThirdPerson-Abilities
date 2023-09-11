#include "AbilitySystem/Abilities/GA_Climb.h"

#include "GameFramework/Character.h"
#include "AbilitySystemComponent.h"
#include "ActionGameCharacter.h"
#include "ActorComponents/AG_CharacterMovementComponent.h"

UGA_Climb::UGA_Climb()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

bool UGA_Climb::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	UE_LOG(LogTemp, Warning, TEXT("pass 0"));
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}
	UE_LOG(LogTemp, Warning, TEXT("pass 1"));
	
	return true;
}

void UGA_Climb::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	UE_LOG(LogTemp, Warning, TEXT("pass 2"));
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	const AActionGameCharacter* Character = CastChecked<AActionGameCharacter>(ActorInfo->AvatarActor.Get());
	Character->AGCharacterMovementComponent->TryClimbing();
	/*ACharacter* Character = CastChecked<ACharacter>(ActorInfo->AvatarActor.Get());
	Character->Crouch();*/
	UE_LOG(LogTemp, Warning, TEXT("pass 2.5"));
	
}

void UGA_Climb::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	const AActionGameCharacter* Character = CastChecked<AActionGameCharacter>(ActorInfo->AvatarActor.Get());// CastChecked<ACharacter>(ActorInfo->AvatarActor.Get());
	Character->AGCharacterMovementComponent->CancelClimbing();
	UE_LOG(LogTemp, Warning, TEXT("pass 3"));
	/*ACharacter* Character = CastChecked<ACharacter>(ActorInfo->AvatarActor.Get());
	Character->UnCrouch();*/
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
