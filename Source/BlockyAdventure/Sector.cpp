#include "Sector.h"
#include "GameWorld.h"
#include "Chunk.h"
#include "BlockType.h"

#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Async/ParallelFor.h"

ASector::ASector()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	checkf(IsValid(RootComponent), TEXT("Unable to create scene component."));
	SetRootComponent(RootComponent);

	CreateTriggers();
}

void ASector::Initialize(
	AGameWorld* const InGameWorld, 
	const FIntVector& InPosition,
	const bool bInShouldIgnoreFirstOverlap
)
{
	GameWorld = InGameWorld;
	Position = InPosition;
	bShouldIgnoreFirstOverlap = bInShouldIgnoreFirstOverlap;
	FileName = FPaths::ProjectSavedDir() + FString::Printf(TEXT("Sectors/sector_%d_%d.bin"), Position.X, Position.Y);

	const FString SectorsDirectory{ FPaths::ProjectSavedDir() + TEXT("Sectors") };
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*SectorsDirectory))
	{
		PlatformFile.CreateDirectory(*SectorsDirectory);
	}

	CreateChunks();
}

void ASector::Generate()
{
	if (DoSectorFileExists())
	{
		LoadFromFile();
		return;
	}

	ParallelFor(Chunks.Num(), [this](int32 Index) { Chunks[Index]->Generate(); });
}

void ASector::CreateMesh()
{
	ParallelFor(Chunks.Num(), [this](int32 Index) { Chunks[Index]->CreateMesh(); });
}

void ASector::CookMesh(const bool bUseAsyncCooking)
{
	for (const TObjectPtr<AChunk> Chunk : Chunks)
	{
		Chunk->CookMesh(bUseAsyncCooking);
	}

	bIsReady = true;
}

AChunk* ASector::GetChunk(const FIntVector& BlockPosition)
{
 	checkf(IsBlockInBounds(BlockPosition), TEXT("Block is outside this chunk bounds."));

	const FIntVector ChunkCoordinate{ (BlockPosition - Position) / AChunk::SIZE };
	const int32 ChunkIndex{ ChunkCoordinate.X * SIZE + ChunkCoordinate.Y };

	return Chunks[ChunkIndex];
}

BlockTypeID& ASector::GetBlock(const FIntVector& BlockPosition)
{
	AChunk* const Chunk{ GetChunk(BlockPosition) };

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

void ASector::SaveToFile() const
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	IFileHandle* FileHandle = PlatformFile.OpenWrite(*FileName);

	if (!FileHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot write to the sector file %s."), *FileName);
		return;
	}

	constexpr int32 CHUNK_SIZE{ AChunk::SIZE * AChunk::SIZE * AChunk::HEIGHT };

	for (const TObjectPtr<AChunk> Chunk : Chunks)
	{
		FileHandle->Write(Chunk->GetBlockData(), CHUNK_SIZE);
	}
	delete FileHandle;
}

void ASector::LoadFromFile()
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	IFileHandle* FileHandle = PlatformFile.OpenRead(*FileName);

	if (!FileHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot read from the sector file %s."), *FileName);
		return;
	}

	constexpr int32 CHUNK_SIZE{ AChunk::SIZE * AChunk::SIZE * AChunk::HEIGHT };

	for (const TObjectPtr<AChunk> Chunk : Chunks)
	{
		FileHandle->Read(Chunk->GetBlockData(), CHUNK_SIZE);
	}
	delete FileHandle;
}

bool ASector::DoSectorFileExists() const
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	return PlatformFile.FileExists(*FileName);
}

void ASector::CreateChunks()
{
	Chunks.Reserve(SIZE * SIZE);

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
			Chunk->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
			Chunk->Initialize(this, FIntVector{ X * AChunk::SIZE, Y * AChunk::SIZE, 0 } + Position);

			Chunks.Add(Chunk);
		}
	}
}

void ASector::CreateTriggers()
{
	constexpr int32 BASE_BOX_SIZE{ ASector::SIZE * AChunk::TOTAL_SIZE / 2 };
	constexpr int32 BOX_SIZE{ BASE_BOX_SIZE + AChunk::TOTAL_SIZE };
	constexpr int32 BOX_HEIGHT{ AChunk::HEIGHT * AChunk::BLOCK_SIZE * 2 };

	#pragma region Trigger Definitions
	CreateTrigger(
		SectorTrigger,
		TEXT("Sector Trigger"),
		FVector{ BASE_BOX_SIZE, BASE_BOX_SIZE, 0 },
		FVector{ BOX_SIZE, BOX_SIZE, BOX_HEIGHT },
		false,
		true
	);

	CreateTrigger(
		NorthTrigger,
		TEXT("North Trigger"),
		FVector{ BASE_BOX_SIZE, 2 * BASE_BOX_SIZE, 0 },
		FVector{ BASE_BOX_SIZE, AChunk::TOTAL_SIZE, BOX_HEIGHT },
		true,
		false
	);

	CreateTrigger(
		EastTrigger,
		TEXT("East Trigger"),
		FVector{ 2 * BASE_BOX_SIZE, BASE_BOX_SIZE, 0 },
		FVector{ AChunk::TOTAL_SIZE, BASE_BOX_SIZE, BOX_HEIGHT },
		true,
		false
	);

	CreateTrigger(
		SouthTrigger,
		TEXT("South Trigger"),
		FVector{ BASE_BOX_SIZE, 0, 0 },
		FVector{ BASE_BOX_SIZE, AChunk::TOTAL_SIZE, BOX_HEIGHT },
		true,
		false
	);

	CreateTrigger(
		WestTrigger,
		TEXT("West Trigger"),
		FVector{ 0, BASE_BOX_SIZE, 0 },
		FVector{ AChunk::TOTAL_SIZE, BASE_BOX_SIZE, BOX_HEIGHT },
		true,
		false
	);
	#pragma endregion
}

void ASector::CreateTrigger(
	TObjectPtr<UBoxComponent>& OutTrigger,
	const FName& Name,
	const FVector& TriggerPosition, 
	const FVector& Size, 
	const bool bShouldBeginOverlap, 
	const bool bShouldEndOverlap,
	const bool bShouldBeHidden
)
{
	OutTrigger = CreateDefaultSubobject<UBoxComponent>(Name);
	checkf(IsValid(OutTrigger), TEXT("Unable to create box component."));
	OutTrigger->SetupAttachment(RootComponent);

	OutTrigger->SetRelativeLocation(TriggerPosition);
	OutTrigger->SetBoxExtent(Size);

	OutTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OutTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	OutTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	OutTrigger->SetHiddenInGame(bShouldBeHidden);

	if (bShouldBeginOverlap)
	{
		FScriptDelegate OnBeginOverlapDelegate{};
		OnBeginOverlapDelegate.BindUFunction(this, TEXT("OnBoxBeginOverlap"));
		OutTrigger->OnComponentBeginOverlap.Add(OnBeginOverlapDelegate);
	}

	if (bShouldEndOverlap)
	{
		FScriptDelegate OnEndOverlapDelegate{};
		OnEndOverlapDelegate.BindUFunction(this, TEXT("OnBoxEndOverlap"));
		OutTrigger->OnComponentEndOverlap.Add(OnEndOverlapDelegate);
	}
}

void ASector::OnBoxBeginOverlap(
	UPrimitiveComponent* OverlappedComponent, 
	AActor* OtherActor, 
	UPrimitiveComponent* OtherComponent, 
	int32 OtherBodyIndex, 
	bool bFromSweep, 
	const FHitResult& SweepResult
)
{
	// Begin overlap will be triggered upon sector spawn, but at that moment sector is not initialized.
	if (GameWorld == nullptr)
	{
		return;
	}

	UE_LOG(
		LogTemp, 
		Display, 
		TEXT("Overlap begins with a spawn sector trigger in sector at %s."), 
		*Position.ToString()
	);

	constexpr int32 SECTOR_SIZE{ SIZE * AChunk::SIZE };
	FIntVector SectorPosition{ Position };

	if (OverlappedComponent == NorthTrigger)
	{
		SectorPosition.Y += SECTOR_SIZE;
	}
	else if (OverlappedComponent == EastTrigger)
	{
		SectorPosition.X += SECTOR_SIZE;
	}
	else if (OverlappedComponent == SouthTrigger)
	{
		SectorPosition.Y -= SECTOR_SIZE;
	}
	else if (OverlappedComponent == WestTrigger)
	{
		SectorPosition.X -= SECTOR_SIZE;
	}
	else
	{
		ensureAlwaysMsgf(false, TEXT("Overlap with invalid object."));
		return;
	}

	GameWorld->SpawnSector(SectorPosition);
}

void ASector::OnBoxEndOverlap(
	UPrimitiveComponent* OverlappedComponent, 
	AActor* OtherActor, 
	UPrimitiveComponent* OtherComponent, 
	int32 OtherBodyIndex, 
	bool bFromSweep, 
	const FHitResult& SweepResult
)
{
	if (bShouldIgnoreFirstOverlap)
	{
		bShouldIgnoreFirstOverlap = false;
		return;
	}
	UE_LOG(
		LogTemp, 
		Display, 
		TEXT("Overlap ends with a despawn sector trigger in sector at %s."), 
		*Position.ToString()
	);

	GameWorld->DespawnSector(Position);
}
