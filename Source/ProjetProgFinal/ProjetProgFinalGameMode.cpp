// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProjetProgFinalGameMode.h"
#include "ProjetProgFinalCharacter.h"
#include "UObject/ConstructorHelpers.h"

AProjetProgFinalGameMode::AProjetProgFinalGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
