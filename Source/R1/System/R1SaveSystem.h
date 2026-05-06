

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "System/R1PlayerSaveGame.h"
#include "R1SaveSystem.generated.h"

/**
 * 
 */
UCLASS()
class R1_API UR1SaveSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
private:
	// 세이브 파일 이름
	const FString RunSaveSlotName = TEXT("CurrentRunSlot");
	const int32 RunSaveUserIndex = 0;
	// 영구 세이브 파일 이름 (언락한 캐릭터, 업적, 메타 진행도 등등)
	const FString MetaSaveSlotName = TEXT("MetaSaveSlot");
	const int32 MetaSaveUserIndex = 0;

	void ExtractAndSaveMetaProgression();
public:

	// 세이브 파일이 있는지 확인 (Continue 버튼 활성화 용도)
	UFUNCTION(BlueprintCallable, Category = "SaveSystem")
	bool HasSavedRun() const;

	// 세이브 파일 폭파 (플레이어 사망 또는 New Run 시작 시 호출)
	UFUNCTION(BlueprintCallable, Category = "SaveSystem")
	void DeleteSavedRun();

	// 현재 플레이어 스탯과 맵 제너레이터의 상태를 싹 긁어와서 저장
	UFUNCTION(BlueprintCallable, Category = "SaveSystem")
	void SaveCurrentRun(class AR1Player* Player, class AR1MapGenerator* MapGenerator);

	// 세이브 데이터를 열어서 플레이어에게 주입하고, 맵 데이터를 제너레이터에 전달
	UFUNCTION(BlueprintCallable, Category = "SaveSystem")
	bool LoadCurrentRun(class AR1Player* Player, class AR1MapGenerator* MapGenerator);

public:
	UFUNCTION(BlueprintCallable, Category = "SaveSystem|Meta")
	void SaveMetaProgression(class UR1MetaSaveGame* MetaSaveObj);

	UFUNCTION(BlueprintCallable, Category = "SaveSystem|Meta")
	class UR1MetaSaveGame* LoadMetaProgression();

	// 메모리 상에서 정산된 레벨을 추적하여 디스크 I/O 병목 및 중복 정산 방지
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SaveSystem")
	int32 CachedLastSettledLevel = 1;

	UPROPERTY()
	TMap<int32, FR1ShopInventorySaveData> ActiveShopInventories;

	// 상점 데이터 관리 함수
	void SaveShopInventory(int32 RoomID, const TArray<class UR1ItemInstance*>& ShopItems);
	bool LoadShopInventory(int32 RoomID, TArray<class UR1ItemInstance*>& OutShopItems, UObject* Outer);
};
