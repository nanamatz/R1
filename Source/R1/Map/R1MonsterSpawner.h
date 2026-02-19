

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "R1MonsterSpawner.generated.h"

// [핵심] 몬스터 종류와 스폰 위치들을 묶어줄 구조체
USTRUCT(BlueprintType)
struct FMonsterSpawnData
{
	GENERATED_BODY()

	// 1. 소환할 몬스터의 종류
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpawnData")
	TSubclassOf<class AR1Monster> MonsterClass;

	// 2. 스폰 위치들 (이 배열의 크기가 곧 '스폰할 몬스터의 마릿수'가 됩니다)
	// meta = (MakeEditWidget = true) : 에디터 화면에 마우스로 드래그 가능한 3D 위젯(마름모)을 생성합니다!
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (MakeEditWidget = true), Category = "SpawnData")
	TArray<FVector> SpawnPoints;
};

UCLASS()
class R1_API AR1MonsterSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AR1MonsterSpawner();

protected:
	virtual void BeginPlay() override;

public:
	// 기준점이 될 루트 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class USceneComponent> RootComp;

	// 에디터에서 자유롭게 추가할 수 있는 몬스터 스폰 세팅 목록
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	TArray<FMonsterSpawnData> SpawnList;

	// 2. [핵심] 이 스포너가 소속된 방의 지휘관입니다. 
	// EditInstanceOnly를 쓰면 에디터의 '스포이드' 툴을 사용할 수 있습니다!
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Spawn")
	TObjectPtr<class ADungeonManager> DungeonManager;

	UFUNCTION(BlueprintCallable, Category = "Spawn")
	void SpawnMonster();
};
