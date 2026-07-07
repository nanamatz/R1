/**
 * UR1RoomStreamingSubsystem — 슬림 버전.
 * 층(Floor) 단위 전체 로딩 모델로 전환하면서, 룸 레벨 인스턴스의 스폰/언로드만
 * 담당합니다. 기존 열적(thermal) 상태 머신/예산(budget)/선로딩 로직은 폐기되었고,
 * 블루프린트 호환을 위해 시그니처만 deprecated no-op으로 남겨 둡니다.
 */

#include "System/R1RoomStreamingSubsystem.h"
#include "R1LogChannels.h"
#include "Data/R1RoomDefinitionData.h"
#include "Engine/LevelStreamingDynamic.h"

void UR1RoomStreamingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	RoomStates.Reset();
}

void UR1RoomStreamingSubsystem::Deinitialize()
{
	RoomStates.Reset();
	Super::Deinitialize();
}

ULevelStreamingDynamic* UR1RoomStreamingSubsystem::SpawnRoomLevel(UR1RoomDefinitionData* RoomDefinition, FVector Location, FRotator Rotation)
{
	if (!RoomDefinition || RoomDefinition->RoomLevel.IsNull()) return nullptr;

	const FName RoomKey = MakeRoomKey(RoomDefinition);
	FR1RoomRuntimeState& State = RoomStates.FindOrAdd(RoomKey);
	State.RoomDefinition = RoomDefinition;

	if (State.StreamingLevel) return State.StreamingLevel;

	bool bOutSuccess = false;
	State.StreamingLevel = ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(
		this, RoomDefinition->RoomLevel, Location, Rotation, bOutSuccess
	);

	return State.StreamingLevel;
}

void UR1RoomStreamingSubsystem::UnloadAllRooms()
{
	UWorld* World = GetWorld();
	if (!World) return;

	const TArray<ULevelStreaming*>& StreamingLevels = World->GetStreamingLevels();
	for (ULevelStreaming* Level : StreamingLevels)
	{
		if (ULevelStreamingDynamic* DynamicLevel = Cast<ULevelStreamingDynamic>(Level))
		{
			DynamicLevel->SetShouldBeLoaded(false);
			DynamicLevel->SetShouldBeVisible(false);
			DynamicLevel->SetIsRequestingUnloadAndRemoval(true);
		}
	}

	// [중요] 다음 층에서 SpawnRoomLevel이 '제거 중'인 옛 인스턴스를 재사용(dedup)하지
	// 않도록 룸 상태 캐시를 비웁니다.
	RoomStates.Reset();

	UE_LOG(LogR1, Warning, TEXT("[RoomStreaming] 이전 층의 모든 방을 메모리에서 해제했습니다."));
}

FName UR1RoomStreamingSubsystem::MakeRoomKey(const UR1RoomDefinitionData* RoomDefinition) const
{
	if (RoomDefinition == nullptr)
	{
		return NAME_None;
	}
	return FName(*RoomDefinition->GetPrimaryAssetId().ToString());
}

// ---- 이하 deprecated no-op 스텁 (블루프린트 호환 목적; 실제 동작 없음) ----

PRAGMA_DISABLE_DEPRECATION_WARNINGS

void UR1RoomStreamingSubsystem::SetRuntimeBudget(const FR1RuntimeBudget& InBudget) {}

FR1RuntimeBudget UR1RoomStreamingSubsystem::GetRuntimeBudget() const { return Budget; }

void UR1RoomStreamingSubsystem::QueuePreloadRooms(const TArray<UR1RoomDefinitionData*>& CandidateRooms) {}

void UR1RoomStreamingSubsystem::MarkRoomGameplayReady(UR1RoomDefinitionData* RoomDefinition) {}

ER1RoomThermalState UR1RoomStreamingSubsystem::GetRoomState(UR1RoomDefinitionData* RoomDefinition) const
{
	return ER1RoomThermalState::Hot;
}

void UR1RoomStreamingSubsystem::TickRoomCachePolicy() {}

void UR1RoomStreamingSubsystem::MarkRoomAsLeft(UR1RoomDefinitionData* RoomDefinition) {}

PRAGMA_ENABLE_DEPRECATION_WARNINGS
