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
			
		}
	}
}

bool UCustomRenderingSetting::CanEditChange(const FProperty* InProperty) const
{
	return Super::CanEditChange(InProperty);
}
#endif