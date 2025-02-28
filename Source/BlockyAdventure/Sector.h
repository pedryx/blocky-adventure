#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BlockPtr.h"
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
	 */
	inline static constexpr int32 SIZE{ 8 };

	/**
	 * Initialize this sector.
	 * 
	 * \param InGameWorld Game world to which this sector belongs.
	 * \param InPosition Block position of the most left-back-down block of this sector
	 * \param bInShouldIgnoreFirstOverlap Determine if first overlap event with sector trigger should be ignored. Note
	 * that this trigger fires when spawned with player in its area. Trigger of this event results of deletion of this
	 * sector.
	 */
	void Initialize(
		AGameWorld* const InGameWorld, 
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
	 * 
	 * \param bUseAsyncCooking Determine if meshes should be cooked asynchrounsly.
	 */
	void CookMesh(const bool bUseAsyncCooking);

	/**
	 * Get a game world to which this sector belongs.
	 */
	AGameWorld* GetGameWorld() const { return GameWorld; }

	/**
	 * Get chunk to which a block at a specified block position belongs. Specified block position must be within this
	 * sector bounds.
	 */
	AChunk* GetChunk(const FIntVector& BlockPosition);

	/**
	 * Get a pointer to a block at a specified block position. Specified block position must be within the bounds of
	 * this sector.
	 */
	FBlockPtr GetBlock(const FIntVector& BlockPosition);

	/**
	 * Get a pointer to a block at a specified block position. Specified block position must be within the bounds of
	 * this sector.
	 */
	const FBlockPtr GetBlock(const FIntVector& BlockPosition) const;

	/**
	 * Determine if a block at a specified block position is within the bounds of this sector.
	 */
	bool IsBlockInBounds(const FIntVector& BlockPosition) const;
	
	/**
	 * Get the block position of the mostleft-back-down block of the sector.
	 */
	FIntVector GetPosition() const { return Position; }

	/**
	 * Determine if sector is fully loaded and if it has generated terrain, generated mesh and cooked mesh.
	 */
	bool IsReady() const { return bIsReady; }

	/**
	 * Store block data of the sector into the sector file.
	 */
	void SaveToFile() const;

	/**
	 * Load block data of the sector from the sector file.
	 */
	void LoadFromFile();

	/**
	 * Determine if file with sector's block data exists.
	 */
	bool DoSectorFileExists() const;

private:
	/**
	 * Contains all chunks within this sector. Chunks are mapped into flat array, first by X, then by Y.
	 */
	UPROPERTY()
	TArray<TObjectPtr<AChunk>> Chunks;
	/**
	 * Game world to which this sector belongs.
	 */
	UPROPERTY()
	TObjectPtr<AGameWorld> GameWorld;
	/**
	 * Block position of the most left-back-down block of the sector.
	 */
	FIntVector Position;
	/**
	 * Determine if first overlap event with sector trigger should be ignored. Note that this trigger fires when
	 * spawned with player in its area. Trigger of this event results of deletion of this sector.
	 */
	bool bShouldIgnoreFirstOverlap;
	/**
	 * Determine if sector is fully loaded and if it has generated terrain, generated mesh and cooked mesh.
	 */
	bool bIsReady{ false };

	/**
	 * Trigger which despawns current sector upong overlap end.
	 */
	UPROPERTY()
	TObjectPtr<UBoxComponent> SectorTrigger;
	/**
	 * Trigger which spawn sector north to this sector upond overlap begin.
	 */
	UPROPERTY()
	TObjectPtr<UBoxComponent> NorthTrigger;
	/**
	 * Trigger which spawn sector east to this sector upond overlap begin.
	 */
	UPROPERTY()
	TObjectPtr<UBoxComponent> EastTrigger;
	/**
	 * Trigger which spawn sector south to this sector upond overlap begin.
	 */
	UPROPERTY()
	TObjectPtr<UBoxComponent> SouthTrigger;
	/**
	 * Trigger which spawn sector west to this sector upond overlap begin.
	 */
	UPROPERTY()
	TObjectPtr<UBoxComponent> WestTrigger;

	/**
	 * File name of the sector file where block data will be stored.
	 */
	FString FileName;

	/**
	 * Create chunk actors for chunks within this sector.
	 */
	void CreateChunks();

	/**
	 * Create triggers for sectors spawning and despawning.
	 */
	void CreateTriggers();

	/**
	 * Create a trigger for sectors spawning and despawning.
	 * 
	 * \param OutTrigger Spawned trigger.
	 * \param Name Name which will be given to created trigger.
	 * \param Position Position of the center of the trigger.
	 * \param Size Size of the trigger.
	 * \param bShouldBeginOverlap Determine if trigger should fire begin overlap events, which results in sectors
	 * being spawned.
	 * \param bShouldEndOverlap Determine if trigger should fire end overlap events, which results in this sector being
	 * despawned.
	 * \param bShouldBeHidden Determine if trigger outline should be hidden.
	 */
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
