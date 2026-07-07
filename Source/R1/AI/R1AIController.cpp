


#include "AI/R1AIController.h"
#include "R1LogChannels.h"
#include "Navigation/PathFollowingComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/R1AbilitySystemComponent.h"
#include "AbilitySystem/Attribute/MonsterAttributeSet.h"

#include "Character/R1Player.h"
#include "Character/R1Monster.h"

AR1AIController::AR1AIController(const FObjectInitializer& ObjectInitializer)
{
	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));

	if (AIPerception && SightConfig)
	{
		// 기본 시야 세팅 (나중에 OnPossess에서 몬스터 스탯으로 덮어씌웁니다)
		SightConfig->SightRadius = 500.0f;
		SightConfig->LoseSightRadius = 600.0f; // 시야에서 벗어나는 판정 거리 (보통 좀 더 긺)
		SightConfig->PeripheralVisionAngleDegrees = 180.0f; // 360도 전방위 감지 원하면 180으로 설정
		SightConfig->SetMaxAge(5.0f); // 5초 뒤에 기억에서 지움

		// ✨ 중요: 팀 시스템(IGenericTeamAgentInterface)을 안 쓴다면 무조건 다 true로 해야 감지됩니다!
		SightConfig->DetectionByAffiliation.bDetectEnemies = true;
		SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
		SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

		AIPerception->ConfigureSense(*SightConfig);
		AIPerception->SetDominantSense(SightConfig->GetSenseImplementation());
	}
}

void AR1AIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	AR1Monster* Monster = Cast<AR1Monster>(InPawn);
	if (Monster)
	{
		UBehaviorTree* BTAsset = Monster->GetBehaviorTree();
		if (BTAsset)
		{
			// 트리가 있다면 블랙보드를 초기화하고 트리를 가동시킵니다.
			RunBehaviorTree(BTAsset);
		}
		else
		{
			UE_LOG(LogR1, Warning, TEXT("몬스터(%s)에 할당된 비헤이비어 트리가 없습니다!"), *Monster->GetName());
		}
	}

	// 2. 델리게이트 바인딩 (이벤트 연결)
	if (AIPerception)
	{
		AIPerception->OnTargetPerceptionUpdated.AddDynamic(this, &AR1AIController::OnTargetPerceptionUpdated);
	}

	if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InPawn))
	{
		bool bFound = false;
		float CurrentRange = ASC->GetGameplayAttributeValue(UMonsterAttributeSet::GetAggroRangeAttribute(), bFound);
		if (bFound)
		{
			UpdateSightRange(CurrentRange);
		}

		ASC->GetGameplayAttributeValueChangeDelegate(UMonsterAttributeSet::GetAggroRangeAttribute()).AddUObject(this, &AR1AIController::OnAggroRangeChanged);
	}
}

void AR1AIController::BeginPlay()
{
	Super::BeginPlay();
}

void AR1AIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// 4. 이벤트 실행부 (기존 BTService_FindTarget의 역할을 완벽히 대체)
void AR1AIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	// 감지된 액터가 플레이어인지 확인
	AR1Player* Player = Cast<AR1Player>(Actor);
	if (Player)
	{
		UBlackboardComponent* BB = GetBlackboardComponent();
		if (BB)
		{
			// 시야에 들어왔을 때 (SuccessfullySensed == true)
			if (Stimulus.WasSuccessfullySensed())
			{
				// 플레이어가 죽지 않았다면 타겟으로 설정
				if (Player->GetCreatureState() != ECreatureState::Dead)
				{
					BB->SetValueAsObject(TargetKeyName, Player); // 블랙보드 키 이름에 맞춰 변경하세요!
				}
			}
			// 시야에서 벗어났을 때 (SuccessfullySensed == false)
			else
			{
				BB->SetValueAsObject(TargetKeyName, nullptr);
			}
		}
	}
}

void AR1AIController::OnAggroRangeChanged(const FOnAttributeChangeData& Data)
{
	UpdateSightRange(Data.NewValue);
}

void AR1AIController::UpdateSightRange(float NewRange)
{
	if (!AIPerception) return;

	FAISenseID SightSenseID = UAISense::GetSenseID(UAISense_Sight::StaticClass());
	UAISenseConfig_Sight* CurrentSightConfig = Cast<UAISenseConfig_Sight>(AIPerception->GetSenseConfig(SightSenseID));

	if (CurrentSightConfig)
	{
		// 꺼내온 설정값 덮어쓰기
		CurrentSightConfig->SightRadius = NewRange;

		CurrentSightConfig->LoseSightRadius = NewRange + 150.0f;

		CurrentSightConfig->DetectionByAffiliation.bDetectEnemies = true;
		CurrentSightConfig->DetectionByAffiliation.bDetectNeutrals = true;
		CurrentSightConfig->DetectionByAffiliation.bDetectFriendlies = true;

		// 다시 적용하고 퍼셉션 업데이트 요청
		AIPerception->ConfigureSense(*CurrentSightConfig);
		AIPerception->SetDominantSense(CurrentSightConfig->GetSenseImplementation());
		AIPerception->RequestStimuliListenerUpdate();

		UE_LOG(LogR1, Warning, TEXT("[AI Controller] 몬스터 시야 세팅/업데이트 완료! 새로운 반경: %f"), NewRange);
	}
}
