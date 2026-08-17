

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "DataTable/SkillDataRow.h"
#include "R1GameInstance.generated.h"

class AR1Player;
/**
 * 
 */
UCLASS()
class R1_API UR1GameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UR1GameInstance(const FObjectInitializer& ObjectInitializer);

public:
	virtual void Init() override;
	virtual void Shutdown() override;

	//UFUNCTION(BlueprintCallable, Category = "SaveGame")
	//bool SavePlayerState(const AR1Player* Player, const FString& SlotName, int32 UserIndex = 0);

	//UFUNCTION(BlueprintCallable, Category = "SaveGame")
	//bool LoadPlayerStateToPlayer(AR1Player* Player, const FString& SlotName, int32 UserIndex = 0);

	//void SaveRespawnSnapshotFromPlayer(const AR1Player* Player);
	//void ApplyRespawnSnapshotToPlayer(AR1Player* Player);

public:
	const FSkillDataRow* GetSkillData(FName SkillName) const;

	// 보스 스킬 전용 조회. 플레이어 테이블과 행 이름이 겹칠 수 있어 테이블을 분리한다
	// (예: UR1GameplayAbility_Attack의 기본 SkillID "Attack"은 양쪽에 다 존재할 수 있음).
	// BossSkillDataTable이 비어 있으면 SkillDataTable로 폴백한다.
	const FSkillDataRow* GetBossSkillData(FName SkillName) const;

protected:
	// 🌟 블루프린트(BP_R1GameInstance)에서 등록할 딱 하나의 스킬 데이터 테이블
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	TObjectPtr<class UDataTable> SkillDataTable;

	// 보스 스킬 테이블 (DT_BossSkillData). BP_R1GameInstance에서 지정할 것.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	TObjectPtr<class UDataTable> BossSkillDataTable;

//private:
//	static const FString RespawnSlotName;
//	static constexpr int32 RespawnUserIndex = 0;
};
