// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Octave.generated.h"

/**
 * Octaves used for random noise generation.
 */
USTRUCT(BlueprintType)
struct BLOCKYADVENTURE_API FOctave
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Noise settings")
	double Weight{ 1.0 };

	UPROPERTY(EditAnywhere, Category = "Noise settings")
	double Frequency{ 1.0 };
};