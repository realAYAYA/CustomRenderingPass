#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "CustomRenderingSetting.generated.h"

/**
 * 
 */
UCLASS(config = CustomRendering, defaultconfig, meta=(DisplayName="CustomRendering"), MinimalAPI)
class UCustomRenderingSetting : public UDeveloperSettings
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(config, EditAnywhere, Category = "ToonLit", meta=(ToolTip = "Enable ToonOutline(backface)"))
	bool bEnableToonOutline = true;
	
	UPROPERTY(config, EditAnywhere, Category = "ToonLit")
	TSoftObjectPtr<UMaterialInterface> ToonOutlineMaterial;
	
	//virtual FName GetContainerName() const override;
	virtual FName GetCategoryName() const override;
	//virtual FText GetSectionText() const override;
	
	static UCustomRenderingSetting* Get();

protected:
	
#if WITH_EDITOR
	virtual void PreEditChange(FProperty* PropertyAboutToChange) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif
};
