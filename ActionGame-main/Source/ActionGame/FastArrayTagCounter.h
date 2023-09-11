// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ActionGameTypes.h"
#include "FastArrayTagCounter.generated.h"

USTRUCT(BlueprintType)
struct FFastArrayTagCounterRecord : public FFastArraySerializerItem
{
	GENERATED_BODY()

public:

	UPROPERTY()
	FGameplayTag Tag;

	UPROPERTY()
	int32 Count;
};

USTRUCT(BlueprintType)
struct FFastArrayTagCounter : public FFastArraySerializer
{
	GENERATED_BODY()

public:

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FFastArrayTagCounterRecord, FFastArrayTagCounter>(TagArray, DeltaParams, *this);
	}

	int32 GetTagCount(FGameplayTag InTag) const;

	void AddTagCount(FGameplayTag InTag, int32 Delta);

	const TArray<FFastArrayTagCounterRecord>& GetTagArray() const;

protected:
	
	UPROPERTY()
	TArray<FFastArrayTagCounterRecord> TagArray;
};

template<>
struct TStructOpsTypeTraits<FFastArrayTagCounter> : public TStructOpsTypeTraitsBase2<FFastArrayTagCounter>
{
	enum { WithNetDeltaSerializer = true };
};