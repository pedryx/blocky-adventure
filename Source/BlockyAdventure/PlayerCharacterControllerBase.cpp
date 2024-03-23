#include "PlayerCharacterControllerBase.h"
#include "Chunk.h"
#include "Sector.h"
#include "BlockType.h"

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

	checkf(IsValid(ActionDestroyBlock), TEXT("ActionDestroyBlock was not specified."));
	EnhancedInputComponent->BindAction(
		ActionDestroyBlock,
		ETriggerEvent::Triggered,
		this,
		&APlayerCharacterControllerBase::HandleDestroyBlock
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

void APlayerCharacterControllerBase::HandleDestroyBlock(const FInputActionValue& InputActionValue)
{
	if (!PlayerCharacter)
	{
		return;
	}

	FVector CameraLocation;
	FRotator CameraRotator;
	GetActorEyesViewPoint(CameraLocation, CameraRotator);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(PlayerCharacter);

	const FVector EndPoint{ CameraLocation + CameraRotator.Vector() * PlayerReach * AChunk::BLOCK_SIZE };
	FHitResult HitResult;
	
	const bool bIsHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		CameraLocation,
		EndPoint,
		ECollisionChannel::ECC_WorldStatic,
		Params
	);

	if (bIsHit)
	{
		AChunk* Chunk = Cast<AChunk>(HitResult.GetActor());
		checkf(IsValid(Chunk), TEXT("Linecast should capture only chunk."));

		const FVector LineTraceDirection{ (EndPoint - CameraLocation).GetUnsafeNormal() };
		// We need to offset impact point by some threshold, because wi need to get a position within a block.
		const FVector WorldPosition{ HitResult.ImpactPoint + LineTraceDirection};

		ASector* Sector{ Chunk->GetSector() };
		const FIntVector BlockPosition = Sector->GetBlockPosition(WorldPosition);
		Sector->SetBlock(BlockPosition, FBlockType::AIR_ID);
		Sector->GetChunk(BlockPosition)->CreateMesh();

		// We need to regenerate also chunk of block behind, because that block can have missing face,
		// We to offset the impact point by a value which will ensure that we will get block from neighbor chunk.
		const FVector BlockBehindWorldPosition{ HitResult.ImpactPoint + LineTraceDirection * AChunk::BLOCK_SIZE * 2 };
		const FIntVector BehindBlockPosition = Sector->GetBlockPosition(BlockBehindWorldPosition);
		if (Sector->IsBlockInBounds(BehindBlockPosition))
		{
			Sector->GetChunk(BehindBlockPosition)->CreateMesh();
		}
	}
}
