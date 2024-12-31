// Copyright Epic Games, Inc. All Rights Reserved.

#include "CustomRenderingPassModule.h"
#include "ToonOutline/ToonOutlineRendering.h"

#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FCustomRenderingPassModule"

IRendererModule* CachedRendererModule = nullptr;
IRendererModule& GetRendererModule()
{
	if (!CachedRendererModule)
	{
		CachedRendererModule = &FModuleManager::LoadModuleChecked<IRendererModule>(TEXT("Renderer"));
	}

	return *CachedRendererModule;
}

void FCustomRenderingPassModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	const FString VertexShaderPath = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("CustomRenderingPass"))->GetBaseDir(), TEXT("Shaders"));
	AddShaderSourceDirectoryMapping(TEXT("/Plugin/CustomRenderingPass"), VertexShaderPath);

	ToonOutlineRenderer = MakeUnique<FToonOutlineRenderer>();
	IRendererModule& RendererModule = GetRendererModule();
	ToonOutlineHandle = RendererModule.RegisterPostOpaqueRenderDelegate(FPostOpaqueRenderDelegate::CreateRaw(ToonOutlineRenderer.Get(), &FToonOutlineRenderer::Render));
}

void FCustomRenderingPassModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	IRendererModule& RendererModule = GetRendererModule();
	RendererModule.RemovePostOpaqueRenderDelegate(ToonOutlineHandle);
	ToonOutlineRenderer.Reset();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCustomRenderingPassModule, CustomRenderingPass)