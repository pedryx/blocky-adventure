#include "GameWorld.h"
#include "Octave.h"
#include "Sector.h"
#include "Chunk.h"

#include "Components/SceneComponent.h"
#include "Async/Async.h"
#include "Async/Future.h"

AGameWorld::AGameWorld()
{
	PrimaryActorTick.bCanEverTick = true;

	TObjectPtr<USceneComponent> SceneComponent{ CreateDefaultSubobject<USceneComponent>(TEXT("Root")) };
	checkf(IsValid(SceneComponent), TEXT("Unable to create scene component."));
	SetRootComponent(SceneComponent);
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
	FIntVector BlockPosition{};

	BlockPosition.X = FMath::RoundToNegativeInfinity(WorldPosition.X / AChunk::BLOCK_SIZE);
	BlockPosition.Y = FMath::RoundToNegativeInfinity(WorldPosition.Y / AChunk::BLOCK_SIZE);
	BlockPosition.Z = FMath::RoundToNegativeInfinity(WorldPosition.Z / AChunk::BLOCK_SIZE);

	return BlockPosition;
}

void AGameWorld::BeginPlay()
{
	Super::BeginPlay();

	SpawnSector(FIntVector::ZeroValue, false);
}

void AGameWorld::Tick(float DeltaSeconds)
{
	TObjectPtr<ASector> SectorToProcess{};
	if (SectorsToProcess.Dequeue(SectorToProcess))
	{
		SectorToProcess->CookMesh(true);
	}

	TObjectPtr<ASector> SectorToDespawn{};
	if (SectorsToDespawn.Peek(SectorToDespawn) && SectorToDespawn->IsReady())
	{
		SectorsToDespawn.Dequeue(SectorToDespawn);
		DespawnSector(SectorToDespawn->GetPosition());
	}
}

FIntVector AGameWorld::ConvertBlockPositionToSectorPosition(const FIntVector& BlockPosition) const
{
	constexpr int32 SECTOR_SIZE{ ASector::SIZE * AChunk::SIZE };
	FIntVector SectorPosition{};

	SectorPosition.X = FMath::RoundToNegativeInfinity(static_cast<float>(BlockPosition.X) / SECTOR_SIZE);
	SectorPosition.Y = FMath::RoundToNegativeInfinity(static_cast<float>(BlockPosition.Y) / SECTOR_SIZE);
	SectorPosition.Z = 0;

	SectorPosition *= SECTOR_SIZE;

	return SectorPosition;
}

void AGameWorld::SpawnSector(const FIntVector& BlockPosition, const bool bShouldIgnoreFirstOverlap)
{
	const FIntVector SectorPosition{ ConvertBlockPositionToSectorPosition(BlockPosition) };

	if (DoContainsSector(SectorPosition))
	{
		return;
	}

	TObjectPtr<ASector> Sector = GetWorld()->SpawnActor<ASector>(
		static_cast<FVector>(SectorPosition * AChunk::BLOCK_SIZE),
		FRotator::ZeroRotator
	);
	checkf(IsValid(Sector), TEXT("Unable to spawn sector."));
	Sectors.Add(Sector);
	LastSpawnedSector = Sector;

	Sector->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
	Sector->Initialize(this, SectorPosition, bShouldIgnoreFirstOverlap);

	auto DoWork = [Sector]()
	{
		Sector->Generate();
		Sector->CreateMesh();
	};

	auto OnComplete = [this, Sector]()
	{
		SectorsToProcess.Enqueue(Sector);
	};

	Async(EAsyncExecution::ThreadPool, MoveTemp(DoWork), MoveTemp(OnComplete));
}

void AGameWorld::DespawnSector(const FIntVector& BlockPosition)
{
	const FIntVector SectorPosition{ ConvertBlockPositionToSectorPosition(BlockPosition) };
	TObjectPtr<ASector> Sector{};

	for (TObjectPtr<ASector> CurrentSector : Sectors)
	{
		if (CurrentSector->GetPosition() == SectorPosition)
		{
			Sector = CurrentSector;
		}
	}
	checkf(IsValid(Sector), TEXT("Sector is not spawned."));

	if (!Sector->IsReady())
	{
		SectorsToDespawn.Enqueue(Sector);
		return;
	}

	Sectors.RemoveSwap(Sector);

	TArray<TObjectPtr<AActor>> AttachedActors;
	Sector->GetAttachedActors(AttachedActors, true, false);

	for (TObjectPtr<AActor> Actor : AttachedActors)
	{
		Actor->Destroy();
	}
	Sector->Destroy();
}

bool AGameWorld::DoContainsSector(const FIntVector& SectorPosition)
{
	for (const TObjectPtr<const ASector> Sector : Sectors)
	{
		if (Sector->GetPosition() == SectorPosition)
		{
			return true;
		}
	}

	return false;
}