#include "CustomRenderingSetting.h"

UCustomRenderingSetting::UCustomRenderingSetting(const FObjectInitializer& ObjectInitlaizer)
	: Super(ObjectInitlaizer)
{
	
}

FName UCustomRenderingSetting::GetCategoryName() const
{
	return TEXT("Plugins");
}

UCustomRenderingSetting* UCustomRenderingSetting::Get()
{
	static UCustomRenderingSetting* Instance;
	if (!Instance)
	{
		Instance = GetMutableDefault<UCustomRenderingSetting>();
	}
	
	return Instance;
}





#if WITH_EDITOR
#include "CustomRenderingPassModule.h"
#include "ToonOutline/ToonOutlineRendering.h"

FCustomRenderingPassModule& GetThisModule()
{
	return FModuleManager::LoadModuleChecked<FCustomRenderingPassModule>(TEXT("CustomRenderingPass"));
}

void UCustomRenderingSetting::PreEditChange(FProperty* PropertyAboutToChange)
{
	Super::PreEditChange(PropertyAboutToChange);
	
}

void UCustomRenderingSetting::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	if (PropertyChangedEvent.Property)
	{
		const FName PropertyName = PropertyChangedEvent.Property->GetFName();
		if (PropertyName == GET_MEMBER_NAME_CHECKED(UCustomRenderingSetting, ToonOutlineMaterial))
		{
			ToonOutlineMaterial.LoadSynchronous();
			GetThisModule().ToonOutlineRenderer.Get()->Setup(true);
			GetThisModule().ToonOutlineRenderer.Get()->Setup();
		}
		else if (PropertyName == GET_MEMBER_NAME_CHECKED(UCustomRenderingSetting, bEnableToonOutline))
		{
			GetThisModule().ToonOutlineRenderer.Get()->Setup(true);
		}
	}
}

bool UCustomRenderingSetting::CanEditChange(const FProperty* InProperty) const
{
	return Super::CanEditChange(InProperty);
}
#endif

