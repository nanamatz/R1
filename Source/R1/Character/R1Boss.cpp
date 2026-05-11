#include "Character/R1Boss.h"
#include "UI/R1HUD.h"
#include "Data/R1ItemPoolData.h"
#include "Object/R1ItemActor.h"
#include "Kismet/GameplayStatics.h"

AR1Boss::AR1Boss()
{
    Tags.Add(FName("Boss"));
}

void AR1Boss::BeginPlay()
{
    Super::BeginPlay();

    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        if (AR1HUD* HUD = Cast<AR1HUD>(PC->GetHUD()))
        {
            HUD->ShowBossInfo(this);
        }
    }
}

void AR1Boss::OnDead(const TObjectPtr<AR1Character> Attacker)
{
    Super::OnDead(Attacker);

    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        if (AR1HUD* HUD = Cast<AR1HUD>(PC->GetHUD()))
        {
            // 보스가 죽으면 HUD에서 보스 정보를 지웁니다.
            HUD->HideBossInfo();
        }
    }

    // Loot Drop Logic
    if (BossLootPool && ItemActorClass && FMath::RandRange(0.0f, 1.0f) < BossDropChance)
    {
        UR1ItemAssetData* RandomItem = UR1ItemPoolData::GetRandomItemFromPool(BossLootPool);
        if (RandomItem)
        {
            FActorSpawnParameters SpawnParams;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

            AR1ItemActor* SpawnedActor = GetWorld()->SpawnActor<AR1ItemActor>(ItemActorClass, GetActorLocation(), FRotator::ZeroRotator, SpawnParams);
            if (SpawnedActor)
            {
                // 아이템 데이터에 정의된 희귀도를 사용합니다.
                SpawnedActor->InitItem(RandomItem, RandomItem->ItemRarity, 1);
                SpawnedActor->PopEffect();
            }
        }
    }
}
