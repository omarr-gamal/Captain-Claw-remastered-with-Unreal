// Copyright Epic Games, Inc. All Rights Reserved.

#include "ClawRemastered2GameMode.h"
#include "ClawRemastered2Character.h"

AClawRemastered2GameMode::AClawRemastered2GameMode()
{
	// Set default pawn class to our character
	DefaultPawnClass = AClawRemastered2Character::StaticClass();	
}
