#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BlockType.h"
#include "GameWorld.generated.h"

class ASector;
class AChunk;
struct FOctave;

/**
 * Represent a game world. Game world is composed out of sectors. Each sector could be loaded or unloaded during
 * runtime.
 */
UCLASS()
class BLOCKYADVENTURE_API AGameWorld final : public AActor
{
	GENERATED_BODY()
	
public:	
	AGameWorld();

	/**
	 * Material used for blocks.
	 */
	UPROPERTY(EditAnywhere, Category = "Game World")
	TObjectPtr<UMaterialInterface> Material;

	/**
	 * Octaves used for generating height map.
	 */
	UPROPERTY(EditAnywhere, Category = "Terrain Generation")
	TArray<FOctave> Octaves;

	/**
	 * Get a sector of a specified block position. Block position must be in the bounds of any loaded sector.
	 */
	TObjectPtr<ASector> GetSector(const FIntVector& BlockPosition);

	/**
	 * Get a chunk of a specified block position. Block position must be in the bounds of any loaded sector.
	 */
	TObjectPtr<AChunk> GetChunk(const FIntVector& BlockPosition);

	/**
	 * Get a reference to a block at specified position. Specified position must be in the bounds of any loaded sector.
	 */
	BlockTypeID& GetBlock(const FIntVector& BlockPosition);

	/**
	 * Determine if a block at a specified position is an air (empty block). Blocks which are not within bounds of any
	 * loaded sector are consider as air (empty) blocks.
	 */
	bool IsBlockAir(const FIntVector& BlockPosition);

	/**
	 * Determine if block is in the bounds of any loaded sector.
	 */
	bool IsBlockInBounds(const FIntVector& BlockPosition) const;

	/**
	 * Compute height for a block at a specified XY position.
	 */
	int32 ComputeHeight(const FIntVector2& BlockPosition) const;

	/**
	 * Compute position of a block from a arbitary position in the world.
	 */
	FIntVector GetBlockPosition(const FVector& WorldPosition) const;

	/**
	 * Spawn a sector which contains a block at specified position.
	 */
	void SpawnSector(const FIntVector& BlockPosition);

	/**
	 * Despawn a sector which contains a block at specified position.
	 */
	void DespawnSector(const FIntVector& BlockPosition);
protected:
	virtual void BeginPlay() override;

private:
	/**
	 * Contains currently loaded sectors.
	 */
	TArray<TObjectPtr<ASector>> Sectors;

	/**
	 * Convert position of a block to a position of a sector.
	 */
	FIntVector ConvertBlockPositionToSectorPosition(const FIntVector& BlockPosition) const;
};
