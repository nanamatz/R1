#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "R1ObjectPoolSystem.generated.h"
class AR1Monster;

/**
 * 
 */
USTRUCT(BlueprintType)
struct FR1MonsterPoolArray
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<AR1Monster*> Pool;
};

UCLASS()
class R1_API UR1ObjectPoolSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

private:
	// [핵심] 몬스터 종류(Class)를 Key로, 해당 몬스터들의 대기실(Array)을 Value로 갖는 창고
	// UHT는 TMap의 Value로 TArray<TObjectPtr<>>를 지원하지 않으므로, 아래와 같이 TArray<AR1Monster*>로 변경
	// TArray<AR1Monster*>는 UPROPERTY()와 함께 TMap의 Value로 사용할 수 없습니다.
	// 아래와 같이 TArray를 래핑한 USTRUCT를 만들어 사용해야 합니다.
	UPROPERTY()
	TMap<TSubclassOf<AR1Monster>, FR1MonsterPoolArray> MonsterPools;

public:
	// 스포너가 몬스터를 요구할 때 호출하는 함수
	UFUNCTION(BlueprintCallable, Category = "Object Pool")
	AR1Monster* GetMonster(TSubclassOf<AR1Monster> MonsterClass, FVector Location, FRotator Rotation);

	// 몬스터가 죽고 디졸브 처리가 끝난 뒤 대기실로 돌아올 때 호출하는 함수
	UFUNCTION(BlueprintCallable, Category = "Object Pool")
	void ReturnMonster(AR1Monster* ReturningMonster);
};
