#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "R1TelegraphActor.generated.h"

class UDecalComponent;
class UR1TelegraphData;

UCLASS()
class R1_API AR1TelegraphActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AR1TelegraphActor();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	void InitializeTelegraph(UR1TelegraphData* InData);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Telegraph")
	TObjectPtr<USceneComponent> DefaultSceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Telegraph")
	TObjectPtr<UDecalComponent> DecalComponent;

private:
	UPROPERTY()
	TObjectPtr<UR1TelegraphData> TelegraphData;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DecalMID;

	float ElapsedTime = 0.0f;
};
