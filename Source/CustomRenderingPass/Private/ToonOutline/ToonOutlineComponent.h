// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SkeletalMeshSceneProxy.h"
#include "ToonOutlineComponent.generated.h"

class FToonOutlineMeshSceneProxy : public FSkeletalMeshSceneProxy
{
	FToonOutlineMeshSceneProxy(const USkinnedMeshComponent* Component, FSkeletalMeshRenderData* InSkelMeshRenderData)
	: FSkeletalMeshSceneProxy(Component, InSkelMeshRenderData)
	{
		
	}
};


/**
 * 
 */
UCLASS(ClassGroup=(ToonLit), meta=(BlueprintSpawnableComponent))
class UToonOutlineComponent : public UPrimitiveComponent
{
	GENERATED_BODY()

public:
	
	CUSTOMRENDERINGPASS_API UToonOutlineComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	virtual void OnAttachmentChanged() override;
	
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;

private:

	UPROPERTY(VisibleInstanceOnly)
	TWeakObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;
};

