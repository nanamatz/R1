#include "Character/R1Boss.h"
#include "UI/R1HUD.h"
#include "Data/R1ItemPoolData.h"
#include "Object/R1ItemActor.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystem/R1AbilitySystemComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "R1LogChannels.h"

AR1Boss::AR1Boss()
{
    Tags.Add(FName("Boss"));
}

void AR1Boss::BeginPlay()
{
    Super::BeginPlay();

    ActiveDefaultSkills = DefaultSkillAbilities;
    ActiveAdditionalSkills = AdditionalSkillAbilities;

    // 페이즈는 내림차순이어야 AdvancePhasesForRatio가 올바르게 동작한다.
    for (int32 i = 1; i < Phases.Num(); ++i)
    {
        ensureMsgf(Phases[i].HealthRatioThreshold <= Phases[i - 1].HealthRatioThreshold,
            TEXT("%s: Phases must be authored in descending HealthRatioThreshold order (index %d)"), *GetName(), i);
    }

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

    GrantedAbilityClasses.Append(DefaultSkillAbilities);
    GrantedAbilityClasses.Append(AdditionalSkillAbilities);
}

void AR1Boss::OnHealthChanged(float Ratio, bool bIsDamage)
{
    Super::OnHealthChanged(Ratio, bIsDamage);

    if (bIsDamage)
    {
        AdvancePhasesForRatio(Ratio);
    }
}

void AR1Boss::AdvancePhasesForRatio(float Ratio)
{
    // 한 방에 두 임계값을 넘길 수 있으므로 while로 순차 진입한다.
    while (CurrentPhaseIndex + 1 < Phases.Num() && Ratio <= Phases[CurrentPhaseIndex + 1].HealthRatioThreshold)
    {
        EnterPhase(CurrentPhaseIndex + 1);
    }
}

void AR1Boss::EnterPhase(int32 PhaseIndex)
{
    if (!Phases.IsValidIndex(PhaseIndex))
    {
        return;
    }

    CurrentPhaseIndex = PhaseIndex;
    const FBossPhase& Phase = Phases[PhaseIndex];

    UR1AbilitySystemComponent* ASC = Cast<UR1AbilitySystemComponent>(AbilitySystemComponent);

    // 1. 아직 부여하지 않은 어빌리티만 ASC에 추가
    if (ASC)
    {
        TArray<TSubclassOf<UGameplayAbility>> ToGrant;
        for (const TSubclassOf<UGameplayAbility>& AbilityClass : Phase.DefaultSkills)
        {
            if (AbilityClass && !GrantedAbilityClasses.Contains(AbilityClass))
            {
                ToGrant.Add(AbilityClass);
            }
        }
        for (const TSubclassOf<UGameplayAbility>& AbilityClass : Phase.AdditionalSkills)
        {
            if (AbilityClass && !GrantedAbilityClasses.Contains(AbilityClass))
            {
                ToGrant.Add(AbilityClass);
            }
        }

        if (ToGrant.Num() > 0)
        {
            ASC->AddCharacterAbilities(ToGrant);
            GrantedAbilityClasses.Append(ToGrant);
        }
    }

    // 2. 스킬 목록 교체 (비어 있으면 유지)
    if (Phase.DefaultSkills.Num() > 0)
    {
        ActiveDefaultSkills = Phase.DefaultSkills;
    }
    if (Phase.AdditionalSkills.Num() > 0)
    {
        ActiveAdditionalSkills = Phase.AdditionalSkills;
    }

    // 3. 격노 GE 적용 (이전 페이즈 GE는 제거하지 않고 누적)
    if (Phase.EnrageEffect && ASC)
    {
        FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
        Context.AddSourceObject(this);
        FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(Phase.EnrageEffect, 1.0f, Context);
        if (SpecHandle.IsValid())
        {
            ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
        }
    }

    // 4. 전환 연출
    if (Phase.TransitionMontage)
    {
        PlayAnimMontage(Phase.TransitionMontage);
    }

    // 5. 블랙보드에 페이즈 노출 (BT에서 분기하고 싶을 때 쓸 수 있게)
    if (AAIController* AIC = Cast<AAIController>(GetController()))
    {
        if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
        {
            BB->SetValueAsInt(TEXT("Phase"), PhaseIndex);
        }
    }

    UE_LOG(LogR1, Log, TEXT("%s entered boss phase %d (threshold %.2f)"), *GetName(), PhaseIndex, Phase.HealthRatioThreshold);
}
