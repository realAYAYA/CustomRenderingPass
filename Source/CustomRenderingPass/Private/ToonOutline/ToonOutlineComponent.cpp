#include "ToonOutlineComponent.h"

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
	if (SkeletalMeshComponent.IsValid())
		return SkeletalMeshComponent.Get()->GetSceneProxy();
	
	return nullptr;
}
