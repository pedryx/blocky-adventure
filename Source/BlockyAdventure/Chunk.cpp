
#include "Chunk.h"
#include "BlockType.h"

#include "ProceduralMeshComponent.h"

AChunk::AChunk()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>("Mesh");
	SetRootComponent(MeshComponent);
}

void AChunk::BeginPlay()
{
	Super::BeginPlay();
	
	for (int32 Direction{ 0 }; Direction < 6; ++Direction)
	{
		CreateFace(FVector::ZeroVector, static_cast<EDirection>(Direction), FBlockType::Stone);
		CreateFace(FVector(200, 0, 0), static_cast<EDirection>(Direction), FBlockType::Dirt);
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

void AChunk::CreateFace(FVector Position, EDirection Direction, FBlockType BlockType)
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
