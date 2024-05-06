#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BlockPtr.h"
#include "GameWorld.generated.h"

class ASector;
class AChunk;
struct FOctave;
template<typename ItemType, EQueueMode Mode>
class TQueue;

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
	UMaterialInterface* Material;

	/**
	 * Octaves used for generating height map.
	 */
	UPROPERTY(EditAnywhere, Category = "Terrain Generation")
	TArray<FOctave> Octaves;

	/**
	 * Get a sector of a specified block position. Block position must be within the bounds of any loaded sector.
	 */
	ASector* GetSector(const FIntVector& BlockPosition);

	/**
	 * Get a chunk of a specified block position. Block position must be within the bounds of any loaded sector.
	 */
	AChunk* GetChunk(const FIntVector& BlockPosition);

	/**
	 * Get a pointer to a block at specified position. Specified position must be within the bounds of any loaded
	 * sector.
	 */
	FBlockPtr GetBlock(const FIntVector& BlockPosition);

	/**
	 * Get a pointer to a block at specified position. Specified position must be within the bounds of any loaded
	 * sector.
	 */
	const FBlockPtr GetBlock(const FIntVector& BlockPosition) const;

	/**
	 * Determine if a block at a specified block position is an air (empty) block. Blocks which are not within bounds
	 * of any loaded sector are also considered as air (empty) blocks.
	 */
	bool IsBlockAir(const FIntVector& BlockPosition);

	/**
	 * Determine if block is within the bounds of any loaded sector.
	 */
	bool IsBlockInBounds(const FIntVector& BlockPosition) const;

	/**
	 * Compute height for a block at a specified XY block position.
	 */
	int32 ComputeHeight(const FIntVector2& BlockPosition) const;

	/**
	 * Compute block position of a block from a arbitary position in the world.
	 */
	FIntVector GetBlockPosition(const FVector& WorldPosition) const;

	/**
	 * Spawn a sector which contains a block at specified block position.
	 * 
	 * \param bShouldIgnoreFirstOverlap Determine if first overlap event with sector trigger should be ignored. Note
	 * that this trigger fires when spawned with player in its area. Trigger of this event results of deletion of the
	 * sector.
	 */
	void SpawnSector(const FIntVector& BlockPosition, const bool bShouldIgnoreFirstOverlap = true);

	/**
	 * Despawn a sector which contains a block at specified block position.
	 */
	void DespawnSector(const FIntVector& BlockPosition);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	/**
	 * Contains currently loaded sectors.
	 */
	UPROPERTY()
	TArray<TObjectPtr<ASector>> Sectors;

	/**
	 * Sectors which are in queue in order to cook up their meshes.
	 */
	TQueue<ASector*> SectorsToProcess;

	/**
	 * Sectors to be despawned.
	 */
	TQueue<ASector*> SectorsToDespawn;

	/**
	 * Convert a block position of a block to a sector position. Sector position is a block position of its most
	 * left-back-down block.
	 */
	FIntVector ConvertBlockPositionToSectorPosition(const FIntVector& BlockPosition) const;

	/**
	 * Determine if game world contains a sector with a specified sector position. Sector position is a block position
	 * of its most left-back-down block.
	 */
	bool DoContainsSector(const FIntVector& SectorPosition);
};
