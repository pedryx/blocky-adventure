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
	 * ID of empty block (the air block).
	 */
	inline static constexpr BlockTypeID AIR_ID{ 0 };

	/**
	 * Get block type with specified ID.
	 */
	static const FBlockType& FromID(BlockTypeID ID)
	{
		checkf(ID <= BlockTypes.Num(), TEXT("Invalid block type ID"));
		checkf(ID != AIR_ID, TEXT("Cannot get block type of empty (air) block"));

		// ID 0 is reserved for the air.
		return BlockTypes[ID - 1];
	}

private:
	FBlockType(FColor Color)
		// ID 0 is reserved for the air.
		: Color{ Color }, ID{ static_cast<BlockTypeID>(BlockTypes.Num() + 1) }
	{
		BlockTypes.Add(*this);
	}

	/**
	 * Contains all created block types.
	 */
	inline static TArray<FBlockType> BlockTypes;
};