#include "Character/R1Boss.h"
#include "UI/R1HUD.h"
#include "Data/R1ItemPoolData.h"
#include "Object/R1ItemActor.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystem/R1AbilitySystemComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "R1LogChannels.h"
#include "AbilitySystem/Attribute/R1AttributeSet.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"

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
    // 전환 연출 도중에 죽는 경우를 대비해 하이퍼아머/이동 제한을 반드시 푼다.
    if (bIsInPhaseTransition)
    {
        EndPhaseTransition();
    }

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

    // 4. 전환 연출 (진행 중인 어빌리티를 끊고 하이퍼아머 상태로 재생)
    BeginPhaseTransition(Phase.TransitionMontage);

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

float AR1Boss::GetHealthFloor() const
{
    if (CoreAttributeSet == nullptr)
    {
        return 0.0f;
    }

    const float MaxHp = CoreAttributeSet->GetMaxHealth();
    if (MaxHp <= 0.0f)
    {
        return 0.0f;
    }

    // 전환 연출 중에는 현재 페이즈 임계값에 체력을 묶어둔다 — 연출이 끝나기 전에는 더 깎이지 않는다.
    if (bIsInPhaseTransition && Phases.IsValidIndex(CurrentPhaseIndex))
    {
        return Phases[CurrentPhaseIndex].HealthRatioThreshold * MaxHp;
    }

    // 평상시에는 '다음에 진입할 페이즈'의 임계값이 하한이다.
    // 한 방에 여러 페이즈를 건너뛰는 것도 이 하한이 막아준다.
    if (Phases.IsValidIndex(CurrentPhaseIndex + 1))
    {
        return Phases[CurrentPhaseIndex + 1].HealthRatioThreshold * MaxHp;
    }

    // 마지막 페이즈까지 진입했으면 제한 없음 (죽을 수 있어야 한다).
    return 0.0f;
}

bool AR1Boss::BeginPhaseTransition(UAnimMontage* TransitionMontage)
{
    if (TransitionMontage == nullptr)
    {
        return false;
    }

    USkeletalMeshComponent* MeshComp = GetMesh();
    UAnimInstance* AnimInstance = MeshComp ? MeshComp->GetAnimInstance() : nullptr;
    if (AnimInstance == nullptr)
    {
        return false;
    }

    if (bIsInPhaseTransition)
    {
        // 이전 전환이 아직 안 끝났으면 먼저 정리한다 (Inhibit 이중 설정 방지).
        EndPhaseTransition();
    }

    if (AbilitySystemComponent)
    {
        // 1. 진행 중인 어빌리티를 끊는다. 안 끊으면 그 몽타주가 전환 연출을 덮어쓴다.
        AbilitySystemComponent->CancelAbilities();

        // 2. 전환이 끝날 때까지 '모든' 어빌리티 활성화를 막는다.
        //    UGameplayAbility::CanActivateAbility의 첫 번째 검사라서 BT 경유든
        //    게임플레이 이벤트(HitReact) 경유든 경로에 상관없이 막힌다.
        //    태그 하나만 막으면 어태크 어빌리티가 새 몽타주를 재생해 연출을 덮어썼다.
        AbilitySystemComponent->SetUserAbilityActivationInhibited(true);
    }

    // 3. 연출 중 미끄러지지 않도록 이동 정지 (HitReact와 같은 방식)
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        CachedMaxWalkSpeed = MoveComp->MaxWalkSpeed;
        MoveComp->MaxWalkSpeed = 0.0f;
        MoveComp->StopMovementImmediately();
    }

    // 4. 몽타주 재생. 재생 실패 시 위 상태를 원복하고 전환에 들어가지 않는다.
    const float Duration = AnimInstance->Montage_Play(TransitionMontage);
    if (Duration <= 0.0f)
    {
        UE_LOG(LogR1, Warning, TEXT("%s: phase transition montage failed to play"), *GetName());
        EndPhaseTransition();
        return false;
    }

    bIsInPhaseTransition = true;

    FOnMontageEnded EndDelegate;
    EndDelegate.BindUObject(this, &AR1Boss::OnPhaseTransitionMontageEnded);
    AnimInstance->Montage_SetEndDelegate(EndDelegate, TransitionMontage);

    UE_LOG(LogR1, Log, TEXT("%s: phase transition montage '%s' started (%.2fs)"), *GetName(), *TransitionMontage->GetName(), Duration);
    return true;
}

void AR1Boss::OnPhaseTransitionMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    UE_LOG(LogR1, Log, TEXT("%s: phase transition montage ended (interrupted: %d)"), *GetName(), bInterrupted ? 1 : 0);

    // 중단되어도 상태는 반드시 원복한다 — 안 그러면 보스가 무적인 채로 굳는다.
    EndPhaseTransition();
}

void AR1Boss::EndPhaseTransition()
{
    bIsInPhaseTransition = false;

    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->SetUserAbilityActivationInhibited(false);
    }

    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        // 격노 GE가 MoveSpeed를 올렸을 수 있으므로 어트리뷰트에서 다시 읽는다.
        const float RestoredSpeed = CoreAttributeSet ? CoreAttributeSet->GetMoveSpeed() : CachedMaxWalkSpeed;
        MoveComp->MaxWalkSpeed = RestoredSpeed > 0.0f ? RestoredSpeed : CachedMaxWalkSpeed;
    }
}
