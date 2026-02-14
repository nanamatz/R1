


#include "AI/R1AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BehaviorTree/BlackboardComponent.h"
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

	// 2. 델리게이트 바인딩 (이벤트 연결)
	if (AIPerception)
	{
		AIPerception->OnTargetPerceptionUpdated.AddDynamic(this, &AR1AIController::OnTargetPerceptionUpdated);
	}

	// 3. 몬스터의 AttributeSet에서 AggroRange를 가져와서 시야 거리에 동기화!
	AR1Monster* Monster = Cast<AR1Monster>(InPawn);
	if (Monster && SightConfig && AIPerception)
	{
		float FindRange = Monster->AggroRange; // (혹은 AttributeSet에서 Get)

		SightConfig->SightRadius = FindRange;
		SightConfig->LoseSightRadius = FindRange + 150.0f; // 놓치는 거리는 살짝 더 길게

		AIPerception->ConfigureSense(*SightConfig); // 설정 덮어쓰기
		AIPerception->RequestStimuliListenerUpdate(); // 업데이트 요청
	}
}

void AR1AIController::BeginPlay()
{
	Super::BeginPlay();
	//FVector Dest = { 0,0,0 };
	//FAIMoveRequest MoveRequest;
	//MoveRequest.SetGoalLocation(Dest);
	//MoveRequest.SetAcceptanceRadius(15.f);

	//FNavPathSharedPtr NavPath;	// 이동 경로를 따로 처리해주고 싶을 때 사용

	//MoveTo(MoveRequest, OUT &NavPath);

	//if (NavPath.IsValid())
	//{
	//	TArray<FNavPathPoint>& PathPoints = NavPath->GetPathPoints();
	//	for (const auto& Point : PathPoints)
	//	{
	//		const FVector& Location = Point.Location;
	//		DrawDebugSphere(GetWorld(), Location, 12.f, 12, FColor::Green, false, 10.0f);
	//	}
	//}
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
