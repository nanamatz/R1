#include "Character/R1Boss.h"
#include "UI/R1HUD.h"
#include "Data/R1ItemPoolData.h"
#include "Object/R1ItemActor.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystem/R1AbilitySystemComponent.h"

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
}

void AR1Boss::AddCharacterAbility()
{
    Super::AddCharacterAbility();

    UR1AbilitySystemComponent* ASC = Cast<UR1AbilitySystemComponent>(AbilitySystemComponent);
    if (ASC == nullptr)
    {
        return;
    }

    ASC->AddCharacterAbilities(DefaultSkillAbilities);
    ASC->AddCharacterAbilities(AdditionalSkillAbilities);

}
