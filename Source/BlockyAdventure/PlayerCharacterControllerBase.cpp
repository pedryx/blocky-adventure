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
#include "UObject/ConstructorHelpers.h"

APlayerCharacterControllerBase::APlayerCharacterControllerBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

void APlayerCharacterControllerBase::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	PlayerCharacter = Cast<ACharacter>(InPawn);
	checkf(IsValid(PlayerCharacter), TEXT("PlayerCharacterController shpuld only posses ACharacter."));

	InitializeInput();

	checkf(IsValid(WireframeClass), TEXT("WireframeClass was not specified."));
	WireframeActor = GetWorld()->SpawnActor(WireframeClass);
	WireframeActor->SetActorHiddenInGame(true);

	checkf(IsValid(DestructingBlockClass), TEXT("DestructingBlockClass was not specified."));
	DestructingBlockActor = GetWorld()->SpawnActor(DestructingBlockClass);
	DestructingBlockActor->SetActorHiddenInGame(true);

	DestructionMaterial = UMaterialInstanceDynamic::Create(
		DestructingBlockActor->GetComponentByClass<UStaticMeshComponent>()->GetMaterial(0),
		nullptr
	);
	checkf(IsValid(DestructionMaterial), TEXT("Unable to create dynamic material instance."));
	DestructingBlockActor->GetComponentByClass<UStaticMeshComponent>()->SetMaterial(0, DestructionMaterial);
}

void APlayerCharacterControllerBase::OnUnPossess()
{
	Super::OnUnPossess();

	InputComponent->ClearActionBindings();
}

void APlayerCharacterControllerBase::Tick(float DeltaSeconds)
{
	UpdateCurrentTrace();
	UpdateWireframePosition();

	if (bDestructionButtonDown)
	{
		if (CurrentTrace.bIsSuccess && CurrentTrace.BlockPosition != BlockBeingDestructedPosition)
		{
			StartBlockDestruction();
		}
		else if (!CurrentTrace.bIsSuccess)
		{
			StopBlockDestruction();
		}
	}

	ProgressBlockDestruction(DeltaSeconds);
}

void APlayerCharacterControllerBase::InitializeInput()
{
	EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	checkf(IsValid(EnhancedInputComponent), TEXT("Unable to get reference to the UEnhancedInputComponent."));

	auto* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	checkf(IsValid(InputSubsystem), TEXT("Unable to get reference to the UEnhancedInputLocalPlayerSubsystem."));

	checkf(InputMappingContext, TEXT("InputMappingContext was not specified."));
	InputSubsystem->ClearAllMappings();
	InputSubsystem->AddMappingContext(InputMappingContext, 0);

	#pragma region Bind Input Actions
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

	checkf(IsValid(ActionSelectSlot1), TEXT("ActionSelectSlot1 was not specified."));
	EnhancedInputComponent->BindAction(
		ActionSelectSlot1,
		ETriggerEvent::Triggered,
		this,
		&APlayerCharacterControllerBase::HandleChangeSlot
	);

	checkf(IsValid(ActionSelectSlot2), TEXT("ActionSelectSlot2 was not specified."));
	EnhancedInputComponent->BindAction(
		ActionSelectSlot2,
		ETriggerEvent::Triggered,
		this,
		&APlayerCharacterControllerBase::HandleChangeSlot
	);

	checkf(IsValid(ActionSelectSlot3), TEXT("ActionSelectSlot3 was not specified."));
	EnhancedInputComponent->BindAction(
		ActionSelectSlot3,
		ETriggerEvent::Triggered,
		this,
		&APlayerCharacterControllerBase::HandleChangeSlot
	);

	checkf(IsValid(ActionSelectSlot4), TEXT("ActionSelectSlot4 was not specified."));
	EnhancedInputComponent->BindAction(
		ActionSelectSlot4,
		ETriggerEvent::Triggered,
		this,
		&APlayerCharacterControllerBase::HandleChangeSlot
	);

	checkf(IsValid(ActionSelectSlot5), TEXT("ActionSelectSlot5 was not specified."));
	EnhancedInputComponent->BindAction(
		ActionSelectSlot5,
		ETriggerEvent::Triggered,
		this,
		&APlayerCharacterControllerBase::HandleChangeSlot
	);

	checkf(IsValid(ActionDebug), TEXT("ActionDebug was not specified."));
	EnhancedInputComponent->BindAction(
		ActionDebug,
		ETriggerEvent::Triggered,
		this,
		&APlayerCharacterControllerBase::HandleDebug
	);
	#pragma endregion
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
	
	bDestructionButtonDown = InputActionValue.Get<bool>();

	if (bDestructionButtonDown)
	{
		StartBlockDestruction();
	}
	else
	{
		StopBlockDestruction();
	}
}

void APlayerCharacterControllerBase::HandlePlaceBlock(const FInputActionValue& InputActionValue)
{
	if (!PlayerCharacter)
	{
		return;
	}

	if (SelectedBlockID == FBlockType::AIR_ID)
	{
		return;
	}

	TrySetLineTracedBlock(SelectedBlockID);
}

void APlayerCharacterControllerBase::HandleChangeSlot(const FInputActionValue& InputActionValue)
{
	if (!PlayerCharacter)
	{
		return;
	}

	const float Value{ InputActionValue.Get<float>() };
	SelectedBlockID = static_cast<BlockTypeID>(Value) - 1;
	UpdateWireframePosition();

	UE_LOG(LogTemp, Warning, TEXT("Select block with ID %d"), SelectedBlockID);
}

void APlayerCharacterControllerBase::HandleDebug(const FInputActionValue& InputActionValue)
{
	UE_LOG(LogTemp, Warning, TEXT("DEBUG!"));

	if (CurrentTrace.GameWorld != nullptr)
	{
		static bool bSpawned{ true };

		if (bSpawned)
			CurrentTrace.GameWorld->DespawnSector(FIntVector{ -1, 0, 0 });
		else
			CurrentTrace.GameWorld->SpawnSector(FIntVector{ -1, 0, 0 });

		bSpawned = !bSpawned;
	}
}

void APlayerCharacterControllerBase::UpdateCurrentTrace()
{
	FVector CameraLocation;
	FRotator CameraRotator;
	GetActorEyesViewPoint(CameraLocation, CameraRotator);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(PlayerCharacter);
	Params.AddIgnoredActor(WireframeActor);
	Params.AddIgnoredActor(DestructingBlockActor);

	FHitResult HitResult;
	const FVector EndPoint{ CameraLocation + CameraRotator.Vector() * PlayerReach * AChunk::BLOCK_SIZE };

	const bool bIsHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		CameraLocation,
		EndPoint,
		ECollisionChannel::ECC_WorldStatic,
		Params
	);

	if (!bIsHit)
	{
		CurrentTrace = FLineTraceResults::Fail();
		return;
	}

	const TObjectPtr<AChunk> Chunk{ Cast<AChunk>(HitResult.GetActor()) };
	checkf(IsValid(Chunk), TEXT("Can cast only chunk actors."));

	const TObjectPtr<AGameWorld> GameWorld{ Chunk->GetGameWorld() };

	const FIntVector BlockPosition{ GameWorld->GetBlockPosition(HitResult.ImpactPoint - HitResult.ImpactNormal) };

	if (!GameWorld->IsBlockInBounds(BlockPosition))
	{
		CurrentTrace = FLineTraceResults::Fail();
		return;
	}

	// The block was destroyed and is set to air, but physics of the chunk is not yet updated.
	if (BlockBeingDestructedPosition == BlockPosition &&  GameWorld->IsBlockAir(BlockPosition))
	{
		CurrentTrace = FLineTraceResults::Fail();
		return;
	}

  	checkf(!GameWorld->IsBlockAir(BlockPosition), TEXT("Air cannot be line traced."));

	CurrentTrace = FLineTraceResults{ true, BlockPosition, HitResult.ImpactNormal, GameWorld };
}

void APlayerCharacterControllerBase::TrySetLineTracedBlock(const BlockTypeID BlockTypeID) const
{
	if (!CurrentTrace.bIsSuccess)
	{
		return;
	}

	// Block could be placed inside player.
	if (BlockTypeID != FBlockType::AIR_ID && DoPlayerIntersect(CurrentTrace.BlockPosition))
	{
		return;
	}

	FIntVector BlockPosition{ CurrentTrace.BlockPosition  };
	if (BlockTypeID != FBlockType::AIR_ID)
	{
		BlockPosition += static_cast<FIntVector>(CurrentTrace.Normal);
	}

	CurrentTrace.GameWorld->GetBlock(BlockPosition) = BlockTypeID;
	TObjectPtr<AChunk> Chunk{ CurrentTrace.GameWorld->GetChunk(BlockPosition) };
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
			if (!CurrentTrace.GameWorld->IsBlockInBounds(NeighborBlockPosition))
			{
				continue;
			}

			TObjectPtr<AChunk> NeighborChunk{ CurrentTrace.GameWorld->GetChunk(NeighborBlockPosition) };
			if (NeighborChunk != Chunk)
			{
				NeighborChunk->CreateMesh();
			}
		}
	}

	UpdateWireframePosition();
}

void APlayerCharacterControllerBase::UpdateWireframePosition() const
{
	const FIntVector BlockPosition{ CurrentTrace.BlockPosition + static_cast<FIntVector>(CurrentTrace.Normal) };

	bool bShouldHideWireframe
	{
		!CurrentTrace.bIsSuccess 
			|| SelectedBlockID == FBlockType::AIR_ID
			|| DoPlayerIntersect(BlockPosition)
	};
	if (bShouldHideWireframe)
	{
		WireframeActor->SetActorHiddenInGame(true);
		return;
	}

	const FVector NewWireframePosition
	{
		static_cast<FVector>(BlockPosition) * AChunk::BLOCK_SIZE
			+ FVector{ AChunk::BLOCK_SIZE, AChunk::BLOCK_SIZE, AChunk::BLOCK_SIZE } / 2
	};

	WireframeActor->SetActorHiddenInGame(false);
	FTransform Transform{ WireframeActor->GetActorTransform() };
	Transform.SetTranslation(NewWireframePosition);
	WireframeActor->SetActorTransform(Transform);
}

bool APlayerCharacterControllerBase::DoPlayerIntersect(const FIntVector& BlockPosition) const
{
	const FVector MinPoint{ static_cast<FVector>(BlockPosition) };
	const FVector MaxPoint{ MinPoint + FVector{ 1, 1, 1 } };
	const FBox BlockBox{ MinPoint * AChunk::BLOCK_SIZE, MaxPoint * AChunk::BLOCK_SIZE };

	return PlayerCharacter->GetCapsuleComponent()->Bounds.GetBox().Intersect(BlockBox);
}

void APlayerCharacterControllerBase::StartBlockDestruction()
{
	if (!CurrentTrace.bIsSuccess)
	{
		return;
	}

	BlockBeingDestructedPosition = CurrentTrace.BlockPosition;
	bDestructionActive = true;
	DestructionAccumulator = 0.0f;

	const FVector NewDestructingBlockPosition
	{
		static_cast<FVector>(CurrentTrace.BlockPosition) * AChunk::BLOCK_SIZE
			+ FVector{ AChunk::BLOCK_SIZE, AChunk::BLOCK_SIZE, AChunk::BLOCK_SIZE } / 2
	};

	DestructingBlockActor->SetActorHiddenInGame(false);
	FTransform Transform{ DestructingBlockActor->GetActorTransform() };
	Transform.SetTranslation(NewDestructingBlockPosition);
	DestructingBlockActor->SetActorTransform(Transform);

	BlockBeingDestructedID = CurrentTrace.GameWorld->GetBlock(CurrentTrace.BlockPosition);

	const FColor Color{ FBlockType::FromID(BlockBeingDestructedID).Color };
	DestructionMaterial->SetVectorParameterValue(TEXT("Color"), Color.ReinterpretAsLinear());
	DestructionMaterial->SetScalarParameterValue(TEXT("Progress"), 0.0f);
}

void APlayerCharacterControllerBase::StopBlockDestruction()
{
	bDestructionActive = false;
	DestructingBlockActor->SetActorHiddenInGame(true);
}

void APlayerCharacterControllerBase::CompleteBlockDestruction()
{
	TrySetLineTracedBlock(FBlockType::AIR_ID);
	StopBlockDestruction();
}

void APlayerCharacterControllerBase::ProgressBlockDestruction(const float DeltaSeconds)
{
	if (!bDestructionActive)
	{
		return;
	}

	DestructionAccumulator += DeltaSeconds;
	const float Progress{ DestructionAccumulator / FBlockType::FromID(BlockBeingDestructedID).DestructionTime };

	if (Progress >= 1.0f)
	{
		CompleteBlockDestruction();
		return;
	}

	DestructionMaterial->SetScalarParameterValue(TEXT("Progress"), Progress);
}
