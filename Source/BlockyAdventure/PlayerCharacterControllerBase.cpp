#include "PlayerCharacterControllerBase.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void APlayerCharacterControllerBase::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	PlayerCharacter = Cast<ACharacter>(InPawn);
	checkf(IsValid(PlayerCharacter), TEXT("PlayerCharacterController shpuld only posses ACharacter."));

	EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	checkf(IsValid(EnhancedInputComponent), TEXT("Unable to get reference to the UEnhancedInputComponent."));

	auto* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	checkf(IsValid(InputSubsystem), TEXT("Unable to get reference to the UEnhancedInputLocalPlayerSubsystem."));

	checkf(InputMappingContext, TEXT("InputMappingContext was not specified."));
	InputSubsystem->ClearAllMappings();
	InputSubsystem->AddMappingContext(InputMappingContext, 0);

	checkf(IsValid(ActionMove), TEXT("ActionMove was not specified."));
	EnhancedInputComponent->BindAction(
		ActionMove,
		ETriggerEvent::Triggered,
		this,
		&APlayerCharacterControllerBase::HandleMove
	);

	checkf(IsValid(ActionLook), TEXT("ActionLook was not specified."));
	EnhancedInputComponent->BindAction(
		ActionLook,
		ETriggerEvent::Triggered,
		this,
		&APlayerCharacterControllerBase::HandleLook
	);

	checkf(IsValid(ActionJump), TEXT("ActionJump was not specified."));
	EnhancedInputComponent->BindAction(
		ActionJump,
		ETriggerEvent::Triggered,
		this,
		&APlayerCharacterControllerBase::HandleJump
	);
}

void APlayerCharacterControllerBase::OnUnPossess()
{
	Super::OnUnPossess();

	InputComponent->ClearActionBindings();
}

void APlayerCharacterControllerBase::HandleMove(const FInputActionValue& InputActionValue)
{
	if (!PlayerCharacter)
	{
		return;
	}

	const FVector2D MovementDirection = InputActionValue.Get<FVector2D>();

	PlayerCharacter->AddMovementInput(
		PlayerCharacter->GetActorForwardVector(),
		MovementDirection.Y * MovementSpeed,
		false
	);

	PlayerCharacter->AddMovementInput(
		PlayerCharacter->GetActorRightVector(),
		MovementDirection.X * MovementSpeed,
		false
	);
}

void APlayerCharacterControllerBase::HandleLook(const FInputActionValue& InputActionValue)
{
	if (!PlayerCharacter)
	{
		return;
	}

	const FVector2D LookDirection = InputActionValue.Get<FVector2D>();

	AddYawInput(LookDirection.X);
	AddPitchInput(-LookDirection.Y);
}

void APlayerCharacterControllerBase::HandleJump(const FInputActionValue& InputActionValue)
{
	if (!PlayerCharacter)
	{
		return;
	}

	PlayerCharacter->Jump();
}