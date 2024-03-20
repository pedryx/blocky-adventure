#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Direction.h"
#include "BlockType.h"
#include "Chunk.generated.h"

class UProceduralMeshComponent;

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
	 * Material used for blocks.
	 */
	UPROPERTY(EditInstanceOnly, Category = "Chunk")
	TObjectPtr<UMaterialInterface> Material;

	inline static constexpr int32 SIZE{ 16 };
	inline static constexpr int32 HEIGHT{ 256 };
	inline static constexpr int32 TOTAL_SIZE{ SIZE * SIZE * HEIGHT };
	inline static constexpr int32 DIRT_LAYER_HEIGHT{ 3 };
	inline static constexpr int32 BLOCK_SIZE{ 100 };
protected:
	virtual void BeginPlay() override;

private:
	TObjectPtr<UProceduralMeshComponent> MeshComponent;

	TArray<FVector> MeshVertices;
	TArray<int32> MeshIndices;
	TArray<FVector> MeshNormals;
	TArray<FColor> MeshColors;

	TArray<BlockTypeID> Blocks;

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
	 * Generate terrain for the chunk.
	 */
	void Generate();

	/**
	 * Create mesh for the chunk.
	 */
	void CreateMesh();

	/**
	 * Try to create mesh for all visible faces of the block at specified position, local to chunk. Will create no mesh
	 * when no face of the block is visible. Create mesh data will be appended to the chunk mesh data.
	 */
	void TryCreateBlock(FIntVector Position);

	/**
	 * Append mesh data for one face of the block.
	 * 
	 * \param Position Position of the block.
	 * \param Direction Direction of the face of the block.
	 * \param BlockType Type of block.
	 */
	void CreateFace(FVector Position, EDirection Direction, const FBlockType& BlockType);

	/**
	 * Get normal vector of the face of the block.
	 * 
	 * \param Face Direction which represent the face of the block.
	 * \return Normal vector of the face of the block.
	 */
	FVector GetFaceNormal(EDirection Face);

	/**
	 * Convert block position, local to chunk, to index which can be used to index chunk's blocks.
	 */
	int32 GetBlockIndex(FIntVector BlockPosition)
	{
		return BlockPosition.Z* SIZE* SIZE + BlockPosition.Y * SIZE + BlockPosition.X;
	}

	/**
	 * Determine if block position, local to chunk, is within chunk bounds.
	 */
	bool IsInBounds(FIntVector BlockPosition)
	{
		return BlockPosition.X >= 0 && BlockPosition.Y >= 0 && BlockPosition.Z >= 0
			&& BlockPosition.X < SIZE && BlockPosition.Y < SIZE && BlockPosition.Z < HEIGHT;
	}

	/**
	* Set block at specified position to a block type with specified ID.
	*/
	void SetBlock(FIntVector BlockPosition, BlockTypeID ID)
	{
		checkf(IsInBounds(BlockPosition), TEXT("Position is outside of chunk bounds"));

		Blocks[GetBlockIndex(BlockPosition)] = ID;
	}

	/**
	 * Get block type ID of block at specified position, local to chunk.
	 */
	BlockTypeID GetBlock(FIntVector BlockPosition)
	{
		checkf(IsInBounds(BlockPosition), TEXT("Position is outside of chunk bounds"));

		return Blocks[GetBlockIndex(BlockPosition)];
	}

	/**
	 * Determine if block at specified position, local to chunk, is air. Blocks outside of this chunk are also
	 * considered as air.
	 */
	bool IsAir(FIntVector BlockPosition)
	{
		if (!IsInBounds(BlockPosition)) {
			return true;
		}

		return GetBlock(BlockPosition) == FBlockType::AIR_ID;
	}

	
};
