
#include "Chunk.h"
#include "Sector.h"

#include "ProceduralMeshComponent.h"

AChunk::AChunk()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>("Mesh");
	SetRootComponent(MeshComponent);

	Blocks.Init(FBlockType::AIR_ID, SIZE * SIZE * HEIGHT);
}

void AChunk::BeginPlay()
{
	Super::BeginPlay();
}

void AChunk::Generate()
{
	MeshVertices.Empty();
	MeshIndices.Empty();
	MeshNormals.Empty();
	MeshColors.Empty();


	for (int32 X = 0; X < SIZE; ++X)
	{
		for (int32 Y = 0; Y < SIZE; ++Y)
		{
			const FIntVector InSectorPosition = GetInSectorPosition(FIntVector{ X, Y, 0 });
			const int32 Height = Sector->ComputeHeight(FIntVector2{ InSectorPosition.X, InSectorPosition.Y });

			for (int32 Z = 0; Z <= Height; ++Z)
			{
				SetBlock(FIntVector{ X, Y, Z }, FBlockType::Stone.ID);
			}

			if (Height >= SNOW_HEIGHT)
			{
				for (int32 Z = SNOW_HEIGHT; Z <= Height; ++Z)
				{
					SetBlock(FIntVector{ X, Y, Z }, FBlockType::Snow.ID);
				}
			}
			else if (Height < ROCK_HEIGHT)
			{
				for (int32 Z = Height - DIRT_LAYER_HEIGHT + 1; Z <= Height - 1; ++Z)
				{
					SetBlock(FIntVector{ X, Y, Z }, FBlockType::Dirt.ID);
				}
				SetBlock(FIntVector{ X, Y, Height }, FBlockType::Grass.ID);
			}
		}
	}

}

void AChunk::CreateMesh()
{
	for (int32 X = 0; X < SIZE; ++X)
	{
		for (int32 Y = 0; Y < SIZE; ++Y)
		{
			for (int32 Z = 0; Z < HEIGHT; ++Z)
			{
				TryCreateBlock(FIntVector{ X, Y, Z });
			}
		}
	}

	MeshComponent->SetMaterial(0, Material);
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

bool AChunk::IsAir(const FIntVector& BlockPosition) const
{
	if (!IsInBounds(BlockPosition)) {
		return Sector->IsBlockAir(GetInSectorPosition(BlockPosition));
	}

	return GetBlock(BlockPosition) == FBlockType::AIR_ID;
}

void AChunk::TryCreateBlock(const FIntVector& Position)
{
	const BlockTypeID ID = GetBlock(Position);

	if (ID == FBlockType::AIR_ID)
	{
		return;
	}

	const FBlockType& BlockType = FBlockType::FromID(ID);

	for (int32 i = 0; i < DIRECTION_COUNT; ++i)
	{
		const EDirection Direction{ static_cast<EDirection>(i) };
		const FIntVector NeighborPosition{ Position + static_cast<FIntVector>(GetFaceNormal(Direction)) };

		if (IsAir(NeighborPosition))
		{
			CreateFace(static_cast<FVector>(Position) * BLOCK_SIZE, Direction, BlockType);
		}
	}
}

void AChunk::CreateFace(const FVector& Position, const EDirection Direction, const FBlockType& BlockType)
{
	const FVector Normal = GetFaceNormal(Direction);

	for (int32 i = 0; i < FACE_VERTICES_COUNT; ++i)
	{
		const int32 VertexIndex{ BlockIndices[4 * static_cast<int32>(Direction) + i] };
		MeshVertices.Add(BlockVertices[VertexIndex] + Position);

		MeshNormals.Add(Normal);
		MeshColors.Add(BlockType.Color);
	}

	for (const auto FaceVertexIndex : FaceVertexIndices)
	{
		MeshIndices.Add(MeshVertices.Num() - FACE_VERTICES_COUNT + FaceVertexIndex);
	}
}

FVector AChunk::GetFaceNormal(const EDirection Face) const
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
		throw std::invalid_argument{ "Direction dont exist" };
	}
}
