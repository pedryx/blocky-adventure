#include "Sector.h"
#include "Chunk.h"
#include "BlockType.h"

ASector::ASector()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ASector::BeginPlay()
{
	Super::BeginPlay();

	for (int32 Y = 0; Y < SIZE; Y++)
	{
		for (int32 X = 0; X < SIZE; X++)
		{
			const FVector SpawnPosition{
				static_cast<double>(X * AChunk::TOTAL_SIZE),
				static_cast<double>(Y * AChunk::TOTAL_SIZE),
				0.0
			};

			AChunk* Chunk{ GetWorld()->SpawnActor<AChunk>(SpawnPosition, FRotator::ZeroRotator) };
			checkf(IsValid(Chunk), TEXT("Chunk cannot be spawned"));

			checkf(IsValid(Material), TEXT("Invalid material"));
			Chunk->Initialize(FIntVector2{ X, Y }, Material, this);
			Chunk->Generate();

			Chunks.Add(Chunk);
		}
	}

	//Chunks[0]->SetBlock(FIntVector{ 0, 15, 0 }, FBlockType::AIR_ID);
	//Chunks[1]->SetBlock(FIntVector{ 0, 15, 0 }, FBlockType::AIR_ID);
	//Chunks[2]->SetBlock(FIntVector{ 0, 15, 0 }, FBlockType::AIR_ID);
	//Chunks[3]->SetBlock(FIntVector{ 0, 15, 0 }, FBlockType::AIR_ID);

	for (AChunk* Chunk : Chunks)
	{
		Chunk->CreateMesh();
	}
}

bool ASector::IsBlockAir(const FIntVector& BlockPosition) const
{
	if (!IsInBounds(BlockPosition))
	{
		return true;
	}

	const int32 ChunkX = BlockPosition.X / AChunk::SIZE;
	const int32 ChunkY = BlockPosition.Y / AChunk::SIZE;
	const int32 ChunkIndex = ChunkY * SIZE + ChunkX;
	const FIntVector InChunkPosition{ BlockPosition - FIntVector{ ChunkX * AChunk::SIZE, ChunkY * AChunk::SIZE, 0 } };

	return Chunks[ChunkIndex]->IsAir(InChunkPosition);
}

bool ASector::IsInBounds(const FIntVector& BlockPosition) const
{
	return BlockPosition.X >= 0 && BlockPosition.Y >= 0 && BlockPosition.Z >= 0
		&& BlockPosition.X < AChunk::SIZE * SIZE && BlockPosition.Y < AChunk::SIZE * SIZE
		&& BlockPosition.Z < AChunk::HEIGHT;
}