

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "R1MapGenerator.generated.h"

UCLASS()
class R1_API AR1MapGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AR1MapGenerator();

protected:
    virtual void BeginPlay() override;

public:
    // 에디터에서 방 데이터 에셋을 할당받을 변수
    UPROPERTY(EditAnywhere, Category = "Dungeon Generation")
    TObjectPtr<class URoomDataAsset> TestRoomData;

    // 방이 스폰될 위치 (테스트용)
    UPROPERTY(EditAnywhere, Category = "Dungeon Generation")
    FVector SpawnLocation;

    // 방을 실제로 스폰하는 함수
    UFUNCTION(BlueprintCallable, Category = "Dungeon Generation")
    void SpawnTestRoom();
};
