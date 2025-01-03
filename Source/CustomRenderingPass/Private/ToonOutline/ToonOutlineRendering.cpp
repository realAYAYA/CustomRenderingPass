#include "ToonOutlineRendering.h"

#include "CustomRenderingSetting.h"
#include "MeshPassProcessor.inl"
#include "SimpleMeshDrawCommandPass.h"
#include "Materials/MaterialRenderProxy.h"
#include "Runtime/Renderer/Private/ScenePrivate.h"

BEGIN_SHADER_PARAMETER_STRUCT(FToonOutlineMeshPassParameters,)
	SHADER_PARAMETER_STRUCT_REF(FViewUniformShaderParameters, View)
	SHADER_PARAMETER_RDG_UNIFORM_BUFFER(FSceneUniformParameters, Scene)
	SHADER_PARAMETER_RDG_UNIFORM_BUFFER(FInstanceCullingGlobalUniforms, InstanceCulling)
	SHADER_PARAMETER_STRUCT_INCLUDE(FInstanceCullingDrawParams, InstanceCullingDrawParams)
	RENDER_TARGET_BINDING_SLOTS()
END_SHADER_PARAMETER_STRUCT()

FInt32Range GetDynamicMeshElementRange(const FViewInfo& View, uint32 PrimitiveIndex)
{
	// DynamicMeshEndIndices contains valid values only for visible primitives with bDynamicRelevance.
	if (View.PrimitiveVisibilityMap[PrimitiveIndex])
	{
		const FPrimitiveViewRelevance& ViewRelevance = View.PrimitiveViewRelevanceMap[PrimitiveIndex];
		if (ViewRelevance.bDynamicRelevance)
			return FInt32Range(View.DynamicMeshElementRanges[PrimitiveIndex].X, View.DynamicMeshElementRanges[PrimitiveIndex].Y);
	}

	return FInt32Range::Empty();
}

// Gather & Flitter MeshBatch from Scene->Primitives.
void HandleToonOutlineMeshProcessor(FToonOutlineMeshPassProcessor& MeshProcessor, FScene* Scene, const FViewInfo* View)
{
	for (int32 PrimitiveIndex = 0; PrimitiveIndex < Scene->Primitives.Num(); PrimitiveIndex++)
	{
		const FPrimitiveSceneInfo* PrimitiveSceneInfo = Scene->Primitives[PrimitiveIndex];

		if (!View->PrimitiveVisibilityMap[PrimitiveSceneInfo->GetIndex()])
			continue;

		const FPrimitiveViewRelevance& ViewRelevance = View->PrimitiveViewRelevanceMap[PrimitiveSceneInfo->GetIndex()];

		if (ViewRelevance.bRenderInMainPass && ViewRelevance.bStaticRelevance)
		{
			for (int32 StaticMeshIdx = 0; StaticMeshIdx < PrimitiveSceneInfo->StaticMeshes.Num(); StaticMeshIdx++)
			{
				const FStaticMeshBatch& StaticMesh = PrimitiveSceneInfo->StaticMeshes[StaticMeshIdx];
				if (View->StaticMeshVisibilityMap[StaticMesh.Id])
				{
					constexpr uint64 DefaultBatchElementMask = ~0ul;
					MeshProcessor.AddMeshBatch(StaticMesh, DefaultBatchElementMask, StaticMesh.PrimitiveSceneInfo->Proxy);
				}
			}
		}

		if (ViewRelevance.bRenderInMainPass && ViewRelevance.bDynamicRelevance)
		{
			const FInt32Range MeshBatchRange = GetDynamicMeshElementRange(*View, PrimitiveSceneInfo->GetIndex());
			for (int32 MeshBatchIndex = MeshBatchRange.GetLowerBoundValue(); MeshBatchIndex < MeshBatchRange.GetUpperBoundValue(); ++MeshBatchIndex)
			{
				const FMeshBatchAndRelevance& MeshAndRelevance = View->DynamicMeshElements[MeshBatchIndex];
				constexpr uint64 BatchElementMask = ~0ull;
				MeshProcessor.AddMeshBatch(*MeshAndRelevance.Mesh, BatchElementMask, MeshAndRelevance.PrimitiveSceneProxy);
			}
		}
	}
}

IRendererModule* CachedRendererModule = nullptr;
IRendererModule& GetRendererModule()
{
	if (!CachedRendererModule)
	{
		CachedRendererModule = &FModuleManager::LoadModuleChecked<IRendererModule>(TEXT("Renderer"));
	}

	return *CachedRendererModule;
}

void FToonOutlineRenderer::Setup(const bool bRelease)
{
	IRendererModule& RendererModule = GetRendererModule();
	if (!bRelease && CheckConfig())
	{
		ToonOutlineHandle = RendererModule.RegisterPostOpaqueRenderDelegate(FPostOpaqueRenderDelegate::CreateRaw(this, &FToonOutlineRenderer::Render));
	}
	else
	{
		RendererModule.RemovePostOpaqueRenderDelegate(ToonOutlineHandle);
	}
}

void FToonOutlineRenderer::Render(FPostOpaqueRenderParameters& Parameters) const
{
	if (!UCustomRenderingSetting::Get()->bEnableToonOutline)
		return;
	
	// Reference search: "RendererModule.RenderPostOpaqueExtensions(GraphBuilder, Views, SceneTextures);"
	auto* GraphBuilder = Parameters.GraphBuilder;
	auto* View = Parameters.View;
	auto* ColorTexture =  Parameters.ColorTexture;
	auto* DepthTexture = Parameters.DepthTexture;
	//auto* VelocityTexture = Parameters.VelocityTexture;
	//auto* NormalTexture = Parameters.NormalTexture;
	
	/*if (!View || View->Family->Scene == nullptr)
	{
		UE_LOG(LogTemp, Log, TEXT("%s - View.Family->Scene is NULL!"), *FString(__FUNCTION__));
		return;
	}*/

	auto* Scene = static_cast<FScene*>(View->Family->Scene);
	/*if (!Scene)
	{
		UE_LOG(LogShaders, Log, TEXT("%s - View.Family->FScene is NULL!"), *FString(__FUNCTION__));
		return;
	}*/
	
	// Render targets bindings should remain constant at this point.
	FRenderTargetBindingSlots BindingSlots;
	BindingSlots[0] = FRenderTargetBinding(ColorTexture, ERenderTargetLoadAction::ELoad);
	BindingSlots.DepthStencil = FDepthStencilBinding(
		DepthTexture,
		ERenderTargetLoadAction::ENoAction,
		ERenderTargetLoadAction::ELoad,
		FExclusiveDepthStencil::DepthWrite_StencilNop);

	FToonOutlineMeshPassParameters* PassParameters = GraphBuilder->AllocParameters<FToonOutlineMeshPassParameters>();
	PassParameters->View = View->ViewUniformBuffer;
	PassParameters->RenderTargets = BindingSlots;
	PassParameters->Scene = GetSceneUniformBufferRef(*GraphBuilder, *View);
	PassParameters->InstanceCulling = FInstanceCullingContext::CreateDummyInstanceCullingUniformBuffer(*GraphBuilder);

	// Todo 写入速度缓冲

	FIntRect ViewportRect = View->ViewRect;
	FIntRect ScissorRect = FIntRect(FIntPoint(EForceInit::ForceInitToZero), ColorTexture->Desc.Extent);

	GraphBuilder->AddPass(
		RDG_EVENT_NAME("ToonOutlinePass"),
		PassParameters,
		ERDGPassFlags::Raster,
		[this, View, ViewportRect, ScissorRect, Scene](FRHICommandList& RHICmdList)
		{
			RHICmdList.SetViewport(
				ViewportRect.Min.X, ViewportRect.Min.Y, 0.0f,
				ViewportRect.Max.X, ViewportRect.Max.Y, 1.0f);

			RHICmdList.SetScissorRect(
				true,
				ScissorRect.Min.X >= ViewportRect.Min.X ? ScissorRect.Min.X : ViewportRect.Min.X,
				ScissorRect.Min.Y >= ViewportRect.Min.Y ? ScissorRect.Min.Y : ViewportRect.Min.Y,
				ScissorRect.Max.X <= ViewportRect.Max.X ? ScissorRect.Max.X : ViewportRect.Max.X,
				ScissorRect.Max.Y <= ViewportRect.Max.Y ? ScissorRect.Max.Y : ViewportRect.Max.Y);
			
			DrawDynamicMeshPass(*View, RHICmdList, [Scene, View](FDynamicPassMeshDrawListContext* DynamicMeshPassContext)
			{
				FMeshPassProcessorRenderState DrawRenderState;
				DrawRenderState.SetDepthStencilState(TStaticDepthStencilState<true, CF_LessEqual>().GetRHI());
				FToonOutlineMeshPassProcessor MeshProcessor(Scene, View, DrawRenderState, DynamicMeshPassContext);
				HandleToonOutlineMeshProcessor(MeshProcessor, Scene, View);
			});
		});
}

bool FToonOutlineRenderer::CheckConfig()
{
	if (auto* Mat = UCustomRenderingSetting::Get()->ToonOutlineMaterial.Get())
	{
		if (auto* Resource =  Mat->GetMaterialResource(GMaxRHIFeatureLevel, EMaterialQualityLevel::Num))
			return UCustomRenderingSetting::Get()->bEnableToonOutline;
	}
	
	return false;
}


IMPLEMENT_MATERIAL_SHADER_TYPE(, FToonOutlineVS, TEXT("/Plugin/CustomRenderingPass/ToonLitOutLine.usf"), TEXT("MainVS"),
                                 SF_Vertex);
IMPLEMENT_MATERIAL_SHADER_TYPE(, FToonOutlinePS, TEXT("/Plugin/CustomRenderingPass/ToonLitOutLine.usf"), TEXT("MainPS"), SF_Pixel);

FToonOutlineMeshPassProcessor::FToonOutlineMeshPassProcessor(
	const FScene* Scene,
	const FSceneView* InViewIfDynamicMeshCommand,
	const FMeshPassProcessorRenderState& InPassDrawRenderState,
	FMeshPassDrawListContext* InDrawListContext)
	: FMeshPassProcessor(EMeshPass::Num, Scene, Scene->GetFeatureLevel(), InViewIfDynamicMeshCommand, InDrawListContext),
	  PassDrawRenderState(InPassDrawRenderState)
{
	if (PassDrawRenderState.GetDepthStencilState() == nullptr)
		PassDrawRenderState.SetDepthStencilState(TStaticDepthStencilState<false, CF_NotEqual>().GetRHI());
	if (PassDrawRenderState.GetBlendState() == nullptr)
		PassDrawRenderState.SetBlendState(TStaticBlendState<>().GetRHI());
}

void FToonOutlineMeshPassProcessor::AddMeshBatch(
	const FMeshBatch& MeshBatch,
	uint64 BatchElementMask,
	const FPrimitiveSceneProxy* PrimitiveSceneProxy,
	int32 StaticMeshId)
{
	const FMaterialRenderProxy* MaterialRenderProxy = MeshBatch.MaterialRenderProxy;
	const FMaterialRenderProxy* FallBackMaterialRenderProxyPtr = nullptr;
	const FMaterial& Material = MaterialRenderProxy->GetMaterialWithFallback(Scene->GetFeatureLevel(), FallBackMaterialRenderProxyPtr);
	if (Material.GetShadingModels() != FMaterialShadingModelField(EMaterialShadingModel::MSM_DefaultLit))
		return;// Only render outline for ToonShadingModel

	auto* Mat = UCustomRenderingSetting::Get()->ToonOutlineMaterial.Get();
	const FMaterial* OutlineMaterial = Mat->GetMaterialResource(Scene->GetFeatureLevel());
	const FMaterialRenderProxy* OutlineMaterialRenderProxy = Mat->GetRenderProxy();
	if (!OutlineMaterial || !OutlineMaterialRenderProxy)
		return;

	if (OutlineMaterial->GetRenderingThreadShaderMap())
	{
		// Determine the mesh's material and blend mode.
		if (Material.GetBlendMode() == BLEND_Opaque || Material.GetBlendMode() == BLEND_Masked)
		{
			Process<false, false>(
				MeshBatch,
				BatchElementMask,
				StaticMeshId,
				PrimitiveSceneProxy,
				*OutlineMaterialRenderProxy,
				*OutlineMaterial,
				FM_Solid,
				CM_CCW);
		}
	}
}

template <bool bPositionOnly, bool bUsesMobileColorValue>
bool FToonOutlineMeshPassProcessor::Process(
	const FMeshBatch& MeshBatch,
	uint64 BatchElementMask,
	int32 StaticMeshId,
	const FPrimitiveSceneProxy* PrimitiveSceneProxy,
	const FMaterialRenderProxy& MaterialRenderProxy,
	const FMaterial& MaterialResource,
	ERasterizerFillMode MeshFillMode,
	ERasterizerCullMode MeshCullMode)
{
	const FVertexFactory* VertexFactory = MeshBatch.VertexFactory;

	TMeshProcessorShaders<FToonOutlineVS, FToonOutlinePS> ToonOutlineShaders;
	{
		FMaterialShaderTypes ShaderTypes;
		ShaderTypes.AddShaderType<FToonOutlineVS>();
		ShaderTypes.AddShaderType<FToonOutlinePS>();

		const FVertexFactoryType* VertexFactoryType = VertexFactory->GetType();

		FMaterialShaders Shaders;
		if (!MaterialResource.TryGetShaders(ShaderTypes, VertexFactoryType, Shaders))
		{
			//UE_LOG(LogShaders, Warning, TEXT("**********************!Shader Not Found!*************************"));
			return false;
		}

		Shaders.TryGetVertexShader(ToonOutlineShaders.VertexShader);
		Shaders.TryGetPixelShader(ToonOutlineShaders.PixelShader);
	}

	FToonOutlinePassShaderElementData ShaderElementData;
	ShaderElementData.InitializeMeshMaterialData(ViewIfDynamicMeshCommand, PrimitiveSceneProxy, MeshBatch, StaticMeshId, false);
	const FMeshDrawCommandSortKey SortKey = CalculateMeshStaticSortKey(ToonOutlineShaders.VertexShader, ToonOutlineShaders.PixelShader);
	
	PassDrawRenderState.SetDepthStencilState(
		TStaticDepthStencilState<
			true, CF_GreaterEqual, // Enable DepthTest, it opposites to OpenGL(which is less)
			false, CF_Never, SO_Keep, SO_Keep, SO_Keep,
			false, CF_Never, SO_Keep, SO_Keep, SO_Keep, // enable stencil test when cull back
			0x00, // disable stencil read
			0x00> // disable stencil write
		::GetRHI());
	PassDrawRenderState.SetStencilRef(0);

	BuildMeshDrawCommands(
		MeshBatch,
		BatchElementMask,
		PrimitiveSceneProxy,
		MaterialRenderProxy,
		MaterialResource,
		PassDrawRenderState,
		ToonOutlineShaders,
		MeshFillMode,
		MeshCullMode,
		SortKey,
		EMeshPassFeatures::Default,
		ShaderElementData
	);

	return true;
}

// 修改引擎源码写法
/*// Register Pass to Global Manager
void SetupToonOutLinePassState(FMeshPassProcessorRenderState& DrawRenderState)
{
	DrawRenderState.SetBlendState(TStaticBlendState<CW_NONE>::GetRHI());

	// !
	PassDrawRenderState.SetDepthStencilState(
		TStaticDepthStencilState<
		true, CF_GreaterEqual,// Enable DepthTest, It reverse about OpenGL(which is less)
		false, CF_Never, SO_Keep, SO_Keep, SO_Keep,
		false, CF_Never, SO_Keep, SO_Keep, SO_Keep,// enable stencil test when cull back
		0x00,// disable stencil read
		0x00>// disable stencil write
		::GetRHI());
	PassDrawRenderState.SetStencilRef(0);
}

FMeshPassProcessor* CreateToonOutLinePassProcessor(
	const FScene* Scene,
	const FSceneView* InViewIfDynamicMeshCommand,
	FMeshPassDrawListContext* InDrawListContext
)
{
	FMeshPassProcessorRenderState ToonOutLinePassState;
	SetupToonOutLinePassState(ToonOutLinePassState);

	return new(FMemStack::Get()) FToonOutlineMeshPassProcessor(
		Scene,
		InViewIfDynamicMeshCommand,
		ToonOutLinePassState,
		InDrawListContext
	);
}

FRegisterPassProcessorCreateFunction RegisteToonOutLineMeshPass(
	&CreateToonOutLinePassProcessor,
	EShadingPath::Deferred,
	EMeshPass::BackfaceOutLinePass,
	EMeshPassFlags::CachedMeshCommands | EMeshPassFlags::MainView
);*/

