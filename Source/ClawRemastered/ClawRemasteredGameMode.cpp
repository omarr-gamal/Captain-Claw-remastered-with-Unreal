// Copyright Epic Games, Inc. All Rights Reserved.

#include "ClawRemasteredGameMode.h"
#include "ClawRemasteredCharacter.h"

AClawRemasteredGameMode::AClawRemasteredGameMode()
{
	// Set default pawn class to our character
	DefaultPawnClass = AClawRemasteredCharacter::StaticClass();	
}
