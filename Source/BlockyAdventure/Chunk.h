#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Direction.h"
#include "BlockType.h"
#include "Chunk.generated.h"

class UProceduralMeshComponent;
class ASector;

/**
 * Represent a chunk of the game world sector. The game world is composed from sectors. Each sector is composed
 * from chunks. Each chunk has its own mesh.
 */
UCLASS()
class BLOCKYADVENTURE_API AChunk : public AActor
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
	 * Size of chunk in X and Y dimension.
	 */
	inline static constexpr int32 TOTAL_SIZE{ AChunk::SIZE * AChunk::BLOCK_SIZE };

	void Initialize(
		const FIntVector2& InChunkCoord,
		TObjectPtr<UMaterialInterface> InMaterial, 
		TObjectPtr<ASector> InSector
	) {
		ChunkCoord = InChunkCoord;
		Material = InMaterial;
		Sector = InSector;
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
	 * Determine if block at specified position, local to chunk, is air.
	 */
	bool IsAir(const FIntVector& BlockPosition) const;

	/**
	* Set block at specified position, local to this chunk, to a block type with specified ID.
	*/
	void SetBlock(const FIntVector& BlockPosition, const BlockTypeID ID)
	{
		checkf(IsInBounds(BlockPosition), TEXT("Position is outside of chunk bounds"));

		Blocks[GetBlockIndex(BlockPosition)] = ID;
	}

	/**
	 * Get block type ID of block at specified position, local to this chunk.
	 */
	BlockTypeID GetBlock(const FIntVector& BlockPosition) const
	{
		checkf(IsInBounds(BlockPosition), TEXT("Position is outside of chunk bounds"));

		return Blocks[GetBlockIndex(BlockPosition)];
	}
protected:
	virtual void BeginPlay() override;

private:
	TObjectPtr<UProceduralMeshComponent> MeshComponent;
	TArray<FVector> MeshVertices;
	TArray<int32> MeshIndices;
	TArray<FVector> MeshNormals;
	TArray<FColor> MeshColors;

	TArray<BlockTypeID> Blocks;

	FIntVector2 ChunkCoord;
	TObjectPtr<UMaterialInterface> Material;
	TObjectPtr<ASector> Sector;

	const FVector BlockVertices[8]
	{
		FVector{          0,          0,          0 }, // 0 left-back-bottom
		FVector{ BLOCK_SIZE,          0,          0 }, // 1 right-back-bottom
		FVector{          0, BLOCK_SIZE,	      0 }, // 2 left-front-bottom
		FVector{ BLOCK_SIZE, BLOCK_SIZE,          0 }, // 3 right-front-bottom
		FVector{          0,          0, BLOCK_SIZE }, // 4 left-back-top
		FVector{ BLOCK_SIZE,          0, BLOCK_SIZE }, // 5 right-back-top
		FVector{          0, BLOCK_SIZE, BLOCK_SIZE }, // 6 left-front-top
		FVector{ BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE }, // 7 right-front-top
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
	 * Try to create mesh for all visible faces of the block at specified position, local to this chunk. Will create no
	 * mesh when no face of the block is visible. Create mesh data will be appended to the chunk mesh data.
	 */
	void TryCreateBlock(const FIntVector& Position);

	/**
	 * Append mesh data for one face of the block.
	 * 
	 * \param Position Position of the block, local to this chunk.
	 * \param Direction Direction of the face of the block.
	 * \param BlockType Type of block.
	 */
	void CreateFace(const FVector& Position, const EDirection Direction, const FBlockType& BlockType);

	/**
	 * Get normal vector of the face of the block.
	 * 
	 * \param Face Direction which represent the face of the block.
	 * \return Normal vector of the face of the block.
	 */
	FVector GetFaceNormal(const EDirection Face) const;

	/**
	 * Convert block position, local to this chunk, into an index which can be used to index chunk's blocks.
	 */
	int32 GetBlockIndex(const FIntVector& BlockPosition) const
	{
		return BlockPosition.Z * SIZE * SIZE + BlockPosition.Y * SIZE + BlockPosition.X;
	}

	/**
	 * Determine if block position, local to this chunk, is within chunk bounds.
	 */
	bool IsInBounds(const FIntVector& BlockPosition) const
	{
		return BlockPosition.X >= 0 && BlockPosition.Y >= 0 && BlockPosition.Z >= 0
			&& BlockPosition.X < SIZE && BlockPosition.Y < SIZE && BlockPosition.Z < HEIGHT;
	}

	/**
	 * Transform a specified position, local to this chunk, into position local to this sector.
	 */
	FIntVector GetInSectorPosition(const FIntVector& BlockPosition) const
	{
		return FIntVector{ ChunkCoord.X * SIZE, ChunkCoord.Y * SIZE, 0 } + BlockPosition;
	}
};