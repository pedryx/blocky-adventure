#include "BlockPtr.h"
#include "Chunk.h"
#include "Sector.h"

FBlockPtr::FBlockPtr(
	BlockTypeID* const InTypeID, 
	const FIntVector& InPosition,
	AChunk* const InChunk,
	ASector* const InSector, 
	AGameWorld* const InGameWorld
) :
	Position{ InPosition },
	Chunk{ InChunk },
	Sector{ InSector },
	GameWorld{ InGameWorld },
	bIsValid{ true },
	TypeID{ InTypeID }
{}

void FBlockPtr::SetAndUpdate(const BlockTypeID ID, const bool bSaveSector, const bool bUseAsyncCooking)
{
	SetBlock(ID);

	Chunk->CreateMesh();
	Chunk->CookMesh(bUseAsyncCooking);

	if (bSaveSector)
	{
		Sector->SaveToFile();
	}
}
