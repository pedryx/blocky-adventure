#pragma once

#include "CoreMinimal.h"
#include "Octave.generated.h"

/**
 * Represent an octave. Octaves are used for terrain generation.
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
