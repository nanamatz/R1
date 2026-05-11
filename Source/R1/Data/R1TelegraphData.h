#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "R1Define.h"
#include "R1TelegraphData.generated.h"

UCLASS()
class R1_API UR1TelegraphData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Telegraph")
	ER1TelegraphShape Shape = ER1TelegraphShape::Circle;

	/** Circle: X=Radius, Cone: X=Radius Y=Angle, Rectangle: X=Length Y=Width */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Telegraph")
	FVector2D TelegraphSize = FVector2D(500.f, 500.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Telegraph")
	float Duration = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Telegraph")
	TObjectPtr<UMaterialInterface> DecalMaterial;
};
