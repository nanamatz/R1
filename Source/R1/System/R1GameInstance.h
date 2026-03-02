

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

	UFUNCTION(BlueprintCallable, Category = "SaveGame")
	bool SavePlayerState(const AR1Player* Player, const FString& SlotName, int32 UserIndex = 0);

	UFUNCTION(BlueprintCallable, Category = "SaveGame")
	bool LoadPlayerStateToPlayer(AR1Player* Player, const FString& SlotName, int32 UserIndex = 0);

	void SaveRespawnSnapshotFromPlayer(const AR1Player* Player);
	void ApplyRespawnSnapshotToPlayer(AR1Player* Player);

public:
	// 🌟 어빌리티들이 스킬 데이터를 물어볼 때 사용할 전역(Global) 함수!
	const FSkillDataRow* GetSkillData(FName SkillName) const;

protected:
	// 🌟 블루프린트(BP_R1GameInstance)에서 등록할 딱 하나의 스킬 데이터 테이블
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	TObjectPtr<class UDataTable> SkillDataTable;

private:
	static const FString RespawnSlotName;
	static constexpr int32 RespawnUserIndex = 0;
};
