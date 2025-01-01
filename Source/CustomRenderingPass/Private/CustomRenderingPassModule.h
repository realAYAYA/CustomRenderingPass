// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FToonOutlineRenderer;

class FCustomRenderingPassModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	TUniquePtr<FToonOutlineRenderer> ToonOutlineRenderer;

private:

	void OnWorldInit(UWorld* InWorld, const UWorld::InitializationValues InInitializationValues);
};
