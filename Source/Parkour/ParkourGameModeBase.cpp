// Copyright Epic Games, Inc. All Rights Reserved.


#include "ParkourGameModeBase.h"
#include "CPlayer.h"

AParkourGameModeBase::AParkourGameModeBase()
{
	DefaultPawnClass = ACPlayer::StaticClass();
}