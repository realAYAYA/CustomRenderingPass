// Copyright Epic Games, Inc. All Rights Reserved.

#include "CustomRenderingPassModule.h"
#include "CustomRenderingSetting.h"
#include "ToonOutline/ToonOutlineRendering.h"

#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FCustomRenderingPassModule"

void FCustomRenderingPassModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	const FString VertexShaderPath = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("CustomRenderingPass"))->GetBaseDir(), TEXT("Shaders"));
	AddShaderSourceDirectoryMapping(TEXT("/Plugin/CustomRenderingPass"), VertexShaderPath);

	FWorldDelegates::OnPreWorldInitialization.AddRaw(this, &FCustomRenderingPassModule::OnWorldInit);
	
	ToonOutlineRenderer = MakeUnique<FToonOutlineRenderer>();
}

void FCustomRenderingPassModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	ToonOutlineRenderer->Setup(true);
	ToonOutlineRenderer.Reset();
	
	FWorldDelegates::OnPreWorldInitialization.RemoveAll(this);
}

void FCustomRenderingPassModule::OnWorldInit(UWorld* InWorld, const UWorld::InitializationValues InInitializationValues)
{
	UCustomRenderingSetting::Get()->ToonOutlineMaterial.LoadSynchronous();
	ToonOutlineRenderer->Setup();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCustomRenderingPassModule, CustomRenderingPass)