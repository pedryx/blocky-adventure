#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BlockType.h"
#include "Sector.generated.h"

class AGameWorld;
class AChunk;
class UBoxComponent;

/**
 * Represent a sector within the game world. Sector is composed out of chunks.
 */
UCLASS()
class BLOCKYADVENTURE_API ASector final : public AActor
{
	GENERATED_BODY()
	
public:	
	ASector();

	/**
	 * Number of chunks in X and Y dimensions.
	 * 
	 * @param bInIgnoreFirstOverlap Determine if first overlap end event with sector trigger should be ignored.
	 */
	inline static constexpr int32 SIZE{ 8 };

	void Initialize(
		const TObjectPtr<AGameWorld> InGameWorld, 
		const FIntVector& InPosition, 
		const bool bInShouldIgnoreFirstOverlap
	);

	/**
	 * Generate terrain for each chunk within this sector.
	 */
	void Generate();

	/**
	 * Create mesh for each chunk within this sector.
	 */
	void CreateMesh();

	/**
	 * Cook mesh for each chunk within this sector.
	 */
	void CookMesh(const bool bUseAsyncCooking);

	/**
	 * Get a game world to which this sector belongs.
	 */
	TObjectPtr<AGameWorld> GetGameWorld() const { return GameWorld; }

	/**
	 * Get chunk to which a block at a specified position belongs. Specified position must be within this sector
	 * bounds.
	 */
	TObjectPtr<AChunk> GetChunk(const FIntVector& BlockPosition);

	/**
	 * Get a reference to a block at specified position. Specified position must be in the bounds of this sector.
	 */
	BlockTypeID& GetBlock(const FIntVector& BlockPosition);

	/**
	 * Determine if a block at a specified position is within the bounds of this sector.
	 */
	bool IsBlockInBounds(const FIntVector& BlockPosition) const;
	
	/**
	 * Get the position of the left-back-down corner of the sector.
	 * 
	 * \return 
	 */
	FIntVector GetPosition() const { return Position; }

	/**
	 * Determine if sector is fully loaded with generated terrain, generated mesh and cooked up mesh.
	 */
	bool IsReady() { return bIsReady; }

	/**
	 * Store block data of the sector into the sector file.
	 */
	void SaveToFile() const;

	/**
	 * Load block data of the sector from the sector file.
	 */
	void LoadFromFile();

private:
	/**
	 * Contains all chunks, which belong to this sector. Chunks are mapped into flat array, first by X, then by Y.
	 */
	TArray<TObjectPtr<AChunk>> Chunks;
	/**
	 * Game world to which this sector belongs.
	 */
	TObjectPtr<AGameWorld> GameWorld;
	/**
	 * Position of the left-back-down corner of the sector.
	 */
	FIntVector Position;
	bool bShouldIgnoreFirstOverlap;
	/**
	 * Determine if sector is fully loaded with generated terrain, generated mesh and cooked up mesh.
	 */
	bool bIsReady{ false };

	TObjectPtr<USceneComponent> RootComponent;

	TObjectPtr<UBoxComponent> SectorTrigger;
	TObjectPtr<UBoxComponent> NorthTrigger;
	TObjectPtr<UBoxComponent> EastTrigger;
	TObjectPtr<UBoxComponent> SouthTrigger;
	TObjectPtr<UBoxComponent> WestTrigger;

	/**
	 * File name where block data will be stored.
	 */
	FString FileName;

	void CreateChunks();

	void CreateTriggers();

	void CreateTrigger(
		TObjectPtr<UBoxComponent>& OutTrigger,
		const FName& Name,
		const FVector& Position,
		const FVector& Size, 
		const bool bShouldBeginOverlap, 
		const bool bShouldEndOverlap,
		const bool bShouldBeHidden = true
	);

	UFUNCTION()
	void OnBoxBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor, 
		UPrimitiveComponent* OtherComponent, 
		int32 OtherBodyIndex, 
		bool bFromSweep, 
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnBoxEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
};
