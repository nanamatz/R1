#include "R1GameInstance.h"
#include "R1AssetManager.h"
#include "Character/R1Player.h"
#include "AbilitySystem/Attribute/PlayerAttributeSet.h"
#include "AbilitySystem/Attribute/R1AttributeSet.h"
#include "System/R1PlayerSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Player/R1PlayerState.h"

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

	AR1PlayerState* PS = Cast<AR1PlayerState>(Player->GetPlayerState());
	UR1AttributeSet* CommonAttributes = Cast<UR1AttributeSet>(PS->GetCommonAttributeSet());
	UPlayerAttributeSet* PlayerAttributes = Cast<UPlayerAttributeSet>(PS->GetPlayerAttributeSet());
	if (CommonAttributes == nullptr || PlayerAttributes == nullptr)
	{
		return false;
	}

	UR1PlayerSaveGame* SaveGameObject = Cast<UR1PlayerSaveGame>(UGameplayStatics::CreateSaveGameObject(UR1PlayerSaveGame::StaticClass()));
	if (SaveGameObject == nullptr)
	{
		return false;
	}

	SaveGameObject->MaxHealth = CommonAttributes->GetMaxHealth();
	SaveGameObject->BaseDamage = CommonAttributes->GetBaseDamage();
	SaveGameObject->BaseDefence = CommonAttributes->GetBaseDefence();

	SaveGameObject->MaxMana = PlayerAttributes->GetMaxMana();
	SaveGameObject->Level = PlayerAttributes->GetLevel();
	SaveGameObject->Exp = PlayerAttributes->GetExp();
	SaveGameObject->MaxExp = PlayerAttributes->GetMaxExp();

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


	AR1PlayerState* PS = Cast<AR1PlayerState>(Player->GetPlayerState());
	UR1AttributeSet* CommonAttributes = Cast<UR1AttributeSet>(PS->GetCommonAttributeSet());
	UPlayerAttributeSet* PlayerAttributes = Cast<UPlayerAttributeSet>(PS->GetPlayerAttributeSet());

	if (CommonAttributes == nullptr || PlayerAttributes == nullptr)
	{
		return false;
	}

	CommonAttributes->SetMaxHealth(SaveGameObject->MaxHealth);
	CommonAttributes->SetBaseDamage(SaveGameObject->BaseDamage);
	CommonAttributes->SetBaseDefence(SaveGameObject->BaseDefence);

	PlayerAttributes->SetLevel(SaveGameObject->Level);
	PlayerAttributes->SetMaxExp(SaveGameObject->MaxExp);
	PlayerAttributes->SetExp(SaveGameObject->Exp);
	PlayerAttributes->SetMaxMana(SaveGameObject->MaxMana);

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

