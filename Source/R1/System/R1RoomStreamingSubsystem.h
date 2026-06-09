#pragma once

/**
 * [파일 역할]
 * 룸(Room)의 스트리밍 및 선로딩(Preload) 상태를 총괄 관리하는 게임 인스턴스 서브시스템입니다.
 * 룸의 상태를 4단계(Cold, Preloading, Warm, Hot)로 관리하며, 플레이어의 이동에 맞춰 
 * 필요한 자원을 동적으로 로드 및 언로드합니다.
 * 
 * [필요성]
 * 1. 프레임 드랍 방지: 플레이어가 다음 방에 진입하기 전, 배경에서 자원을 미리 로드하여 
 *    게임 플레이 중 발생하는 끊김 현상(Hitch)을 최소화합니다.
 * 2. 리소스 최적화: 정해진 예산(Budget) 내에서만 데이터를 유지하고, 일정 시간 사용되지 않는 
 *    방의 데이터를 자동으로 정리하여 메모리 부족 현상을 방지합니다.
 * 3. 룸 전환 전략: 단순 로딩을 넘어, '현재 위치 + 인접 룸' 정책을 통해 최적의 게임 경험을 제공합니다.
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

	UPROPERTY(BlueprintAssignable, Category = "Room Streaming")
	FR1RoomStateChanged OnRoomBecameHot;

private:
	UPROPERTY(EditAnywhere, Category = "Room Streaming")
	FR1RuntimeBudget Budget;

	UPROPERTY()
	TMap<FName, FR1RoomRuntimeState> RoomStates;

private:
	// [추가] 진짜로 메모리에서 맵과 에셋을 내리는 함수
	void UnloadRoomInternal(FR1RoomRuntimeState& State);

private:
	FName MakeRoomKey(const UR1RoomDefinitionData* RoomDefinition) const;
};
