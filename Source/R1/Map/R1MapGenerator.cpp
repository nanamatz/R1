#include "Map/R1MapGenerator.h"
#include "Data/R1RoomDefinitionData.h"
#include "System/R1RoomStreamingSubsystem.h" // 스트리밍 서브시스템 헤더
#include "Engine/LevelStreamingDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Map/R1Door.h"
#include "Containers/Queue.h"
#include "Data/R1AssetData.h" 
#include "EngineUtils.h" // TActorIterator를 사용하기 위해 추가

AR1MapGenerator::AR1MapGenerator()
{
	PrimaryActorTick.bCanEverTick = false; // 제너레이터는 매 프레임 업데이트가 필요 없습니다.
}

void AR1MapGenerator::BeginPlay()
{
	Super::BeginPlay();

	// 1. 맵 생성 알고리즘이 돌기 전에, 라벨을 기반으로 풀(Pool)을 꽉 채웁니다.
	InitializeRoomPools();
	// 1. 게임이 시작되면 아이작 알고리즘으로 맵(배열 데이터)을 먼저 완성합니다.
	GenerateMap();

	// 2. 지도가 완성되었다면, 첫 번째 방(시작 방)을 실제로 스폰합니다.
	if (GeneratedMap.Num() > 0 && GeneratedMap[0].RoomDefinition != nullptr)
	{
		UR1RoomStreamingSubsystem* RoomSubsystem = GetGameInstance()->GetSubsystem<UR1RoomStreamingSubsystem>();
		if (RoomSubsystem)
		{
			// 서브시스템에 0번 방 스폰을 요청
			ULevelStreamingDynamic* StreamingLevel = RoomSubsystem->SpawnRoomLevel(
				GeneratedMap[0].RoomDefinition,
				GeneratedMap[0].SpawnLocation,
				FRotator::ZeroRotator
			);

			// 스폰 지시가 성공적으로 들어갔다면, 완료 델리게이트 연결
			if (StreamingLevel)
			{
				UE_LOG(LogTemp, Warning, TEXT("[MapGenerator] 0번 방 스폰을 요청했습니다. 대기 중..."));
				StreamingLevel->OnLevelLoaded.AddDynamic(this, &AR1MapGenerator::OnRoomLoaded);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[MapGenerator] 맵이 생성되지 않았거나 시작 방 데이터가 없습니다!"));
	}
}

void AR1MapGenerator::GenerateMap()
{
	int32 MaxRetries = 50; // 확률로 인해 맵이 덜 만들어졌을 때 재시도할 최대 횟수
	bool bMapGeneratedSuccessfully = false;

	while (MaxRetries > 0 && !bMapGeneratedSuccessfully)
	{
		GeneratedMap.Empty();
		TQueue<int32> RoomQueue;
		int32 CurrentNodeID = 0;

		// 1. 시작 방 생성 (그리드의 중심인 0,0 좌표에서 시작)
		FR1MapNode StartNode;
		StartNode.NodeID = CurrentNodeID;
		StartNode.GridPosition = FIntPoint(0, 0);
		StartNode.SpawnLocation = FVector(0.0f, 0.0f, 0.0f);

		GeneratedMap.Add(StartNode);
		RoomQueue.Enqueue(CurrentNodeID);
		CurrentNodeID++;

		// 2. BFS 방식 가지치기 루프
		while (!RoomQueue.IsEmpty() && CurrentNodeID < TotalRoomCount)
		{
			int32 ParentID;
			RoomQueue.Dequeue(ParentID);

			// 원본 배열 참조가 깨질 수 있으므로 좌표값만 복사해서 사용
			FIntPoint ParentPos = GeneratedMap[ParentID].GridPosition;

			// 방향 무작위 셔플 (한쪽으로만 뻗어나가는 것을 방지)
			TArray<FIntPoint> Directions = { FIntPoint(0, 1), FIntPoint(0, -1), FIntPoint(-1, 0), FIntPoint(1, 0) };
			for (int32 i = Directions.Num() - 1; i > 0; i--)
			{
				int32 j = FMath::RandRange(0, i);
				Directions.Swap(i, j);
			}

			// 인접한 4방향 탐색
			for (FIntPoint Dir : Directions)
			{
				if (CurrentNodeID >= TotalRoomCount) break; // 목표 개수 달성 시 중단

				FIntPoint NewPos = ParentPos + Dir;

				// 이미 이 자리에 방이 있는지 체크
				if (HasRoomAt(NewPos)) continue;

				// [아이작 핵심 규칙] 뭉침 방지: 새 위치 주변에 방이 2개 이상 있다면 생성 취소
				int32 NeighborCount = 0;
				FIntPoint CheckDirs[4] = { FIntPoint(0, 1), FIntPoint(0, -1), FIntPoint(-1, 0), FIntPoint(1, 0) };
				for (FIntPoint CheckDir : CheckDirs)
				{
					if (HasRoomAt(NewPos + CheckDir)) NeighborCount++;
				}

				if (NeighborCount > 1) continue;

				// 50% 확률로 새 방 생성
				if (FMath::RandRange(0, 100) < 50)
				{
					FR1MapNode NewNode;
					NewNode.NodeID = CurrentNodeID;
					NewNode.GridPosition = NewPos;

					// 실제 월드 좌표 = 그리드 좌표 * 우리가 정한 방 간격
					NewNode.SpawnLocation = FVector(NewPos.X * RoomSpacing, NewPos.Y * RoomSpacing, 0.0f);

					// 서로 연결 상태 기록
					NewNode.ConnectedNodeIDs.Add(ParentID);
					GeneratedMap.Add(NewNode);
					GeneratedMap[ParentID].ConnectedNodeIDs.Add(CurrentNodeID);

					RoomQueue.Enqueue(CurrentNodeID);
					CurrentNodeID++;
				}
			}
		}

		// 목표한 방 개수를 정확히 채웠는지 확인
		if (GeneratedMap.Num() == TotalRoomCount)
		{
			bMapGeneratedSuccessfully = true;
		}

		MaxRetries--;
	}

	// 3. 지도가 완성되었다면, 데이터(PDA)를 각 방에 할당합니다.
	if (bMapGeneratedSuccessfully)
	{
		AssignRoomTypes();
		UE_LOG(LogTemp, Warning, TEXT("[MapGenerator] 총 %d개의 방 지도 생성이 완료되었습니다!"), GeneratedMap.Num());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[MapGenerator] 맵 생성 실패: 확률 문제로 목표 개수 미달. 다시 플레이해주세요."));
	}
}

void AR1MapGenerator::InitializeRoomPools()
{
	if (!GlobalAssetData)
	{
		UE_LOG(LogTemp, Error, TEXT("[MapGenerator] GlobalAssetData가 할당되지 않았습니다!"));
		return;
	}

	// 람다(Lambda) 함수를 활용해 라벨 이름만 주면 알아서 풀을 채우도록 만듭니다.
	auto LoadPoolByLabel = [&](FName Label, TArray<UR1RoomDefinitionData*>& OutPool)
		{
			OutPool.Empty();

			// 1. 라벨에 해당하는 에셋 목록을 글로벌 데이터에서 긁어옵니다.
			const FAssetSet& AssetSet = GlobalAssetData->GetAssetSetByLabel(Label);

			// 2. 긁어온 Soft 경로들을 순회하며 메모리에 로드(TryLoad)합니다.
			for (const FAssetEntry& Entry : AssetSet.AssetEntries)
			{
				// 주의: 여기서 로드하는 것은 '무거운 3D 맵'이 아니라 '가벼운 PDA 명세서'입니다.
				// 텍스트 데이터에 불과하므로 동기 로드(TryLoad)를 해도 프레임 드랍이 전혀 없습니다!
				UObject* LoadedObj = Entry.AssetPath.TryLoad();

				// 3. 로드된 오브젝트가 방 데이터(UR1RoomDefinitionData)가 맞다면 배열에 넣습니다.
				if (UR1RoomDefinitionData* RoomData = Cast<UR1RoomDefinitionData>(LoadedObj))
				{
					OutPool.Add(RoomData);
				}
			}

			UE_LOG(LogTemp, Log, TEXT("[MapGenerator] '%s' 풀 로드 완료: %d개"), *Label.ToString(), OutPool.Num());
		};

	// 이제 원하는 라벨만 던져주면 끝입니다!
	LoadPoolByLabel(FName("Room_Start"), StartRoomPool);
	LoadPoolByLabel(FName("Room_Combat"), CombatRoomPool);
	LoadPoolByLabel(FName("Room_Boss"), BossRoomPool);
}

void AR1MapGenerator::AssignRoomTypes()
{
	// 에디터에서 풀을 안 채워놨을 경우를 대비한 방어 로직
	if (StartRoomPool.IsEmpty() || CombatRoomPool.IsEmpty() || BossRoomPool.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("[MapGenerator] Room Pool이 비어있습니다. 에디터에서 PDA를 채워주세요!"));
		return;
	}

	// 0번 방(시작 지점)은 무조건 시작 방 세팅
	GeneratedMap[0].RoomDefinition = StartRoomPool[FMath::RandRange(0, StartRoomPool.Num() - 1)];

	int32 FurthestNodeID = -1;
	float MaxDistance = -1.0f;

	// 1번 방부터 순회하며 속성 찾기
	for (int32 i = 1; i < GeneratedMap.Num(); ++i)
	{
		// 기본적으로 모든 방을 일반(Combat) 방으로 깔아둡니다.
		GeneratedMap[i].RoomDefinition = CombatRoomPool[FMath::RandRange(0, CombatRoomPool.Num() - 1)];

		// 연결된 방이 1개뿐이라면 = '막다른 길(Dead End)'
		if (GeneratedMap[i].ConnectedNodeIDs.Num() == 1)
		{
			// 시작점(0,0)으로부터의 거리를 계산합니다.
			float Dist = FVector::Dist(FVector::ZeroVector, FVector(GeneratedMap[i].GridPosition.X, GeneratedMap[i].GridPosition.Y, 0));
			if (Dist > MaxDistance)
			{
				MaxDistance = Dist;
				FurthestNodeID = i;
			}
		}
	}

	// 시작 방에서 가장 멀리 떨어진 '막다른 길'을 찾아 '보스 방'으로 덮어씌웁니다.
	if (FurthestNodeID != -1)
	{
		GeneratedMap[FurthestNodeID].RoomDefinition = BossRoomPool[FMath::RandRange(0, BossRoomPool.Num() - 1)];
		UE_LOG(LogTemp, Warning, TEXT("[MapGenerator] 보스 방이 %d번 노드에 할당되었습니다."), FurthestNodeID);
	}
}

bool AR1MapGenerator::HasRoomAt(FIntPoint Pos)
{
	for (const FR1MapNode& Node : GeneratedMap)
	{
		if (Node.GridPosition == Pos) return true;
	}
	return false;
}

int32 AR1MapGenerator::GetConnectedNodeInDirection(int32 CurrentNodeID, ER1DoorDirection Direction)
{
	if (!GeneratedMap.IsValidIndex(CurrentNodeID)) return -1;

	const FR1MapNode& CurrentNode = GeneratedMap[CurrentNodeID];
	FIntPoint TargetGridPos = CurrentNode.GridPosition;

	// 1. 타겟 방향의 가상 그리드 좌표를 계산합니다.
	switch (Direction)
	{
	case ER1DoorDirection::North: TargetGridPos.Y += 1; break;
	case ER1DoorDirection::South: TargetGridPos.Y -= 1; break;
	case ER1DoorDirection::East:  TargetGridPos.X += 1; break;
	case ER1DoorDirection::West:  TargetGridPos.X -= 1; break;
	default: return -1;
	}

	// 2. 현재 방과 "연결된(Connected)" 방들 중에서, 타겟 좌표에 위치한 방이 있는지 검사합니다.
	for (int32 ConnectedID : CurrentNode.ConnectedNodeIDs)
	{
		if (GeneratedMap.IsValidIndex(ConnectedID) && GeneratedMap[ConnectedID].GridPosition == TargetGridPos)
		{
			return ConnectedID; // 찾았습니다! 그 방의 번호를 반환합니다.
		}
	}

	return -1; // 해당 방향으로 뚫린 방이 없습니다.
}

void AR1MapGenerator::OnPlayerEnteredDoor(ER1DoorDirection Direction)
{
	int32 NextNodeID = GetConnectedNodeInDirection(CurrentActiveNodeID, Direction);

	if (NextNodeID != -1)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MapGenerator] 문 통과 감지! %d번 방에서 %d번 방으로 텔레포트를 시도합니다."), CurrentActiveNodeID, NextNodeID);

		// 다음 스텝: 이 곳에서 서브시스템을 호출해 NextNodeID 방을 스폰하고, 
		// 플레이어를 다음 방의 '반대편 문' 앞으로 이동시키는 로직을 작성하게 됩니다!
	}
}

void AR1MapGenerator::OnRoomLoaded()
{
	UE_LOG(LogTemp, Warning, TEXT("[MapGenerator] 0번 시작 방 로딩 완료! 플레이어를 안전하게 이동시킵니다."));

	// 1. 플레이어 강제 이동
	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0);
	if (PlayerCharacter)
	{
		// 0번 방의 스폰 위치(0,0,0)에서 위로 살짝 띄운 곳으로 텔레포트
		FVector SafeLocation = GeneratedMap[0].SpawnLocation + FVector(0.0f, 0.0f, 150.0f);
		PlayerCharacter->SetActorLocation(SafeLocation);
		PlayerCharacter->GetVelocity() = FVector::ZeroVector;
	}

	// 2. [가장 중요] 서브시스템에게 "이 방은 플레이 준비가 끝났으니(Hot) 메모리에서 지우지 마!" 라고 보고
	UR1RoomStreamingSubsystem* RoomSubsystem = GetGameInstance()->GetSubsystem<UR1RoomStreamingSubsystem>();
	if (RoomSubsystem)
	{
		RoomSubsystem->MarkRoomGameplayReady(GeneratedMap[0].RoomDefinition);
	}
	FTimerHandle SetupTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(SetupTimerHandle, FTimerDelegate::CreateLambda([this]()
		{
			UE_LOG(LogTemp, Warning, TEXT("[MapGenerator] 타이머 발동! 문 세팅을 시작합니다."));

			// --- 여기서부터 기존 문 찾는 로직 시작 ---
			FVector CurrentRoomLocation = GeneratedMap[CurrentActiveNodeID].SpawnLocation;
			int32 FoundDoors = 0;

			for (TActorIterator<AR1Door> It(GetWorld()); It; ++It)
			{
				AR1Door* Door = *It;

				// 다시 거리를 필터링해 줍니다 (방이 크다면 15000 등 넉넉하게!)
				if (FVector::Dist(Door->GetActorLocation(), CurrentRoomLocation) < 15000.0f)
				{
					FoundDoors++;
					int32 TargetNode = GetConnectedNodeInDirection(CurrentActiveNodeID, Door->DoorDirection);

					UE_LOG(LogTemp, Warning, TEXT("[문 세팅] %d번 방 문 세팅. 방향: %d, 타겟 방: %d"),
						CurrentActiveNodeID, (int32)Door->DoorDirection, TargetNode);

					Door->SetupDoorConnection(TargetNode);

					if (TargetNode != -1)
					{
						Door->OnDoorEntered.RemoveDynamic(this, &AR1MapGenerator::OnPlayerEnteredDoor);
						Door->OnDoorEntered.AddDynamic(this, &AR1MapGenerator::OnPlayerEnteredDoor);
					}
				}
			}
			UE_LOG(LogTemp, Warning, TEXT("[문 세팅 완료] 총 %d개의 문을 찾았습니다."), FoundDoors);

			// 3. 플레이어 강제 이동 (문 세팅이 끝난 후 안전하게 이동)
			ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0);
			if (PlayerCharacter)
			{
				FVector SafeLocation = GeneratedMap[CurrentActiveNodeID].SpawnLocation + FVector(0.0f, 0.0f, 150.0f);
				PlayerCharacter->SetActorLocation(SafeLocation);
				PlayerCharacter->GetVelocity() = FVector::ZeroVector;
			}

		}), 0.1f, false); // 0.1초 뒤에 1번만 실행
}