


#include "R1SaveSystem.h"
#include "Kismet/GameplayStatics.h"
#include "R1PlayerSaveGame.h" // 우리가 수정한 세이브 객체 헤더

#include "Character/R1Player.h"

#include "Player/R1PlayerState.h"

#include "AbilitySystem/Attribute/PlayerAttributeSet.h"
#include "AbilitySystem/Attribute/R1AttributeSet.h"

#include "Map/R1MapGenerator.h"
#include "Data/R1RoomDefinitionData.h"

bool UR1SaveSystem::HasSavedRun() const
{
	return UGameplayStatics::DoesSaveGameExist(RunSaveSlotName, RunSaveUserIndex);
}

void UR1SaveSystem::DeleteSavedRun()
{
	if (HasSavedRun())
	{
		UGameplayStatics::DeleteGameInSlot(RunSaveSlotName, RunSaveUserIndex);
		UE_LOG(LogTemp, Warning, TEXT("[SaveSystem] 💥 세이브 파일 삭제 완료! (사망 처리)"));
	}
}

void UR1SaveSystem::SaveCurrentRun(AR1Player* Player, AR1MapGenerator* MapGenerator)
{
	UR1PlayerSaveGame* SaveObj = Cast<UR1PlayerSaveGame>(UGameplayStatics::CreateSaveGameObject(UR1PlayerSaveGame::StaticClass()));
	if (!SaveObj) return;

	// 1. 플레이어 상태 저장 (작성하셨던 코드 재활용!)
	if (Player)
	{
		if (AR1PlayerState* PS = Cast<AR1PlayerState>(Player->GetPlayerState()))
		{
			if (UR1AttributeSet* CommonAttr = Cast<UR1AttributeSet>(PS->GetCommonAttributeSet()))
			{
				SaveObj->MaxHealth = CommonAttr->GetMaxHealth();
				SaveObj->Health = CommonAttr->GetHealth();

				SaveObj->BaseDamage = CommonAttr->GetBaseDamage();
				SaveObj->BaseDefence = CommonAttr->GetBaseDefence();
				SaveObj->MoveSpeed = CommonAttr->GetMoveSpeed();
				SaveObj->AttackSpeed = CommonAttr->GetAttackSpeed();
			}
			if (UPlayerAttributeSet* PlayerAttr = Cast<UPlayerAttributeSet>(PS->GetPlayerAttributeSet()))
			{
				SaveObj->MaxMana = PlayerAttr->GetMaxMana();
				SaveObj->Mana = PlayerAttr->GetMana();
				SaveObj->Level = PlayerAttr->GetLevel();
				SaveObj->Exp = PlayerAttr->GetExp();
				SaveObj->MaxExp = PlayerAttr->GetMaxExp();
			}
		}
	}

	// 2. 맵 데이터 압축 및 저장
	if (MapGenerator)
	{
		SaveObj->CurrentFloorIndex = MapGenerator->CurrentFloorIndex;
		SaveObj->CurrentActiveNodeID = MapGenerator->GetCurrentNodeID();

		// MapGenerator가 들고 있는 무거운 GeneratedMap을 가벼운 SaveNode로 변환
		for (const FR1MapNode& Node : MapGenerator->GeneratedMap)
		{
			FR1MapNodeSaveData SaveNode;
			SaveNode.NodeID = Node.NodeID;
			SaveNode.bIsCleared = Node.bIsCleared;
			SaveNode.GridPosition = Node.GridPosition;
			SaveNode.ConnectedNodeIDs = Node.ConnectedNodeIDs;

			if (Node.RoomDefinition)
			{
				SaveNode.RoomLabel = Node.RoomDefinition->GetFName();
			}
			else
			{
				SaveNode.RoomLabel = NAME_None;
			}

			SaveObj->SavedMapNodes.Add(SaveNode);
		}
	}

	UGameplayStatics::SaveGameToSlot(SaveObj, RunSaveSlotName, RunSaveUserIndex);
	UE_LOG(LogTemp, Log, TEXT("[SaveSystem] 💾 현재 런(Run) 진행도 저장 완료!"));
}

bool UR1SaveSystem::LoadCurrentRun(AR1Player* Player, AR1MapGenerator* MapGenerator)
{
	if (!HasSavedRun()) return false;

	UR1PlayerSaveGame* SaveObj = Cast<UR1PlayerSaveGame>(UGameplayStatics::LoadGameFromSlot(RunSaveSlotName, RunSaveUserIndex));
	if (!SaveObj) return false;

	// 1. 플레이어 상태 주입
	if (Player)
	{
		if (AR1PlayerState* PS = Cast<AR1PlayerState>(Player->GetPlayerState()))
		{
			if (UR1AttributeSet* CommonAttr = Cast<UR1AttributeSet>(PS->GetCommonAttributeSet()))
			{
				CommonAttr->SetMaxHealth(SaveObj->MaxHealth);
				CommonAttr->SetHealth(SaveObj->Health);
				CommonAttr->SetBaseDamage(SaveObj->BaseDamage);
				CommonAttr->SetBaseDefence(SaveObj->BaseDefence);
				CommonAttr->SetMoveSpeed(SaveObj->MoveSpeed);
				CommonAttr->SetAttackSpeed(SaveObj->AttackSpeed);
			}
			if (UPlayerAttributeSet* PlayerAttr = Cast<UPlayerAttributeSet>(PS->GetPlayerAttributeSet()))
			{
				PlayerAttr->SetLevel(SaveObj->Level);
				PlayerAttr->SetMaxExp(SaveObj->MaxExp);
				PlayerAttr->SetExp(SaveObj->Exp);
				PlayerAttr->SetMaxMana(SaveObj->MaxMana);
				PlayerAttr->SetMana(SaveObj->Mana);
			}
		}
	}

	if (MapGenerator)
	{
		MapGenerator->LoadMapFromSaveData(SaveObj->SavedMapNodes, SaveObj->CurrentFloorIndex, SaveObj->CurrentActiveNodeID);
	}

	UE_LOG(LogTemp, Log, TEXT("[SaveSystem] 📂 저장된 런(Run) 진행도 복구 완료!"));
	return true;
}
