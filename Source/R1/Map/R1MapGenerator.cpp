


#include "Map/R1MapGenerator.h"
#include "Data/RoomDataAsset.h"
#include "Engine/LevelStreamingDynamic.h" // 동적 레벨 스트리밍을 위한 헤더
#include "R1MapGenerator.h"
#include "Kismet/GameplayStatics.h" // 플레이어를 찾기 위한 헤더
#include "GameFramework/Character.h" // 캐릭터 조작을 위한 헤더

// Sets default values
AR1MapGenerator::AR1MapGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SpawnLocation = FVector(0.0f, 0.0f, 0.0f); // 기본 좌표 초기화
}

// Called when the game starts or when spawned
void AR1MapGenerator::BeginPlay()
{
	Super::BeginPlay();

	// 게임이 시작되자마자 테스트 방을 하나 생성해 봅니다.
	SpawnTestRoom();
}

void AR1MapGenerator::SpawnTestRoom()
{
    // 데이터 에셋이 제대로 할당되었는지 확인
    if (!TestRoomData)
    {
        UE_LOG(LogTemp, Error, TEXT("TestRoomData가 할당되지 않았습니다!"));
        return;
    }

    // 1. 데이터 에셋에서 불러올 레벨(맵)의 Soft 경로를 가져옵니다.
    TSoftObjectPtr<UWorld> LevelToLoad = TestRoomData->RoomLevelInstance;

    // 2. 경로가 비어있는지 체크
    if (LevelToLoad.IsNull())
    {
        UE_LOG(LogTemp, Error, TEXT("데이터 에셋 안에 레벨 인스턴스 경로가 비어있습니다."));
        return;
    }

    // 3. 스폰할 위치와 회전값 설정
    bool bOutSuccess = false;
    FRotator SpawnRotation = FRotator::ZeroRotator;

    // 4. 언리얼의 동적 레벨 스트리밍 함수 호출!
    ULevelStreamingDynamic* StreamingLevel = ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(
        this,
        LevelToLoad,
        SpawnLocation,
        SpawnRotation,
        bOutSuccess
    );

    // 결과 확인
    if (bOutSuccess)
    {
        UE_LOG(LogTemp, Warning, TEXT("방 로드중..."));
        StreamingLevel->OnLevelLoaded.AddDynamic(this, &AR1MapGenerator::OnRoomLoaded);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("방 스폰에 실패했습니다."));
    }
}

void AR1MapGenerator::OnRoomLoaded()
{
    UE_LOG(LogTemp, Warning, TEXT("방 로딩 완료! 플레이어를 안전하게 이동시킵니다."));

    // 현재 월드의 첫 번째 플레이어 캐릭터를 찾습니다.
    ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0);
    if (PlayerCharacter)
    {
        // 바닥에 파묻히지 않도록 Z축으로 살짝 띄워서(예: +100) 이동시킵니다.
        FVector SafeLocation = SpawnLocation + FVector(0.0f, 0.0f, 100.0f);

        // 플레이어를 로딩이 끝난 방의 중앙으로 강제 텔레포트!
        PlayerCharacter->SetActorLocation(SafeLocation);

        // 혹시 떨어지는 중이었다면 속도를 0으로 초기화
        PlayerCharacter->GetVelocity() = FVector::ZeroVector;
    }
}

