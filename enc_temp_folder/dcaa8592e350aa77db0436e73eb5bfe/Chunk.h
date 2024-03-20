#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Direction.h"
#include "Chunk.generated.h"

class UProceduralMeshComponent;

UCLASS()
class BLOCKYADVENTURE_API AChunk : public AActor
{
	GENERATED_BODY()

public:	
	AChunk();

protected:
	virtual void BeginPlay() override;

private:
	TObjectPtr<UProceduralMeshComponent> MeshComponent;

	TArray<FVector> MeshVertices;
	TArray<int32> MeshIndices;
	TArray<FVector> MeshNormals;
	TArray<FLinearColor> MeshColors;

	const FVector BlockVertices[8] =
	{
		FVector(  0,   0,   0), // 0 left-back-bottom
		FVector(100,   0,   0), // 1 right-back-bottom
		FVector(  0, 100,   0), // 2 left-front-bottom
		FVector(100, 100,   0), // 3 right-front-bottom
		FVector(  0,   0, 100), // 4 left-back-top
		FVector(100,   0, 100), // 5 right-back-top
		FVector(  0, 100, 100), // 6 left-front-top
		FVector(100, 100, 100), // 7 right-front-top
	};

	const int32 BlockIndices[24] =
	{
		0, 1, 2, 3, // bottom
		2, 3, 6, 7, // front
		0, 2, 4, 6, // left
		1, 5, 3, 7, // right
		0, 4, 1, 5, // back
		4, 6, 5, 7, // top
	};

	const int32 FaceVertexIndices[6] = { 0, 1, 2, 1, 3, 2 };

	static inline constexpr int32 FACE_VERTICES_COUNT = 4;

	void CreateFace(FVector Position, EDirection Direction, FLinearColor Color);
	FVector GetFaceNormal(EDirection Face);
};
