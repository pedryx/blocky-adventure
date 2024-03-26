
#include "Chunk.h"
#include "GameWorld.h"
#include "Sector.h"

#include "ProceduralMeshComponent.h"
#include "Containers/BitArray.h"
#include "HAL/UnrealMemory.h"

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
			//const int32 Height{ 64 };
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

	const int32 BlockIndex{ GetBlockIndex(BlockPosition) };

	return Blocks[BlockIndex];
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

void AChunk::CreateMesh()
{
	MeshVertices.Empty();
	MeshIndices.Empty();
	MeshNormals.Empty();
	MeshColors.Empty();

	FMemory::Memzero(ProcessedBlocks.GetData(), ProcessedBlocks.GetAllocatedSize());

	for (int32 X = Position.X; X < Position.X + SIZE; ++X)
	{
		for (int32 Y = Position.Y; Y < Position.Y + SIZE; ++Y)
		{
			for (int32 Z = Position.Z; Z < Position.Z + HEIGHT; ++Z)
			{
				const FIntVector BlockPosition{ X, Y, Z };
				const int32 BlockIndex{ GetBlockIndex(BlockPosition) };

				StartMeshRun(BlockPosition, BlockIndex);
			}
		}
	}
}

void AChunk::StartMeshRun(const FIntVector& BlockPosition, const int32 BlockIndex)
{
	const BlockTypeID BlockTypeID = Blocks[BlockIndex];
	if (BlockTypeID == FBlockType::AIR_ID || ProcessedBlocks[BlockIndex])
	{
		return;
	}

	for (int32 FaceDirectionIndex = 0; FaceDirectionIndex < DIRECTION_COUNT; ++FaceDirectionIndex)
	{
		const EDirection FaceDirection{ static_cast<EDirection>(FaceDirectionIndex) };
		const FDirectionData FaceDirectionData{ GetDirectionData(FaceDirection, BlockPosition) };

		EDirection Direction[2];
		FDirectionData DirectionData[2];
		int32 Size[2];

		for (int DirectionIndex = 0; DirectionIndex < 2; ++DirectionIndex)
		{
			Direction[DirectionIndex] = FaceDirectionData.PerpendicularDirections[DirectionIndex];
			DirectionData[DirectionIndex] = GetDirectionData(Direction[DirectionIndex], BlockPosition);
			Size[DirectionIndex] = 0;

			for (int32 _ = DirectionData[DirectionIndex].Position; _ < DirectionData[DirectionIndex].Bound; ++_)
			{
				const int32 IndexToCheck{ BlockIndex + DirectionData[DirectionIndex].Offset * Size[DirectionIndex] };

				const bool bIsSameType{ Blocks[IndexToCheck] == BlockTypeID };
				const bool bIsAlreadyProcessed{ ProcessedBlocks[BLOCK_COUNT * FaceDirectionIndex + IndexToCheck] };
				if (!bIsSameType || bIsAlreadyProcessed)
				{
					break;
				}
				
				const FIntVector PositionToCheck
				{
					BlockPosition
						+ DirectionData[DirectionIndex].Normal * Size[DirectionIndex]
						+ FaceDirectionData.Normal
				};
				if (IsBlockInBounds(PositionToCheck))
				{
					if (Blocks[IndexToCheck + FaceDirectionData.Offset] != FBlockType::AIR_ID)
					{
						break;
					}
				}
				else if (!GetGameWorld()->IsBlockAir(PositionToCheck))
				{
					break;
				}

				ProcessedBlocks[BLOCK_COUNT * FaceDirectionIndex + IndexToCheck] = true;
				Size[DirectionIndex]++;
			}

			if (Size[DirectionIndex] > 1)
			{
				break;
			}
		}

		int32 BestDirectionIndex{ Size[1] > Size[0] ? 1 : 0 };

		if (Size[BestDirectionIndex] > 0)
		{
			const FVector InChunkBlockPosition{ static_cast<FVector>(BlockPosition - Position) };

			for (int32 i = 0; i < FACE_VERTICES_COUNT; ++i)
			{
				const int32 VertexIndex{ BlockIndices[FACE_VERTICES_COUNT * static_cast<int32>(FaceDirection) + i] };
				const FVector Scale
				{
					FVector::OneVector 
						+ static_cast<FVector>(DirectionData[BestDirectionIndex].Normal)
						* (Size[BestDirectionIndex] - 1)
				};
				const FVector Vertex{ BlockVertices[VertexIndex] * Scale + InChunkBlockPosition * BLOCK_SIZE };

				MeshVertices.Add(Vertex);
				MeshNormals.Add(static_cast<FVector>(FaceDirectionData.Normal));
				MeshColors.Add(FBlockType::FromID(BlockTypeID).Color);
			}

			for (const auto FaceVertexIndex : FaceVertexIndices)
			{
				MeshIndices.Add(MeshVertices.Num() - FACE_VERTICES_COUNT + FaceVertexIndex);
			}
		}
	}
}

int32 AChunk::GetBlockIndex(const FIntVector& BlockPosition) const
{
	const FIntVector InChunkPosition{ BlockPosition - Position };

	return InChunkPosition.Z * SIZE * SIZE + InChunkPosition.Y * SIZE + InChunkPosition.X;
}

AChunk::FDirectionData AChunk::GetDirectionData(const EDirection Direction, const FIntVector& BlockPosition) const
{
	const FIntVector InChunkPosition{ BlockPosition - Position };

	switch (Direction)
	{
	case EDirection::Bottom:
		return FDirectionData
		{
			FIntVector{ 0, 0, -1 }, 0, -SIZE * SIZE, InChunkPosition.Z,
			{ EDirection::Right, EDirection::Front }
		};
	case EDirection::Front:
		return FDirectionData
		{
			FIntVector{ 0, 1, 0 }, SIZE, SIZE, InChunkPosition.Y,
			{ EDirection::Right, EDirection::Top }
		};
	case EDirection::Left:
		return FDirectionData
		{
			FIntVector{ -1, 0, 0 }, 0, -1, InChunkPosition.X,
			{ EDirection::Top, EDirection::Front }
		};
	case EDirection::Right:
		return FDirectionData
		{
			FIntVector{ 1, 0, 0 }, SIZE, 1, InChunkPosition.X,
			{ EDirection::Top, EDirection::Front }
		};
	case EDirection::Back:
		return FDirectionData
		{
			FIntVector{ 0, -1, 0 }, 0, -SIZE, InChunkPosition.Y,
			{ EDirection::Right, EDirection::Top }
		};
	case EDirection::Top:
		return FDirectionData
		{
			FIntVector{ 0, 0, 1 }, HEIGHT, SIZE * SIZE, InChunkPosition.Z,
			{ EDirection::Right, EDirection::Front  }
		};
	default:
		checkf(false, TEXT("Direction dont exist."));
		return {};
	}
}
