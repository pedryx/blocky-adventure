#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Octave.h"
#include "Sector.generated.h"

class AChunk;

UCLASS()
class BLOCKYADVENTURE_API ASector : public AActor
{
	GENERATED_BODY()
	
public:	
	ASector();

	/**
	 * Material used for blocks.
	 */
	UPROPERTY(EditInstanceOnly, Category = "Sector")
	TObjectPtr<UMaterialInterface> Material;

	/**
	 * Octaves used for generating height map.
	 */
	UPROPERTY(EditInstanceOnly, Category = "Sector")
	TArray<FOctave> Octaves;

	/**
	 * Number of chunks in X and Y dimensions.
	 */
	inline static constexpr int32 SIZE{ 8 };

	/**
	 * Generate terrain for each chunk within this sector.
	 */
	void Generate();

	/**
	 * Create mesh for each chunk within this sector.
	 * 
	 */
	void CreateMesh();

	/**
	 * Determine if a block at specified position, local to this sector, is an air (empty) block.  Blocks outside of
	 * this sector are also considered as air.
	 */
	bool IsBlockAir(const FIntVector& BlockPosition) const;

	/**
	 * Compute height for block at a specified XY position, local to this sector.
	 */
	int32 ComputeHeight(const FIntVector2& BlockPosition) const;
protected:
	virtual void BeginPlay() override;

private:
	/**
	 * Contains all chunks, which belong to this sector.
	 */
	TArray<AChunk*> Chunks;

	/**
	 * Determine if a block at specified position, local to this sector, is in sector bounds.
	 */
	bool IsInBounds(const FIntVector& BlockPosition) const;
};
