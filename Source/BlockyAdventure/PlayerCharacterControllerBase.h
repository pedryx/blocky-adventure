#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlockType.h"
#include "PlayerCharacterControllerBase.generated.h"

class UInputAction;
class UInputMappingContext;
class UEnhancedInputComponent;
struct FInputActionValue;

UCLASS(Abstract, Blueprintable)
class BLOCKYADVENTURE_API APlayerCharacterControllerBase : public APlayerController
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, Category="Player Input")
	TObjectPtr<UInputAction> ActionMove{};

	UPROPERTY(EditDefaultsOnly, Category = "Player Input")
	TObjectPtr<UInputAction> ActionLook{};

	UPROPERTY(EditDefaultsOnly, Category = "Player Input")
	TObjectPtr<UInputAction> ActionJump{};

	UPROPERTY(EditDefaultsOnly, Category = "Player Input")
	TObjectPtr<UInputAction> ActionDestroyBlock{};

	UPROPERTY(EditDefaultsOnly, Category = "Player Input")
	TObjectPtr<UInputAction> ActionPlaceBlock{};

	UPROPERTY(EditDefaultsOnly, Category = "Player Input")
	TObjectPtr<UInputAction> ActionSelectSlot1;

	UPROPERTY(EditDefaultsOnly, Category = "Player Input")
	TObjectPtr<UInputAction> ActionSelectSlot2;

	UPROPERTY(EditDefaultsOnly, Category = "Player Input")
	TObjectPtr<UInputAction> ActionSelectSlot3;

	UPROPERTY(EditDefaultsOnly, Category = "Player Input")
	TObjectPtr<UInputAction> ActionSelectSlot4;

	UPROPERTY(EditDefaultsOnly, Category = "Player Input")
	TObjectPtr<UInputAction> ActionSelectSlot5;

	UPROPERTY(EditDefaultsOnly, Category = "Player Input")
	TObjectPtr<UInputMappingContext> InputMappingContext{};

	UPROPERTY(EditAnywhere, Category = "Player Input")
	float MovementSpeed{ 1.0f };

	UPROPERTY(EditAnywhere, Category = "Player Input")
	float PlayerReach{ 5.0f };

	UPROPERTY(EditAnywhere, Category = "Player Input")
	TSubclassOf<AActor> WireframeClass{};

protected:
	void OnPossess(APawn* InPawn) override;
	void OnUnPossess() override;

private:
	TObjectPtr<ACharacter> PlayerCharacter{};
	TObjectPtr<UEnhancedInputComponent> EnhancedInputComponent{};
	BlockTypeID SelectedBlockID{ 1 };
	TObjectPtr<AActor> WireframeActor{};

	void HandleMove(const FInputActionValue& InputActionValue);
	void HandleLook(const FInputActionValue& InputActionValue);
	void HandleJump(const FInputActionValue& InputActionValue);
	void HandleDestroyBlock(const FInputActionValue& InputActionValue);
	void HandlePlaceBlock(const FInputActionValue& InputActionValue);
	void HandleChangeSlot(const FInputActionValue& InputActionValue);

	/**
	 * Line trace from player to game world.
	 */
	bool TryLineTraceFromPlayer(FHitResult& OutHitResult) const;

	/**
	 * Line trace from player into the world. When line trace succed get a block at result position and set it to a
	 * block of a type of a specified ID.
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
};
