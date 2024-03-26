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

	/**
	 * Initialize this sector.
	 * 
	 * \param InGameWorld Game world to which this sector belongs.
	 * \param InPosition Block position of the lower left back corner of this sector
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
	 * \param bUseAsyncCooking Determine if mesh should be cooked asynchrounsly.
	 */
	void CookMesh(const bool bUseAsyncCooking);

	/**
	 * Get a game world to which this sector belongs.
	 */
	AGameWorld* GetGameWorld() const { return GameWorld; }

	/**
	 * Get chunk to which a block at a specified position belongs. Specified position must be within this sector
	 * bounds.
	 */
	AChunk* GetChunk(const FIntVector& BlockPosition);

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
	 * Contains all chunks, which belong to this sector. Chunks are mapped into flat array, first by X, then by Y.
	 */
	UPROPERTY()
	TArray<TObjectPtr<AChunk>> Chunks;
	/**
	 * Game world to which this sector belongs.
	 */
	UPROPERTY()
	TObjectPtr<AGameWorld> GameWorld;
	/**
	 * Position of the left-back-down corner of the sector.
	 */
	FIntVector Position;
	/**
	 * Determine if first overlap event with sector trigger should be ignored. Note that this trigger fires when
	 * spawned with player in its area. Trigger of this event results of deletion of this sector.
	 */
	bool bShouldIgnoreFirstOverlap;
	/**
	 * Determine if sector is fully loaded with generated terrain, generated mesh and cooked up mesh.
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
	 * File name where block data will be stored.
	 */
	FString FileName;

	/**
	 * Create chunk actors which belong to this sector.
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
	 * \param bShouldBeginOverlap Determine if trigger should fires begin overlap events which results in sectors
	 * being spawned.
	 * \param bShouldEndOverlap Determine if trigger should fires end overlap events which results in this sector being
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
