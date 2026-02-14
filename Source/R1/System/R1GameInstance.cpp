#include "R1GameInstance.h"
#include "R1AssetManager.h"
#include "Character/R1Player.h"
#include "AbilitySystem/Attribute/R1AttributeSet.h"
#include "System/R1PlayerSaveGame.h"
#include "Kismet/GameplayStatics.h"

const FString UR1GameInstance::RespawnSlotName = TEXT("Slot1");

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

bool UR1GameInstance::SavePlayerState(const AR1Player* Player, const FString& SlotName, int32 UserIndex)
{
	if (Player == nullptr || SlotName.IsEmpty())
	{
		return false;
	}

	const UR1AttributeSet* Attributes = Cast<UR1AttributeSet>(Player->GetR1AttributeSet());
	if (Attributes == nullptr)
	{
		return false;
	}

	UR1PlayerSaveGame* SaveGameObject = Cast<UR1PlayerSaveGame>(UGameplayStatics::CreateSaveGameObject(UR1PlayerSaveGame::StaticClass()));
	if (SaveGameObject == nullptr)
	{
		return false;
	}

	SaveGameObject->MaxHealth = Attributes->GetMaxHealth();
	SaveGameObject->MaxMana = Attributes->GetMaxMana();
	SaveGameObject->BaseDamage = Attributes->GetBaseDamage();
	SaveGameObject->BaseDefence = Attributes->GetBaseDefence();
	SaveGameObject->Level = Attributes->GetLevel();
	SaveGameObject->Exp = Attributes->GetExp();
	SaveGameObject->MaxExp = Attributes->GetMaxExp();

	return UGameplayStatics::SaveGameToSlot(SaveGameObject, SlotName, UserIndex);
}

bool UR1GameInstance::LoadPlayerStateToPlayer(AR1Player* Player, const FString& SlotName, int32 UserIndex)
{
	//플레이어 상태를 저장된 세이브 게임에서 불러와서 플레이어에게 적용
	if (Player == nullptr || SlotName.IsEmpty())
	{
		return false;
	}

	USaveGame* RawSaveGame = UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex);
	UR1PlayerSaveGame* SaveGameObject = Cast<UR1PlayerSaveGame>(RawSaveGame);
	if (SaveGameObject == nullptr)
	{
		return false;
	}

	UR1AttributeSet* Attributes = Cast<UR1AttributeSet>(Player->GetR1AttributeSet());
	if (Attributes == nullptr)
	{
		return false;
	}

	Attributes->SetMaxHealth(SaveGameObject->MaxHealth);
	Attributes->SetMaxMana(SaveGameObject->MaxMana);
	Attributes->SetBaseDamage(SaveGameObject->BaseDamage);
	Attributes->SetBaseDefence(SaveGameObject->BaseDefence);
	Attributes->SetLevel(SaveGameObject->Level);
	Attributes->SetMaxExp(SaveGameObject->MaxExp);
	Attributes->SetExp(SaveGameObject->Exp);

	return true;
}

void UR1GameInstance::SaveRespawnSnapshotFromPlayer(const AR1Player* Player)
{
	if (SavePlayerState(Player, RespawnSlotName, RespawnUserIndex) == false)
	{
		return;
	}

	USaveGame* RawSaveGame = UGameplayStatics::LoadGameFromSlot(RespawnSlotName, RespawnUserIndex);
	UR1PlayerSaveGame* SaveGameObject = Cast<UR1PlayerSaveGame>(RawSaveGame);
	if (SaveGameObject)
	{
		SaveGameObject->bPendingRespawn = true;
		UGameplayStatics::SaveGameToSlot(SaveGameObject, RespawnSlotName, RespawnUserIndex);
	}
}

void UR1GameInstance::ApplyRespawnSnapshotToPlayer(AR1Player* Player)
{
	//죽기 전에 저장된 데이터를 플레이어에게 적용
	USaveGame* RawSaveGame = UGameplayStatics::LoadGameFromSlot(RespawnSlotName, RespawnUserIndex);
	UR1PlayerSaveGame* SaveGameObject = Cast<UR1PlayerSaveGame>(RawSaveGame);
	if (SaveGameObject == nullptr || SaveGameObject->bPendingRespawn == false)
	{
		return;
	}

	if (LoadPlayerStateToPlayer(Player, RespawnSlotName, RespawnUserIndex) == false)
	{
		return;
	}

	SaveGameObject->bPendingRespawn = false;
	UGameplayStatics::SaveGameToSlot(SaveGameObject, RespawnSlotName, RespawnUserIndex);
}

