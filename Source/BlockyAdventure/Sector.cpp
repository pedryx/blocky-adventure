#include "Sector.h"
#include "GameWorld.h"
#include "Chunk.h"
#include "BlockType.h"
#include "Components/SceneComponent.h"

ASector::ASector()
{
	PrimaryActorTick.bCanEverTick = false;

	TObjectPtr<USceneComponent> SceneComponent{ CreateDefaultSubobject<USceneComponent>(TEXT("Root")) };
	checkf(IsValid(SceneComponent), TEXT("Unable to create scene component."));
	SetRootComponent(SceneComponent);
}

void ASector::Generate()
{
	for (const TObjectPtr<AChunk> Chunk : Chunks)
	{
		Chunk->Destroy();
	}
	Chunks.Empty(SIZE * SIZE);

	for (int32 X = 0; X < SIZE; ++X)
	{
		for (int32 Y = 0; Y < SIZE; ++Y)
		{
			const FVector SpawnPosition{
				static_cast<double>(X * AChunk::TOTAL_SIZE),
				static_cast<double>(Y * AChunk::TOTAL_SIZE),
				0.0
			};

			TObjectPtr<AChunk> Chunk{ GetWorld()->SpawnActor<AChunk>(SpawnPosition, FRotator::ZeroRotator) };
			checkf(IsValid(Chunk), TEXT("Unable to spawn chunk."));
			Chunk->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
			Chunk->Initialize(this, FIntVector{ X * AChunk::SIZE, Y * AChunk::SIZE, 0 } + Position);

			Chunks.Add(Chunk);
			Chunk->Generate();
		}
	}
}

void ASector::CreateMesh()
{
	for (const TObjectPtr<AChunk> Chunk : Chunks)
	{
		Chunk->CreateMesh();
	}
}

TObjectPtr<AChunk> ASector::GetChunk(const FIntVector& BlockPosition)
{
 	checkf(IsBlockInBounds(BlockPosition), TEXT("Block is outside this chunk bounds."));

	const FIntVector ChunkCoordinate{ (BlockPosition - Position) / AChunk::SIZE };
	const int32 ChunkIndex{ ChunkCoordinate.X * SIZE + ChunkCoordinate.Y };

	return Chunks[ChunkIndex];
}

BlockTypeID& ASector::GetBlock(const FIntVector& BlockPosition)
{
	TObjectPtr<AChunk> Chunk{ GetChunk(BlockPosition) };

	return Chunk->GetBlock(BlockPosition);
}

bool ASector::IsBlockInBounds(const FIntVector& BlockPosition) const
{
	constexpr int32 SECTOR_SIZE{ SIZE * AChunk::SIZE };

	const FIntVector TopRightFront{ Position + FIntVector{ SECTOR_SIZE, SECTOR_SIZE, AChunk::HEIGHT } };

	bool bIsLargerThanBottomLeftBack
	{
		BlockPosition.X >= Position.X &&
		BlockPosition.Y >= Position.Y &&
		BlockPosition.Z >= Position.Z
	};
	bool bIsSmallerThanTopRightFront
	{
		BlockPosition.X < TopRightFront.X &&
		BlockPosition.Y < TopRightFront.Y &&
		BlockPosition.Z < TopRightFront.Z
	};

	return bIsLargerThanBottomLeftBack && bIsSmallerThanTopRightFront;
}
