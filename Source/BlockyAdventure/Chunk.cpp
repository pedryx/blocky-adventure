
#include "Chunk.h"
#include "GameWorld.h"
#include "Sector.h"

#include "ProceduralMeshComponent.h"

AChunk::AChunk()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>("Mesh");
	SetRootComponent(MeshComponent);

	Blocks.Init(FBlockType::AIR_ID, SIZE * SIZE * HEIGHT);
}

void AChunk::Generate()
{
	for (int32 X = Position.X; X < Position.X + SIZE; ++X)
	{
		for (int32 Y = Position.Y; Y < Position.Y + SIZE; ++Y)
		{
			const int32 Height{ GetGameWorld()->ComputeHeight(FIntVector2{ X, Y }) };
	
			for (int32 Z = 0; Z <= Height; ++Z)
			{
				GetBlock(FIntVector{ X, Y, Z }) = FBlockType::Stone.ID;
			}
	
			if (Height >= SNOW_HEIGHT)
			{
				for (int32 Z = SNOW_HEIGHT; Z <= Height; ++Z)
				{
					GetBlock(FIntVector{ X, Y, Z }) = FBlockType::Snow.ID;
				}
			}
			else if (Height < ROCK_HEIGHT)
			{
				for (int32 Z = Height - DIRT_LAYER_HEIGHT + 1; Z <= Height - 1; ++Z)
				{
					GetBlock(FIntVector{ X, Y, Z }) = FBlockType::Dirt.ID;
				}
				GetBlock(FIntVector{ X, Y, Height }) = FBlockType::Grass.ID;
			}
		}
	}
}

void AChunk::CreateMesh()
{
	MeshVertices.Empty();
	MeshIndices.Empty();
	MeshNormals.Empty();
	MeshColors.Empty();

	for (int32 X = Position.X; X < Position.X + SIZE; ++X)
	{
		for (int32 Y = Position.Y; Y < Position.Y + SIZE; ++Y)
		{
			for (int32 Z = Position.Z; Z < Position.Z + HEIGHT; ++Z)
			{
				CreateBlockMesh(FIntVector{ X, Y, Z });
			}
		}
	}
}

void AChunk::CookMesh(const bool bUseAsyncCooking)
{
	checkf(IsValid(GetGameWorld()->Material), TEXT("Material was not specified."));
	MeshComponent->SetMaterial(0, GetGameWorld()->Material);
	MeshComponent->bUseAsyncCooking = bUseAsyncCooking;
	MeshComponent->CreateMeshSection(
		0,
		MeshVertices,
		MeshIndices,
		MeshNormals,
		TArray<FVector2D>{},
		MeshColors,
		TArray<FProcMeshTangent>{},
		true
	);
}

TObjectPtr<AGameWorld> AChunk::GetGameWorld()
{
	return Sector->GetGameWorld();
}

BlockTypeID& AChunk::GetBlock(const FIntVector& BlockPosition)
{
	checkf(IsBlockInBounds(BlockPosition), TEXT("Block is not within position of this chunk."));

	const FIntVector InChunkPosition{ BlockPosition - Position };

	return Blocks[InChunkPosition.Z * SIZE * SIZE + InChunkPosition.Y * SIZE + InChunkPosition.X];
}

bool AChunk::IsBlockInBounds(const FIntVector& BlockPosition) const
{
	const FIntVector TopRightFront{ Position + FIntVector{ SIZE, SIZE, HEIGHT } };

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

void AChunk::CreateBlockMesh(const FIntVector& BlockPosition)
{
	const BlockTypeID BlockTypeID = GetBlock(BlockPosition);

	if (BlockTypeID == FBlockType::AIR_ID)
	{
		return;
	}

	for (int32 i = 0; i < DIRECTION_COUNT; ++i)
	{
		const EDirection Direction{ static_cast<EDirection>(i) };
		const FIntVector NeighborPosition{ BlockPosition + static_cast<FIntVector>(GetBlockFaceNormal(Direction)) };

		if (GetGameWorld()->IsBlockAir(NeighborPosition))
		{
			const FIntVector InChunkPosition{ BlockPosition - Position };
			CreateBlockFace(static_cast<FVector>(InChunkPosition) * BLOCK_SIZE, Direction, BlockTypeID);
		}
	}
}

FVector AChunk::GetBlockFaceNormal(const EDirection Face) const
{
	switch (Face)
	{
		case EDirection::Bottom:
			return FVector{ 0, 0, -1 };
		case EDirection::Front:
			return FVector{ 0, 1, 0 };
		case EDirection::Left:
			return FVector{ -1, 0, 0 };
		case EDirection::Right:
			return FVector{ 1, 0, 0 };
		case EDirection::Back:
			return FVector{ 0, -1, 0 };
		case EDirection::Top:
			return FVector{ 0, 0, 1 };
		default:
			checkf(false, TEXT("Direction dont exist."));
			return FVector::ZeroVector;
	}
}

void AChunk::CreateBlockFace(const FVector& InChunkPosition, const EDirection Direction, const BlockTypeID BlockTypeID)
{
	const FVector Normal{ GetBlockFaceNormal(Direction) };

	for (int32 i = 0; i < FACE_VERTICES_COUNT; ++i)
	{
		const int32 VertexIndex{ BlockIndices[FACE_VERTICES_COUNT * static_cast<int32>(Direction) + i] };
		MeshVertices.Add(BlockVertices[VertexIndex] + InChunkPosition);

		MeshNormals.Add(Normal);
		MeshColors.Add(FBlockType::FromID(BlockTypeID).Color);
	}

	for (const auto FaceVertexIndex : FaceVertexIndices)
	{
		MeshIndices.Add(MeshVertices.Num() - FACE_VERTICES_COUNT + FaceVertexIndex);
	}
}
