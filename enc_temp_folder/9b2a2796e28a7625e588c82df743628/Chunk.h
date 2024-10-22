#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Direction.h"
#include "Containers/BitArray.h"
#include "BlockPtr.h"
#include "Chunk.generated.h"

class UProceduralMeshComponent;
class AGameWorld;
class ASector;

/**
 * Represent a chunk of the game world sector. The game world is composed from sectors. Each sector is composed
 * from chunks. Each chunk has its own mesh.
 */
UCLASS()
class BLOCKYADVENTURE_API AChunk final : public AActor
{
	GENERATED_BODY()

public:
	AChunk();

	/**
	 * Number of blocks in chunk in X and Y dimension.
	 */
	inline static constexpr int32 SIZE{ 16 };
	/**
	 * Number of blocks in chunk in Z dimension.
	 */
	inline static constexpr int32 HEIGHT{ 128 };
	/**
	 * Height from which all blocks are snow.
	 */
	inline static constexpr int32 SNOW_HEIGHT{ 75 };
	/**
	 * Height from which all blocks are stone.
	 */
	inline static constexpr int32 ROCK_HEIGHT{ 60 };
	/**
	 * Height of dirt blocks layer.
	 */
	inline static constexpr int32 DIRT_LAYER_HEIGHT{ 5 };
	/**
	 * Size of one block.
	 */
	inline static constexpr int32 BLOCK_SIZE{ 100 };
	/**
	 * World size of chunk in X and Y dimension.
	 */
	inline static constexpr int32 TOTAL_SIZE{ SIZE * BLOCK_SIZE };
	/**
	 * Number of blocks in the chunk.
	 */
	inline static constexpr int32 BLOCK_COUNT{ SIZE * SIZE * HEIGHT };

	/**
	 * Initialize this chunk.
	 * 
	 * \param InSector Sector to which this chunk belongs.
	 * \param InPosition Block position of the lower left back block of this chunk.
	 */
	void Initialize(ASector* const InSector, const FIntVector& InPosition)
	{
		Sector = InSector;
		Position = InPosition;
	}

	/**
	 * Generate terrain for the chunk.
	 */
	void Generate();

	/**
	 * Create mesh for the chunk.
	 */
	void CreateMesh();

	/**
	 * Cook created mesh for the chunk.
	 */
	void CookMesh(const bool bUseAsyncCooking);

	/**
	 * Get a game world which owns sector which owns this chunk.
	 */
	AGameWorld* GetGameWorld();

	/**
	 * Get a sector which owns this chunk.
	 */
	ASector* GetSector() const { return Sector; }

	/**
	 * Get a reference to a block at specified position. Specified position must be in the bounds of this chunk.
	 */
	const FBlockPtr GetBlock(const FIntVector& BlockPosition) const;

	/**
	 * Get a reference to a block at specified position. Specified position must be in the bounds of this chunk.
	 */
	FBlockPtr GetBlock(const FIntVector& BlockPosition);

	/**
	 * Determine if a block at a specified position is within the bounds of this chunk.
	 */
	bool IsBlockInBounds(const FIntVector& BlockPosition) const;

	/**
	 * Gete block data of this chunk.
	 */
	uint8* GetBlockData() { return Blocks.GetData(); }

private:
	/**
	 * Mesh for blocks which belongs to this chunk.
	 */
	UPROPERTY()
	TObjectPtr<UProceduralMeshComponent> MeshComponent;
	TArray<FVector> MeshVertices;
	TArray<int32> MeshIndices;
	TArray<FVector> MeshNormals;
	TArray<FColor> MeshColors;
	/**
	 * Contains information about which blocks have been processed. Is used during chunk mesh generation.
	 */
	TBitArray<> ProcessedBlocks{ false, BLOCK_COUNT * DIRECTION_COUNT };

	/**
	 * Represent this chunk. Blocks are mapped into flat array, first by Z, then by Y, then by X. Each block is
	 * represented by aits ID. Air (empty) blocks are represented by air block ID.
	 */
	TArray<BlockTypeID> Blocks;
	/**
	 * Represent a sector to which this chunk belongs.
	 */
	UPROPERTY()
	TObjectPtr<ASector> Sector;
	/**
	 * Position of the left-back-down corner of the chunk.
	 */
	FIntVector Position;

	const FVector BlockVertices[8]
	{
		FVector{          0,          0,          0 }, // 0 left-front-bottom
		FVector{ BLOCK_SIZE,          0,          0 }, // 1 right-front-bottom
		FVector{          0, BLOCK_SIZE,	      0 }, // 2 left-back-bottom
		FVector{ BLOCK_SIZE, BLOCK_SIZE,          0 }, // 3 right-back-bottom
		FVector{          0,          0, BLOCK_SIZE }, // 4 left-front-top
		FVector{ BLOCK_SIZE,          0, BLOCK_SIZE }, // 5 right-front-top
		FVector{          0, BLOCK_SIZE, BLOCK_SIZE }, // 6 left-back-top
		FVector{ BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE }, // 7 right-back-top
	};

	const int32 BlockIndices[24]
	{
		0, 1, 2, 3, // bottom
		2, 3, 6, 7, // front
		0, 2, 4, 6, // left
		1, 5, 3, 7, // right
		0, 4, 1, 5, // back
		4, 6, 5, 7, // top
	};

	const int32 FaceVertexIndices[6]{ 0, 1, 2, 1, 3, 2 };

	inline static constexpr int32 FACE_VERTICES_COUNT{ 4 };

	/**
	 * Start creating mesh run for block at a specified position, run will try to create as much quads as possible
	 * using greedy meshing alghoritm. Currently using only quad strips.
	 */
	void StartMeshRun(const FIntVector& BlockPosition, const int32 BlockIndex);

	/**
	 * Get index which can be used to access blocks array from a specified block position.
	 */
	int32 GetBlockIndex(const FIntVector& BlockPosition) const;

	struct FDirectionData
	{
		FIntVector Normal;
		int32 Bound;
		int32 Offset;
		int32 Position;
		EDirection PerpendicularDirections[2];
	};

	FDirectionData GetDirectionData(const EDirection Direction, const FIntVector& BlockPosition) const;
};
