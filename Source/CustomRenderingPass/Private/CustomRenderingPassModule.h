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

private:

	TUniquePtr<FToonOutlineRenderer> ToonOutlineRenderer;
	FDelegateHandle ToonOutlineHandle;
};
