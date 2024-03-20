#pragma once

#include "CoreMinimal.h"

/**
 * Numerical type used as identificator for block types.
 */
using BlockTypeID = uint8;

/**
 * Represent a type of the block.
 */
struct BLOCKYADVENTURE_API FBlockType
{
	/**
	 * Color of the block.
	 */
	const FColor Color;
	/**
	 * Numerical identificator of the block.
	 */
	const BlockTypeID ID;

	static const FBlockType Stone;
	static const FBlockType Dirt;

	/**
	 * Get block type with specified ID.
	 */
	static const FBlockType& FromID(BlockTypeID ID) { return BlockTypes[ID]; }

private:
	FBlockType(FColor Color)
		: ID{ static_cast<BlockTypeID>(BlockTypes.Num()) }, Color{ Color }
	{
		BlockTypes.Add(*this);
	}

	/**
	 * Contains all created block types.
	 */
	inline static TArray<FBlockType> BlockTypes;
};

const FBlockType FBlockType::Stone{ FColor{ 70, 70, 70 } };
const FBlockType FBlockType::Dirt{ FColor{ 128, 79, 45 } };