#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PlayerCharacterControllerBase.generated.h"

class UInputAction;
class UInputMappingContext;
class UEnhancedInputComponent;
struct FInputActionValue;

UCLASS()
class BLOCKYADVENTURE_API APlayerCharacterControllerBase : public APlayerController
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, Category="Player Input")
	UInputAction* ActionMove{};

	UPROPERTY(EditDefaultsOnly, Category = "Player Input")
	UInputAction* ActionLook{};

	UPROPERTY(EditDefaultsOnly, Category = "Player Input")
	UInputAction* ActionJump{};

	UPROPERTY(EditDefaultsOnly, Category = "Player Input")
	TObjectPtr<UInputMappingContext> InputMappingContext{};

	UPROPERTY(EditDefaultsOnly, Category = "Player Input")
	float MovementSpeed{ 1.0f };

protected:
	void OnPossess(APawn* InPawn) override;
	void OnUnPossess() override;

private:
	ACharacter* PlayerCharacter{};
	UPROPERTY()
	UEnhancedInputComponent* EnhancedInputComponent{};

	void HandleMove(const FInputActionValue& InputActionValue);
	void HandleLook(const FInputActionValue& InputActionValue);
	void HandleJump(const FInputActionValue& InputActionValue);
};
