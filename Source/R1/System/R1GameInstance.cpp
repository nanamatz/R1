#include "R1GameInstance.h"
#include "R1LogChannels.h"
#include "R1AssetManager.h"
#include "Character/R1Player.h"
#include "AbilitySystem/Attribute/PlayerAttributeSet.h"
#include "AbilitySystem/Attribute/R1AttributeSet.h"
#include "System/R1PlayerSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Player/R1PlayerState.h"

//const FString UR1GameInstance::RespawnSlotName = TEXT("Slot1");

UR1GameInstance::UR1GameInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UR1GameInstance::Init()
{
	Super::Init();

	UR1AssetManager::Initialize();
}

void UR1GameInstance::Shutdown()
{
	Super::Shutdown();

}

const FSkillDataRow* UR1GameInstance::GetSkillData(FName SkillName) const
{
	// 에디터에서 데이터 테이블을 안 넣었을 경우를 대비한 안전장치
	if (SkillDataTable == nullptr)
	{
		UE_LOG(LogR1, Error, TEXT("UR1GameInstance: 스킬 데이터 테이블이 비어있습니다!"));
		return nullptr;
	}

	// 🌟 매개변수로 받은 SkillName(행 이름)을 검색해서 반환합니다.
	return SkillDataTable->FindRow<FSkillDataRow>(SkillName, TEXT(""));
}

