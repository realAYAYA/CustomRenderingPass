// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SkeletalMeshSceneProxy.h"
#include "ToonOutlineComponent.generated.h"

class FToonOutlineMeshSceneProxy : public FSkeletalMeshSceneProxy
{
	
public:
	
	FToonOutlineMeshSceneProxy(const USkinnedMeshComponent* Component, FSkeletalMeshRenderData* InSkelMeshRenderData)
	: FSkeletalMeshSceneProxy(Component, InSkelMeshRenderData)
	{
		if (Component)
			IsMe = true;
	}

	bool IsMe = false;
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

	UPROPERTY(VisibleAnywhere, Category = "ToonLit")
	TWeakObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;
};

