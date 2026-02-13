

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
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

private:
	static const FString RespawnSlotName;
	static constexpr int32 RespawnUserIndex = 0;
};
