#pragma once

#include "CoreMinimal.h"
#include "BlockType.h"

class AChunk;
class ASector;
class AGameWorld;
class AGameWorld;

/**
 * Represent a pointer to a block.
 */
struct BLOCKYADVENTURE_API FBlockPtr
{
public:
	FBlockPtr(
		BlockTypeID* const InTypeID, 
		const FIntVector& InPosition,
		AChunk* const InChunk, 
		ASector* const InSector, 
		AGameWorld* const InGameWorld
	);

	FBlockPtr() {};


	/**
	 * Get the block position of this block.
	 */
	FIntVector GetPosition() const { return Position; }

	/**
	 * Get the chunk to which this block belongs.
	 */
	AChunk* GetChunk() const { return Chunk; }

	/**
	 * Get the sector to which this block belongs.
	 */
	ASector* GetSector() const { return Sector; }

	/**
	 * Get the game world to which this block belongs.
	 */
	AGameWorld* GetGameWorld() const { return GameWorld; }

	/**
	 * Determine if this block pointer is valid.
	 */
	bool IsValid() const { return bIsValid && TypeID != nullptr; }

	/**
	 * Determine if this block is an air (empty) block.
	 */
	bool IsAir() const { return *TypeID == FBlockType::AIR_ID; }

	/**
	 * Get ID of the block type of this block.
	 */
	BlockTypeID GetBlockTypeID() const { return *TypeID; }

	/**
	 * Set this block to a block type of a specified ID.
	 */
	void SetBlock(BlockTypeID ID) { *TypeID = ID; }

	/**
	 * Set this block to a block type of a specified ID and then recreate and cook the mesh of the chunk to which this
	 * block belongs.
	 * 
	 * \param bSaveSector Determine if owning sector should be saved to the file after setting this block.
	 * \param bUseAsyncCooking Determine if the mesh should by cooked asynchrously.
	 */
	void SetAndUpdate(const BlockTypeID ID, const bool bSaveSector = true, const bool bUseAsyncCooking = false);


	/**
	 * Set this block to a block type of a specified ID.
	 */
	void operator=(const BlockTypeID ID) { SetBlock(ID); }

private:
	/**
	 * Position of this block.
	 */
	FIntVector Position{};
	/**
	 * Chunk to which this block belongs.
	 */
	AChunk* Chunk{};
	/**
	 * Sector to which this block belongs.
	 */
	ASector* Sector{};
	/**
	 * Game world to which this block belongs.
	 */
	AGameWorld* GameWorld{};
	/**
	* Determine if this block pointer is valid.
	*/
	bool bIsValid{ false };
	/**
	* ID of the block type of this block.
	*/
	BlockTypeID* TypeID{};
};
