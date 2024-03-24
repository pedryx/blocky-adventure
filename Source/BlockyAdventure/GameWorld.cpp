#include "GameWorld.h"
#include "Octave.h"
#include "Sector.h"
#include "Chunk.h"

AGameWorld::AGameWorld()
{
	PrimaryActorTick.bCanEverTick = false;
}

TObjectPtr<ASector> AGameWorld::GetSector(const FIntVector& BlockPosition)
{
	const FIntVector SectorPosition{ ConvertBlockPositionToSectorPosition(BlockPosition) };

	for (const TObjectPtr<ASector> Sector : Sectors)
	{
		if (Sector->GetPosition() == SectorPosition)
		{
			return Sector;
		}
	}

	ensureMsgf(false, TEXT("Block is not in bounds of any loaded sector."));
	return nullptr;
}

TObjectPtr<AChunk> AGameWorld::GetChunk(const FIntVector& BlockPosition)
{
	const TObjectPtr<ASector> Sector{ GetSector(BlockPosition) };
	
	return Sector->GetChunk(BlockPosition);
}

BlockTypeID& AGameWorld::GetBlock(const FIntVector& BlockPosition)
{
	TObjectPtr<AChunk> Chunk{ GetChunk(BlockPosition) };

	return Chunk->GetBlock(BlockPosition);
}

bool AGameWorld::IsBlockAir(const FIntVector& BlockPosition)
{
	if (!IsBlockInBounds(BlockPosition))
	{
		return true;
	}
	else
	{
		return GetBlock(BlockPosition) == FBlockType::AIR_ID;
	}
}

bool AGameWorld::IsBlockInBounds(const FIntVector& BlockPosition) const
{
	for (const TObjectPtr<const ASector> Sector : Sectors)
	{
		if (Sector->IsBlockInBounds(BlockPosition))
		{
			return true;
		}
	}

	return false;
}

int32 AGameWorld::ComputeHeight(const FIntVector2& BlockPosition) const
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

FIntVector AGameWorld::GetBlockPosition(const FVector& WorldPosition) const
{
	return static_cast<FIntVector>(WorldPosition) / AChunk::BLOCK_SIZE;
}

void AGameWorld::BeginPlay()
{
	Super::BeginPlay();

	SpawnSector(FIntVector::ZeroValue);
}

FIntVector AGameWorld::ConvertBlockPositionToSectorPosition(const FIntVector& BlockPosition) const
{
	constexpr int32 SECTOR_SIZE{ ASector::SIZE * AChunk::SIZE };
	FIntVector SectorPosition{ (BlockPosition / SECTOR_SIZE) * SECTOR_SIZE };
	SectorPosition.Z = 0;

	return SectorPosition;
}

void AGameWorld::SpawnSector(const FIntVector& BlockPosition)
{
	const FIntVector SectorPosition{ ConvertBlockPositionToSectorPosition(BlockPosition) };

	TObjectPtr<ASector> Sector = GetWorld()->SpawnActor<ASector>(
		static_cast<FVector>(SectorPosition),
		FRotator::ZeroRotator
	);
	checkf(IsValid(Sector), TEXT("Unable to spawn sector."));
	Sector->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
	Sector->Initialize(this, SectorPosition);

	Sectors.Add(Sector);
	Sector->Generate();
	Sector->CreateMesh();
}
