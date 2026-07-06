

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

// 방 하나에 대한 '완성된 몬스터 구성' 한 벌. SpawnMonster()가 이 중 하나를 가중 랜덤 선택합니다.
USTRUCT(BlueprintType)
struct FMonsterSpawnPreset
{
	GENERATED_BODY()

	// 에디터 표시용 라벨 (배열 요소 제목으로 표시됨). 런타임 미사용.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpawnData")
	FString PresetName;

	// 이 프리셋이 선택됐을 때 스폰할 전체 구성 (기존 SpawnList와 동일 구조)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpawnData")
	TArray<FMonsterSpawnData> SpawnList;

	// 상대 선택 가중치. 전부 동일하면 균등 랜덤.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpawnData", meta = (ClampMin = "0.0"))
	float Weight = 1.0f;
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
	virtual void PostLoad() override;

public:
	// 기준점이 될 루트 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class USceneComponent> RootComp;

	// 에디터에서 자유롭게 추가할 수 있는 몬스터 스폰 세팅 목록
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	TArray<FMonsterSpawnData> SpawnList;

	// [신규] 프리셋 목록. 비어 있지 않으면 SpawnMonster()가 이 중 하나를 가중 랜덤으로 선택해 스폰합니다.
	// 비어 있으면 위의 SpawnList(레거시)를 그대로 사용합니다. 기존 맵은 수정 없이 동작합니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (TitleProperty = "PresetName"))
	TArray<FMonsterSpawnPreset> SpawnPresets;

	// 2. [핵심] 이 스포너가 소속된 방의 지휘관입니다. 
	// EditInstanceOnly를 쓰면 에디터의 '스포이드' 툴을 사용할 수 있습니다!
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Spawn")
	TObjectPtr<class ADungeonManager> DungeonManager;

	UFUNCTION(BlueprintCallable, Category = "Spawn")
	void SpawnMonster();

private:
	// 스폰 목록 하나를 실제로 스폰하는 공통 처리 (기존 SpawnMonster 본문을 추출)
	void SpawnFromList(const TArray<FMonsterSpawnData>& InSpawnList);
};
