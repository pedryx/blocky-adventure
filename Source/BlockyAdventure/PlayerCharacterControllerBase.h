#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlockType.h"
#include "PlayerCharacterControllerBase.generated.h"

class UInputAction;
class UInputMappingContext;
class UEnhancedInputComponent;
struct FInputActionValue;
class AGameWorld;

UCLASS(Abstract, Blueprintable)
class BLOCKYADVENTURE_API APlayerCharacterControllerBase : public APlayerController
{
	GENERATED_BODY()
public:
	APlayerCharacterControllerBase();

	#pragma region Input Properties
	UPROPERTY(EditDefaultsOnly, Category = "Input Actions")
	TObjectPtr<UInputAction> ActionMove{};

	UPROPERTY(EditDefaultsOnly, Category = "Input Actions")
	TObjectPtr<UInputAction> ActionLook{};

	UPROPERTY(EditDefaultsOnly, Category = "Input Actions")
	TObjectPtr<UInputAction> ActionJump{};

	UPROPERTY(EditDefaultsOnly, Category = "Input Actions")
	TObjectPtr<UInputAction> ActionDestroyBlock{};

	UPROPERTY(EditDefaultsOnly, Category = "PInput Actions")
	TObjectPtr<UInputAction> ActionPlaceBlock{};

	UPROPERTY(EditDefaultsOnly, Category = "Input Actions")
	TObjectPtr<UInputAction> ActionSelectSlot1;

	UPROPERTY(EditDefaultsOnly, Category = "Input Actions")
	TObjectPtr<UInputAction> ActionSelectSlot2;

	UPROPERTY(EditDefaultsOnly, Category = "Input Actions")
	TObjectPtr<UInputAction> ActionSelectSlot3;

	UPROPERTY(EditDefaultsOnly, Category = "Input Actions")
	TObjectPtr<UInputAction> ActionSelectSlot4;

	UPROPERTY(EditDefaultsOnly, Category = "Input Actions")
	TObjectPtr<UInputAction> ActionSelectSlot5;

	UPROPERTY(EditDefaultsOnly, Category = "Input Actions")
	TObjectPtr<UInputAction> ActionDebug;

	UPROPERTY(EditDefaultsOnly, Category = "Input Actions")
	TObjectPtr<UInputMappingContext> InputMappingContext{};

	UPROPERTY(EditAnywhere, Category = "Input Parameters")
	float MovementSpeed{ 1.0f };

	UPROPERTY(EditAnywhere, Category = "Input Parameters")
	float PlayerReach{ 5.0f };
	#pragma endregion

	UPROPERTY(EditDefaultsOnly, Category = "Misc")
	TSubclassOf<AActor> WireframeClass{};

	UPROPERTY(EditDefaultsOnly, Category = "Misc")
	TSubclassOf<AActor> DestructingBlockClass{};

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	TObjectPtr<ACharacter> PlayerCharacter{};
	TObjectPtr<UEnhancedInputComponent> EnhancedInputComponent{};

	/**
	 * ID of a block which is currently selected for placement.
	 */
	BlockTypeID SelectedBlockID{ 1 };
	TObjectPtr<AActor> WireframeActor{};
	
	/**
	 * Actor which will be spawned atop of block which is currently being destroyd.
	 */
	TObjectPtr<AActor> DestructingBlockActor{};
	/**
	 * Dynamic material instance of destructing actor.
	 */
	TObjectPtr<UMaterialInstanceDynamic> DestructionMaterial{};
	/**
	 * Determine if button for block destruction is being held down.
	 */
	bool bDestructionButtonDown{ false };
	/**
	 * Determine if block destruction is currently in progress.
	 */
	bool bDestructionActive{ false };
	/**
	 * Position of a block which is currently being destructed.
	 */
	FIntVector BlockBeingDestructedPosition{ FIntVector::ZeroValue };
	/**
	 * ID of a block type of a block which is currently being destructed.
	 */
	BlockTypeID BlockBeingDestructedID{};
	/**
	 * Time elapsed from the time the destruction of blocked begined in seconds.
	 */
	float DestructionAccumulator{ 0.0f };

	void InitializeInput();

	#pragma region Input Handlers
	void HandleMove(const FInputActionValue& InputActionValue);
	void HandleLook(const FInputActionValue& InputActionValue);
	void HandleJump(const FInputActionValue& InputActionValue);
	void HandleDestroyBlock(const FInputActionValue& InputActionValue);
	void HandlePlaceBlock(const FInputActionValue& InputActionValue);
	void HandleChangeSlot(const FInputActionValue& InputActionValue);
	void HandleDebug(const FInputActionValue& InputActionValue);
	#pragma endregion

	/**
	 * Result of the line trace from player into the game world.
	 */
	struct FLineTraceResults
	{
		/**
		 * Determine if line trace hit some block.
		 */
		bool bIsSuccess;
		/**
		 * Position of hitted block if any otherwise zero vector.
		 */
		FIntVector BlockPosition;
		/**
		 * Normal of hitted face of block if any block was hitted otherwise zero vector.
		 */
		FVector Normal;
		/**
		 * GameWorld to which hitted block belongs. If no block was hitted then nullptr.
		 */
		TObjectPtr<AGameWorld> GameWorld;

		/**
		 * Create instance of failed line trace.
		 */
		static FLineTraceResults Fail()
		{
			return FLineTraceResults{ false, FIntVector::ZeroValue, FVector::ZeroVector, nullptr };
		}
	};

	/**
	 * Result of current trace. Result is updated every frame.
	 */
	FLineTraceResults CurrentTrace{};

	/**
	 * Update CurrentTrace.
	 * 
	 */
	void UpdateCurrentTrace();

	/**
	 * If there is a block currently being line traced, set it to a block of a type of a specified ID.
	 */
	void TrySetLineTracedBlock(const BlockTypeID ID) const;

	/**
	 * Update position of wireframe actor into a new location based on block onto which is player currently looking.
	 */
	void UpdateWireframePosition() const;

	/**
	 * Determine if player intersect with a block at a specified position.
	 */
	bool DoPlayerIntersect(const FIntVector& BlockPosition) const;

	/**
	 * Make destruction block actor appear and move it to a new location. Then start block destruction.
	 */
	void StartBlockDestruction();
	
	/**
	 * Stop destruction of a block and hide destruction block actor.
	 */
	void StopBlockDestruction();
	
	/**
	 * Remove block which is being destroyed and stop destruction.
	 */
	void CompleteBlockDestruction();
	
	/**
	 * Update destruction porgress of a block which is currently being destroyed including destruction animation.
	 */
	void ProgressBlockDestruction(const float DeltaSeconds);
};
