#include "ToonOutlineComponent.h"

#include "GPUSkinVertexFactory.h"
#include "SkeletalMeshSceneProxy.h"
#include "SkeletalRenderPublic.h"
#include "Rendering/SkeletalMeshRenderData.h"

UToonOutlineComponent::UToonOutlineComponent(const FObjectInitializer& ObjectInitializer)
{
#if UE_SERVER
	AlwaysLoadOnServer = false;
#else
	AlwaysLoadOnServer = true;
#endif
	
	AlwaysLoadOnClient = true;
}

void UToonOutlineComponent::OnAttachmentChanged()
{
	Super::OnAttachmentChanged();

	if (auto* SkeletalMesh = Cast<USkeletalMeshComponent>(GetAttachParent()))
	{
		SkeletalMeshComponent = MakeWeakObjectPtr(SkeletalMesh);
	}
	else
	{
		SkeletalMeshComponent.Reset();
	}
}

FPrimitiveSceneProxy* UToonOutlineComponent::CreateSceneProxy()
{
	if (!SkeletalMeshComponent.IsValid())
		return nullptr;

	auto* SkeletalMesh = SkeletalMeshComponent.Get();
	
	FSkeletalMeshSceneProxy* Result = nullptr;
	FSkeletalMeshRenderData* SkelMeshRenderData = SkeletalMesh->GetSkeletalMeshRenderData();

	if (CheckPSOPrecachingAndBoostPriority() && GetPSOPrecacheProxyCreationStrategy() == EPSOPrecacheProxyCreationStrategy::DelayUntilPSOPrecached)
	{
		return nullptr;
	}

	// Only create a scene proxy for rendering if properly initialized
	if (SkelMeshRenderData &&
		SkelMeshRenderData->LODRenderData.IsValidIndex(SkeletalMesh->GetPredictedLODLevel()) &&
		!SkeletalMesh->bHideSkin &&
		SkeletalMesh->MeshObject)
	{
		// Only create a scene proxy if the bone count being used is supported, or if we don't have a skeleton (this is the case with destructibles)
		int32 MinLODIndex = SkeletalMesh->ComputeMinLOD();
		int32 MaxBonesPerChunk = SkelMeshRenderData->GetMaxBonesPerSection(MinLODIndex);
		int32 MaxSupportedNumBones = SkeletalMesh->MeshObject->IsCPUSkinned() ? MAX_int32 : FGPUBaseSkinVertexFactory::GetMaxGPUSkinBones();
		if (MaxBonesPerChunk <= MaxSupportedNumBones)
		{
			Result = ::new FSkeletalMeshSceneProxy(SkeletalMesh, SkelMeshRenderData);
		}
	}
	
	return Result;
}
