

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
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
};
