


#include "Item/R1ItemInstance.h"
#include "Engine/DataTable.h"
#include "UObject/UObjectGlobals.h"

UR1ItemInstance::UR1ItemInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UR1ItemInstance::Init(int32 InItemID, UDataTable* InDataTable)
{
	// 1. 내 ID 세팅
	ItemID = InItemID;

	if (InDataTable)
	{
		FName RowName = FName(*FString::FromInt(InItemID));
		FString ContextString = TEXT("Item Init");
		FR1ItemDataRow* FoundData = InDataTable->FindRow<FR1ItemDataRow>(RowName, ContextString);
		CachedItemData = FoundData;

		if (!CachedItemData)
		{
			UE_LOG(LogTemp, Error, TEXT("아이템 ID %d를 데이터 테이블에서 찾을 수 없습니다!"), ItemID);
		}
	}

}