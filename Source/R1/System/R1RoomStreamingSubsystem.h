#pragma once

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

	// 현재 + 인접 3개 정책
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

	ER1RoomThermalState ThermalState = ER1RoomThermalState::Cold;
	TObjectPtr<ULevelStreamingDynamic> StreamingLevel = nullptr;
	double LastTouchedTime = 0.0;
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

	UFUNCTION(BlueprintCallable, Category = "Room Streaming")
	void SetRuntimeBudget(const FR1RuntimeBudget& InBudget);

	UFUNCTION(BlueprintPure, Category = "Room Streaming")
	FR1RuntimeBudget GetRuntimeBudget() const;

	UFUNCTION(BlueprintCallable, Category = "Room Streaming")
	void QueuePreloadRooms(const TArray<UR1RoomDefinitionData*>& CandidateRooms);

	UFUNCTION(BlueprintCallable, Category = "Room Streaming")
	void MarkRoomGameplayReady(UR1RoomDefinitionData* RoomDefinition);

	UFUNCTION(BlueprintPure, Category = "Room Streaming")
	ER1RoomThermalState GetRoomState(UR1RoomDefinitionData* RoomDefinition) const;

	UFUNCTION(BlueprintCallable, Category = "Room Streaming")
	bool CanOpenDoorImmediately(UR1RoomDefinitionData* RoomDefinition) const;

	UFUNCTION(BlueprintCallable, Category = "Room Streaming")
	void TickRoomCachePolicy();

	UPROPERTY(BlueprintAssignable, Category = "Room Streaming")
	FR1RoomStateChanged OnRoomBecameHot;

private:
	void BeginPreload(UR1RoomDefinitionData* RoomDefinition);
	void TrimPreloadIfNeeded();
	FName MakeRoomKey(const UR1RoomDefinitionData* RoomDefinition) const;

private:
	UPROPERTY(EditAnywhere, Category = "Room Streaming")
	FR1RuntimeBudget Budget;

	UPROPERTY()
	TMap<FName, FR1RoomRuntimeState> RoomStates;
};
