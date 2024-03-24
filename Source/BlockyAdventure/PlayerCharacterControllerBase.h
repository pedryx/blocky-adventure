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

protected:
	void OnPossess(APawn* InPawn) override;
	void OnUnPossess() override;

private:
	TObjectPtr<ACharacter> PlayerCharacter{};
	TObjectPtr<UEnhancedInputComponent> EnhancedInputComponent{};
	BlockTypeID SelectedBlockID{ 1 };

	void HandleMove(const FInputActionValue& InputActionValue);
	void HandleLook(const FInputActionValue& InputActionValue);
	void HandleJump(const FInputActionValue& InputActionValue);
	void HandleDestroyBlock(const FInputActionValue& InputActionValue);
	void HandlePlaceBlock(const FInputActionValue& InputActionValue);
	void HandleChangeSlot(const FInputActionValue& InputActionValue);

	/**
	 * Line cast from player into the world. When line cast succed get a block at result position and set it to a
	 * block of a type of a specified ID.
	 * 
	 * @param offset Block will be picked by hit position which can be offseted by this parameter.
	 */
	void TrySetLineTracedBlock(const BlockTypeID ID) const;
};
