

#pragma once

#include "CoreMinimal.h"
#include "UI/R1UserWidget.h"
#include "R1StatRowWidget.generated.h"

class UImage;
class UTextBlock;
class UTexture2D;
/**
 * 
 */
UCLASS()
class R1_API UR1StatRowWidget : public UR1UserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_StatIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_StatInfo;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Text_StatValue;
public:
	// 툴팁에서 이 위젯을 찍어낼 때 데이터를 채워줄 함수
	void InitStatRow(UTexture2D* Icon, const FString& StatText, const FString& StatValue);
};
