// Copyright Epic Games, Inc. All Rights Reserved.

#include "minijam184GameMode.h"
#include "minijam184Character.h"
#include "UObject/ConstructorHelpers.h"

Aminijam184GameMode::Aminijam184GameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
