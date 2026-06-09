#pragma once

/**
 * [파일 역할]
 * 룸(Room) 레벨 인스턴스의 스폰/언로드를 담당하는 게임 인스턴스 서브시스템입니다.
 * 층(Floor) 단위 전체 로딩 모델로 전환되어, 기존의 4단계 열적(thermal) 상태 머신과
 * 인접 룸 선로딩(Preload)/예산(Budget) 정책은 폐기되었습니다.
 * 블루프린트 호환을 위해 일부 함수는 deprecated no-op 스텁으로만 남아 있습니다.
 */

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "R1RoomStreamingSubsystem.generated.h"

class ULevelStreamingDynamic;
class UR1RoomDefinitionData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FR1RoomStateChanged, FName, RoomKey);

USTRUCT(BlueprintType)
struct FR1RuntimeBudget
{
	GENERATED_BODY()

	// 현재 + 인접 4개 정책
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget")
	int32 MaxPreloadedRooms = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget")
	int32 MaxAliveMonstersPerRoom = 32;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget")
	int32 MaxProjectilesPerRoom = 48;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget")
	int32 MaxNiagaraSystemsPerRoom = 24;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget")
	int32 MaxConcurrentSfxVoices = 20;

	// 플레이어가 뒤돌아올 수 있는 시간 창
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget")
	float UnloadGraceSeconds = 8.0f;
};

UENUM(BlueprintType)
enum class ER1RoomThermalState : uint8
{
	Cold,
	Preloading,
	Warm,
	Hot,
};

USTRUCT()
struct FR1RoomRuntimeState
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UR1RoomDefinitionData> RoomDefinition = nullptr;

	UPROPERTY()
	TObjectPtr<ULevelStreamingDynamic> StreamingLevel = nullptr;
};

/**
 * 하이브리드 전환 전략(큰 전환 + 룸 Async 선로딩)의 뼈대 시스템.
 */
UCLASS()
class R1_API UR1RoomStreamingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// [추가] Warm 상태인 방을 물리적으로 스폰합니다.
	UFUNCTION(BlueprintCallable, Category = "Room Streaming")
	ULevelStreamingDynamic* SpawnRoomLevel(UR1RoomDefinitionData* RoomDefinition, FVector Location, FRotator Rotation);

	UE_DEPRECATED(5.3, "Thermal streaming retired; floor loads whole at once. No-op.")
	UFUNCTION(BlueprintCallable, Category = "Room Streaming", meta = (DeprecatedFunction))
	void SetRuntimeBudget(const FR1RuntimeBudget& InBudget);

	UE_DEPRECATED(5.3, "Thermal streaming retired. Returns default budget.")
	UFUNCTION(BlueprintPure, Category = "Room Streaming", meta = (DeprecatedFunction))
	FR1RuntimeBudget GetRuntimeBudget() const;

	UE_DEPRECATED(5.3, "Whole floor is preloaded; preload-on-demand retired. No-op.")
	UFUNCTION(BlueprintCallable, Category = "Room Streaming", meta = (DeprecatedFunction))
	void QueuePreloadRooms(const TArray<UR1RoomDefinitionData*>& CandidateRooms);

	UE_DEPRECATED(5.3, "Activation moved to AR1MapGenerator::ActivateRoom. No-op.")
	UFUNCTION(BlueprintCallable, Category = "Room Streaming", meta = (DeprecatedFunction))
	void MarkRoomGameplayReady(UR1RoomDefinitionData* RoomDefinition);

	UE_DEPRECATED(5.3, "Thermal state retired. Always returns Hot.")
	UFUNCTION(BlueprintPure, Category = "Room Streaming", meta = (DeprecatedFunction))
	ER1RoomThermalState GetRoomState(UR1RoomDefinitionData* RoomDefinition) const;

	UE_DEPRECATED(5.3, "Cache policy retired. No-op.")
	UFUNCTION(BlueprintCallable, Category = "Room Streaming", meta = (DeprecatedFunction))
	void TickRoomCachePolicy();

	UE_DEPRECATED(5.3, "Rooms stay resident for the whole floor. No-op.")
	UFUNCTION(BlueprintCallable, Category = "Room Streaming", meta = (DeprecatedFunction))
	void MarkRoomAsLeft(UR1RoomDefinitionData* RoomDefinition);

	UFUNCTION(BlueprintCallable, Category = "Room Streaming")
	void UnloadAllRooms();

	// [폐기] 더 이상 Broadcast되지 않습니다. 기존 블루프린트 바인딩 호환을 위해서만 남겨 둡니다.
	UPROPERTY(BlueprintAssignable, Category = "Room Streaming")
	FR1RoomStateChanged OnRoomBecameHot;

private:
	// GetRuntimeBudget() deprecated 스텁의 반환값으로만 유지됩니다. (실제 동작 없음)
	UPROPERTY(EditAnywhere, Category = "Room Streaming")
	FR1RuntimeBudget Budget;

	UPROPERTY()
	TMap<FName, FR1RoomRuntimeState> RoomStates;

private:
private:
	FName MakeRoomKey(const UR1RoomDefinitionData* RoomDefinition) const;
};
