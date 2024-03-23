#include "Sector.h"
#include "Chunk.h"
#include "BlockType.h"

ASector::ASector()
{
	PrimaryActorTick.bCanEverTick = false;
}

FIntVector ASector::GetBlockPosition(const FVector& WorldPosition) const
{
	FIntVector BlockPosition{
		static_cast<int>(WorldPosition.X) / AChunk::BLOCK_SIZE,
		static_cast<int>(WorldPosition.Y) / AChunk::BLOCK_SIZE,
		static_cast<int>(WorldPosition.Z) / AChunk::BLOCK_SIZE
	};

	return BlockPosition;
}

void ASector::SetBlock(const FIntVector& BlockPosition, BlockTypeID ID)
{
	AChunk* Chunk = GetChunk(BlockPosition);
	const FIntVector InChunkBlockPosition = GetInChunkBlockPosition(Chunk, BlockPosition);
	Chunk->SetBlock(InChunkBlockPosition, ID);
}

void ASector::BeginPlay()
{
	Super::BeginPlay();

	Chunks.Reserve(SIZE * SIZE);

	Create();
}

void ASector::Create()
{
	Generate();
	CreateMesh();
}

void ASector::Clear()
{
	for (AChunk* Chunk : Chunks)
	{
		Chunk->Destroy();
	}
	Chunks.Empty(SIZE * SIZE);
}

void ASector::Generate()
{
	Clear();

	for (int32 X = 0; X < SIZE; ++X)
	{
		for (int32 Y = 0; Y < SIZE; ++Y)
		{
			const FVector SpawnPosition{
				static_cast<double>(X * AChunk::TOTAL_SIZE),
				static_cast<double>(Y * AChunk::TOTAL_SIZE),
				0.0
			};

			AChunk* Chunk{ GetWorld()->SpawnActor<AChunk>(SpawnPosition, FRotator::ZeroRotator) };
			checkf(IsValid(Chunk), TEXT("Unable to spawn chunk."));

			checkf(IsValid(Material), TEXT("Material was not specified."));
			Chunk->Initialize(FIntVector2{ X, Y }, Material, this);
			Chunk->Generate();

			Chunks.Add(Chunk);
		}
	}
}

void ASector::CreateMesh()
{
	for (AChunk* Chunk : Chunks)
	{
		Chunk->CreateMesh();
	}
}

bool ASector::IsBlockAir(const FIntVector& BlockPosition) const
{
	if (!IsBlockInBounds(BlockPosition))
	{
		return true;
	}

	const AChunk* Chunk = GetChunk(BlockPosition);
	const FIntVector InChunkPosition = GetInChunkBlockPosition(Chunk, BlockPosition);

	return Chunk->IsBlockAir(InChunkPosition);
}

bool ASector::IsBlockInBounds(const FIntVector& BlockPosition) const
{
	return BlockPosition.X >= 0 && BlockPosition.Y >= 0 && BlockPosition.Z >= 0
		&& BlockPosition.X < AChunk::SIZE * SIZE && BlockPosition.Y < AChunk::SIZE * SIZE
		&& BlockPosition.Z < AChunk::HEIGHT;
}

AChunk* ASector::GetChunk(const FIntVector& BlockPosition) const
{
	const int32 ChunkX = BlockPosition.X / AChunk::SIZE;
	const int32 ChunkY = BlockPosition.Y / AChunk::SIZE;
	const int32 ChunkIndex = ChunkX * SIZE + ChunkY;

	return Chunks[ChunkIndex];
}

FIntVector ASector::GetInChunkBlockPosition(const AChunk* Chunk, const FIntVector& BlockPosition) const
{
	const FIntVector2 ChunkCoord = Chunk->GetCoord();
	const FIntVector InChunkPosition{ BlockPosition - FIntVector{ 
		ChunkCoord.X * AChunk::SIZE,
		ChunkCoord.Y * AChunk::SIZE,
		0
	} };
	checkf(Chunk->IsBlockInBounds(InChunkPosition), TEXT("Block is not within chunk bounds."));

	return InChunkPosition;
}

int32 ASector::ComputeHeight(const FIntVector2& BlockPosition) const
{
	checkf(Octaves.Num() > 0, TEXT("Cannot generate noise from zero octaves."));

	FVector2D NoisePosition{ static_cast<double>(BlockPosition.X), static_cast<double>(BlockPosition.Y) };
	NoisePosition /= AChunk::SIZE;

	double NoiseValue{ 0.0 };
	double WeightSum{ 0.0 };
	for (const FOctave Octave : Octaves)
	{
		NoiseValue += Octave.Weight * (FMath::PerlinNoise2D(NoisePosition * Octave.Frequency) + 1.0) / 2.0;
		WeightSum += Octave.Weight;
	}
	NoiseValue /= WeightSum;

	return NoiseValue * AChunk::HEIGHT;
}