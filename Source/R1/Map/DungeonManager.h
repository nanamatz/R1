

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DungeonManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoomClearedDelegate, int32, ClearedNodeID);

UENUM(BlueprintType)
enum class ER1RoomClearCondition : uint8
{
	None			UMETA(DisplayName = "조건 없음 (항상 열림)"),
	KillAllMonsters UMETA(DisplayName = "모든 몬스터 처치"),
	KillBoss		UMETA(DisplayName = "보스 처치")
};

UCLASS()
class R1_API ADungeonManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADungeonManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// 에디터에서 레벨 디자이너가 이 방의 룰을 정할 수 있게 노출합니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Setup")
	ER1RoomClearCondition ClearCondition = ER1RoomClearCondition::None;

	// [추가] 내가 관리하고 있는 방의 고유 번호 (제너레이터가 주입해 줄 예정)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Room State")
	int32 RoomNodeID = -1;

	// 클리어 시 발동될 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Room Events")
	FOnRoomClearedDelegate OnRoomCleared;

	// 숫자(int32) 대신, 현재 살아있는 몬스터들의 목록을 관리합니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Room Status")
	TSet<class AR1Character*> ActiveMonsters;

	// 몬스터가 '스스로' 자신을 등록할 때 호출하는 함수
	UFUNCTION(BlueprintCallable, Category = "Room Logic")
	void RegisterMonster(class AR1Character* MonsterToAdd);

	// 몬스터가 죽었을 때 명부에서 지우는 함수
	UFUNCTION(BlueprintCallable, Category = "Room Logic")
	void UnregisterMonster(class AR1Character* DeadCharacter, class AR1Character* Attacker);

public:
	// 이 방이 이미 클리어 되었는지 확인하는 플래그
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Room State")
	bool bIsCleared = false;

	// 문을 잠그거나 여는 함수
	UFUNCTION(BlueprintCallable, Category = "Room Logic")
	void LockRoomDoors();

	UFUNCTION(BlueprintCallable, Category = "Room Logic")
	void UnlockRoomDoors();

	// 전투가 끝났을 때 호출될 내부 함수
	void CompleteRoom();

	UFUNCTION(BlueprintCallable, Category = "Room Logic")
	void StartRoomCombat();

	UFUNCTION()
	void HandleMonsterReadyToSleep(class AR1Monster* DeadMonster);

public:
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Room State")
	TArray<class AR1Door*> RoomDoors;

public:
	UPROPERTY(EditDefaultsOnly, Category = "Room Setup")
	TSubclassOf<AActor> PortalClass;

};
