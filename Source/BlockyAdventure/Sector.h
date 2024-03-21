#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
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
	 * Number of chunks in X and Y dimensions.
	 */
	inline static constexpr int32 SIZE{ 8 };

	/**
	 * Determine if a block at specified position, local to this sector, is an air (empty) block.
	 */
	bool IsBlockAir(const FIntVector& BlockPosition) const;
protected:
	virtual void BeginPlay() override;

private:
	TArray<AChunk*> Chunks;

	/**
	 * Determine if a block at specified position, local to this sector, is in sector bounds.
	 */
	bool IsInBounds(const FIntVector& BlockPosition) const;
};
