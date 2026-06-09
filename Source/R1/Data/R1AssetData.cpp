


#include "Data/R1AssetData.h"
#include "UObject/ObjectSaveContext.h"

void UR1AssetData::PreSave(FObjectPreSaveContext ObjectSaveContext)
{
	Super::PreSave(ObjectSaveContext);

	AssetNameToPath.Empty();
	AssetLabelToSet.Empty();

	AssetGroupNameToSet.KeySort([](const FName& A, const FName& B)
		{
			return (A.Compare(B) < 0);
		});

	for (const auto& Pair : AssetGroupNameToSet)
	{
		const FAssetSet& AssetSet = Pair.Value;
		for (FAssetEntry AssetEntry : AssetSet.AssetEntries)
		{
			FSoftObjectPath& AssetPath = AssetEntry.AssetPath;
			/*const FString& AssetName = AssetPath.GetAssetName();
			if (AssetName.StartsWith(TEXT("BP_")) || AssetName.StartsWith(TEXT("B_")) || AssetName.StartsWith(TEXT("WBP_")) ||
				AssetName.StartsWith(TEXT("GE_")) || AssetName.StartsWith(TEXT("GA_")) || AssetName.StartsWith(TEXT("ABP_")))
			{
				FString AssetPathString = AssetPath.GetAssetPathString();
				AssetPathString.Append(TEXT("_C"));
				AssetPath = FSoftObjectPath(AssetPathString);
			}*/

			AssetNameToPath.Emplace(AssetEntry.AssetName, AssetEntry.AssetPath);
			for (const FName& Label : AssetEntry.AssetLabels)
			{
				AssetLabelToSet.FindOrAdd(Label).AssetEntries.Emplace(AssetEntry);
			}
		}
	}
}

FSoftObjectPath UR1AssetData::GetAssetPathByName(const FName& AssetName)
{
	FSoftObjectPath* AssetPath = AssetNameToPath.Find(AssetName);
	ensureAlwaysMsgf(AssetPath, TEXT("Can't find Asset Path from Asset Name [%s]."), *AssetName.ToString());
	return *AssetPath;
}

const FAssetSet& UR1AssetData::GetAssetSetByLabel(const FName& Label)
{
	const FAssetSet* AssetSet = AssetLabelToSet.Find(Label);
	if (AssetSet == nullptr)
	{
		// 라벨이 없으면 크래시(널 역참조) 대신 빈 세트를 반환한다.
		// ensure로 개발 중에는 알리되, 런타임은 비어 있는 세트로 안전하게 진행.
		ensureAlwaysMsgf(false, TEXT("Can't find Asset Set from Label [%s]."), *Label.ToString());
		static const FAssetSet EmptyAssetSet;
		return EmptyAssetSet;
	}
	return *AssetSet;
}
