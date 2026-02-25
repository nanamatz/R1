


#include "Item/R1ItemInstance.h"
#include "Engine/DataTable.h"
#include "UObject/UObjectGlobals.h"

UR1ItemInstance::UR1ItemInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

//void UR1ItemInstance::Init(int32 InItemID, FIntPoint InItemSize, ER1EquipmentSlot InEquipSlot)
//{
//	if (InItemID <= 0)
//	{
//		return;
//	}
//
//	ItemID = InItemID;
//	ItemRarity = EItemRarity::Common;
//
//	// 초기화 시 전달받은 크기와 슬롯 정보를 저장합니다.
//	ItemSize = InItemSize;
//	EquipSlot = InEquipSlot;
//}

void UR1ItemInstance::Init(int32 InItemID, UDataTable* InDataTable)
{
	// 1. 내 ID 세팅
	ItemID = InItemID;

	if (InDataTable)
	{
		FName RowName = FName(*FString::FromInt(InItemID));
		FString ContextString = TEXT("Item Init");
		FR1ItemDataRow* FoundData = InDataTable->FindRow<FR1ItemDataRow>(RowName, ContextString);

		if (FoundData)
		{
			ItemData = *FoundData;
		}
	}

	//// 2. 데이터 테이블 에셋 로드
	//FString DataTablePath = TEXT("/Script/Engine.DataTable'/Game/DataTable/DT_ItemDataTable.DT_ItemDataTable'");
	//UDataTable* ItemDataTable = LoadObject<UDataTable>(nullptr, *DataTablePath);

	//if (ItemDataTable)
	//{
	//	// 3. int32 숫자 ID를 데이터 테이블의 행 이름(FName)으로 변환
	//	FName RowName = FName(*FString::FromInt(InItemID));
	//	FString ContextString = TEXT("ItemInstance Init Context"); // 에러 로그 출력용 텍스트

	//	// 4. 데이터 테이블에서 해당 행(Row) 검색
	//	FR1ItemDataRow* FoundData = ItemDataTable->FindRow<FR1ItemDataRow>(RowName, ContextString);

	//	if (FoundData)
	//	{
	//		// 5. 검색 성공! 원본 데이터를 내 멤버 변수(ItemData)로 깊은 복사(값 복사)
	//		ItemData = *FoundData;

	//		UE_LOG(LogTemp, Log, TEXT("[%s] 아이템 데이터 로드 성공! (크기: %dx%d)"),
	//			*ItemData.ItemName.ToString(), ItemData.ItemSize.X, ItemData.ItemSize.Y);
	//	}
	//	else
	//	{
	//		// 데이터 테이블에 해당 번호의 행이 없을 경우
	//		UE_LOG(LogTemp, Warning, TEXT("ID %d번 아이템을 데이터 테이블에서 찾을 수 없습니다!"), InItemID);
	//	}
	//}
	//else
	//{
	//	// 경로가 틀렸거나 파일이 없을 경우
	//	UE_LOG(LogTemp, Error, TEXT("데이터 테이블을 로드할 수 없습니다! 경로를 확인해주세요: %s"), *DataTablePath);
	//}
}