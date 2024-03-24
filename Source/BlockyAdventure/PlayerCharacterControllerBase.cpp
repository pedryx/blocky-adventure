#include "PlayerCharacterControllerBase.h"
#include "Chunk.h"
#include "Sector.h"
#include "GameWorld.h"
#include "BlockType.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

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

	checkf(IsValid(ActionPlaceBlock), TEXT("ActionPlaceBlock was not specified."));
	EnhancedInputComponent->BindAction(
		ActionPlaceBlock,
		ETriggerEvent::Triggered,
		this,
		&APlayerCharacterControllerBase::HandlePlaceBlock
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

	TrySetLineTracedBlock(FBlockType::AIR_ID);
}

void APlayerCharacterControllerBase::HandlePlaceBlock(const FInputActionValue& InputActionValue)
{
	if (!PlayerCharacter)
	{
		return;
	}

	TrySetLineTracedBlock(FBlockType::Stone.ID);
}

void APlayerCharacterControllerBase::TrySetLineTracedBlock(const BlockTypeID BlockTypeID) const
{
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

	if (!bIsHit)
	{
		return;
	}

	TObjectPtr<AGameWorld> GameWorld = Cast<AChunk>(HitResult.GetActor())->GetGameWorld();

	// We need to offset impact point by some threshold, because we need to get a position within a block.
	const float Offset{ BlockTypeID == FBlockType::AIR_ID ? -1.0f : 1.0f };
	FIntVector BlockPosition{ GameWorld->GetBlockPosition(HitResult.ImpactPoint + HitResult.ImpactNormal * Offset) };

	if (!GameWorld->IsBlockInBounds(BlockPosition))
	{
		return;
	}

	GameWorld->GetBlock(BlockPosition) = BlockTypeID;
	TObjectPtr<AChunk> Chunk{ GameWorld->GetChunk(BlockPosition) };
	Chunk->CreateMesh();

	if (BlockTypeID == FBlockType::AIR_ID)
	{
		// Removed block could be at the edge of the chunk, so neighbor blocks can have missing face.
		TArray<FIntVector> Directions
		{
			FIntVector{  1,  0, 0 },
			FIntVector{  0,  1, 0 },
			FIntVector{ -1,  0, 0 },
			FIntVector{  0, -1, 0 },
		};
		for (const FIntVector& Direction : Directions)
		{
			FIntVector NeighborBlockPosition{ BlockPosition + Direction };
			if (!GameWorld->IsBlockInBounds(NeighborBlockPosition))
			{
				continue;
			}

			TObjectPtr<AChunk> NeighborChunk{ GameWorld->GetChunk(NeighborBlockPosition) };
			if (NeighborChunk != Chunk)
			{
				NeighborChunk->CreateMesh();
			}
		}
	}
}
