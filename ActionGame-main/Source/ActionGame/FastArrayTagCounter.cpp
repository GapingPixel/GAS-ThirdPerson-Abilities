// Fill out your copyright notice in the Description page of Project Settings.


#include "FastArrayTagCounter.h"

int32 FFastArrayTagCounter::GetTagCount(FGameplayTag InTag) const
{
	for (auto ItemIter = TagArray.CreateConstIterator(); ItemIter; ++ItemIter)
	{
		const FFastArrayTagCounterRecord& TagRecord = *ItemIter;
		if (TagRecord.Tag == InTag)
		{
			return TagRecord.Count;
		}
	}

	return 0;
}

void FFastArrayTagCounter::AddTagCount(FGameplayTag InTag, int32 Delta)
{
	for (auto ItemIter = TagArray.CreateIterator(); ItemIter; ++ItemIter)
	{
		FFastArrayTagCounterRecord& TagRecord = *ItemIter;
		if (TagRecord.Tag == InTag)
		{
			TagRecord.Count += Delta;

			if (TagRecord.Count <= 0)
			{
				ItemIter.RemoveCurrent();
				MarkArrayDirty();
			}
			else
			{
				MarkItemDirty(TagRecord);
			}

			return;
		}
	}

	FFastArrayTagCounterRecord& Item = TagArray.AddDefaulted_GetRef();
	Item.Count = Delta;
	Item.Tag = InTag;
	MarkItemDirty(Item);
}

const TArray<FFastArrayTagCounterRecord>& FFastArrayTagCounter::GetTagArray() const
{
	return TagArray;
}