
#include "UI/System/Options/R1Category_Gameplay.h"
#include "UI/System/Options/R1SettingRow_Slider.h"

void UR1Category_Gameplay::NativeConstruct()
{
	Super::NativeConstruct();

	if (WBP_Slider_MinimapOpacity)
	{
		WBP_Slider_MinimapOpacity->InitSlider(FText::FromString(TEXT("미니맵 투명도")), 0.0f, 1.0f, 1.0f);
		WBP_Slider_MinimapOpacity->OnValueChanged.AddDynamic(this, &UR1Category_Gameplay::HandleMinimapOpacityChanged);
	}
}

void UR1Category_Gameplay::HandleMinimapOpacityChanged(float Value)
{
	if (OnMinimapOpacityChanged.IsBound())
	{
		OnMinimapOpacityChanged.Broadcast(Value);
	}
}
