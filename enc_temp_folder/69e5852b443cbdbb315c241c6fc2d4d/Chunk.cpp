
#include "Chunk.h"

#include "ProceduralMeshComponent.h"

AChunk::AChunk()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>("Mesh");
	SetRootComponent(MeshComponent);

	Blocks.Init(FBlockType::AIR_ID, TOTAL_SIZE);
}

void AChunk::BeginPlay()
{
	Super::BeginPlay();

	Generate();
	CreateMesh();
}

void AChunk::Generate()
{
	for (int32 X = 0; X < SIZE; X++)
	{
		for (int32 Y = 0; Y < SIZE; Y++)
		{
			int32 Height = HEIGHT / 2;

			for (int32 Z = 0; Z < Height - DIRT_LAYER_HEIGHT; Z++)
			{
				SetBlock(FIntVector{ X, Y, Z }, FBlockType::Stone.ID);
			}
			for (int32 Z = Height - DIRT_LAYER_HEIGHT; Z < HEIGHT; Z++)
			{
				SetBlock(FIntVector{ X, Y, Z }, FBlockType::Dirt.ID);
			}
		}
	}
}

void AChunk::CreateMesh()
{
	for (int32 X = 0; X < SIZE; X++)
	{
		for (int32 Y = 0; Y < SIZE; Y++)
		{
			for (int32 Z = 0; Z < SIZE; Z++)
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
		false
	);
}

void AChunk::TryCreateBlock(FIntVector Position)
{
	BlockTypeID ID = GetBlock(Position);

	if (ID == FBlockType::AIR_ID)
	{
		return;
	}

	const FBlockType& BlockType = FBlockType::FromID(ID);

	for (int32 i = 0; i < DIRECTION_COUNT; ++i)
	{
		EDirection Direction{ static_cast<EDirection>(i) };
		FIntVector NeighborPosition{ Position + static_cast<FIntVector>(GetFaceNormal(Direction)) };

		if (IsAir(NeighborPosition))
		{
			CreateFace(static_cast<FVector>(Position), Direction, BlockType);
		}
	}
}

void AChunk::CreateFace(FVector Position, EDirection Direction, const FBlockType& BlockType)
{
	FVector Normal = GetFaceNormal(Direction);

	for (int32 i = 0; i < FACE_VERTICES_COUNT; ++i)
	{
		int32 VertexIndex{ BlockIndices[4 * static_cast<int32>(Direction) + i] };
		MeshVertices.Add(BlockVertices[VertexIndex] + Position);

		MeshNormals.Add(Normal);
		MeshColors.Add(BlockType.Color);
	}

	for (const auto FaceVertexIndex : FaceVertexIndices)
	{
		MeshIndices.Add(MeshVertices.Num() - FACE_VERTICES_COUNT + FaceVertexIndex);
	}
}

FVector AChunk::GetFaceNormal(EDirection Face)
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
