#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Octave.h"
#include "BlockType.h"
#include "Sector.generated.h"

class AChunk;

UCLASS()
class BLOCKYADVENTURE_API ASector : public AActor
{
	GENERATED_BODY()
	
public:	
	ASector();

	/**
	 * Material used for blocks.
	 */
	UPROPERTY(EditInstanceOnly, Category = "Sector")
	TObjectPtr<UMaterialInterface> Material;

	/**
	 * Octaves used for generating height map.
	 */
	UPROPERTY(EditInstanceOnly, Category = "Sector")
	TArray<FOctave> Octaves;

	/**
	 * Number of chunks in X and Y dimensions.
	 */
	inline static constexpr int32 SIZE{ 8 };

	/**
	 * Generate terrain and create mesh for the sector.
	 */
	UFUNCTION(CallInEditor, Category = "Sector")
	void Create();

	/**
	 * Destroy all the chunks.
	 */
	UFUNCTION(CallInEditor, Category = "Sector")
	void Clear();

	/**
	 * Generate terrain for each chunk within this sector.
	 */
	void Generate();

	/**
	 * Create mesh for each chunk within this sector.
	 * 
	 */
	void CreateMesh();

	/**
	 * Determine if a block at specified position, local to this sector, is an air (empty) block.  Blocks outside of
	 * this sector are also considered as air.
	 */
	bool IsBlockAir(const FIntVector& BlockPosition) const;

	/**
	 * Compute height for block at a specified XY position, local to this sector.
	 */
	int32 ComputeHeight(const FIntVector2& BlockPosition) const;

	/**
	 * Get position of a block, local to this sector, from the world position.
	 */
	FIntVector GetBlockPosition(const FVector& WorldPosition) const;

	/**
	 * Set a block at a specified position, local to this sector, into a block of a type with a specified id.
	 */
	void SetBlock(const FIntVector& BlockPosition, BlockTypeID ID);

	/**
	 * Get chunk to which a block with a specified position, local to this sector, belongs.
	 */
	AChunk* GetChunk(const FIntVector& BlockPosition) const;

	/**
	 * Determine if a block at specified position, local to this sector, is in sector bounds.
	 */
	bool IsBlockInBounds(const FIntVector& BlockPosition) const;
protected:
	virtual void BeginPlay() override;

private:
	/**
	 * Contains all chunks, which belong to this sector.
	 */
	TArray<AChunk*> Chunks;

	/**
	 * Get a position, local to a specified chunk, from a specified block position, local to this sector.
	 */
	FIntVector GetInChunkBlockPosition(const AChunk* Chunk, const FIntVector& BlockPosition) const;
};
