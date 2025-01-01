// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ToonOutlineComponent.generated.h"

/**
 * Abendont now
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

	TSet<FPrimitiveComponentId> ComponentIds;
};

