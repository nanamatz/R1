

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/TimerHandle.h"
#include "R1LoadingSubSystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLoadingScreenHiddenSignature);
class UR1ProgressWidget;
class AR1MapGenerator;
/**
 * 
 */
UCLASS()
class R1_API UR1LoadingSubSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

private:
	// 화면에 띄울 로딩 위젯 인스턴스 보관용
	UPROPERTY()
	TObjectPtr<class UR1LoadingScreenWidget> LoadingWidget;

	// 로딩 화면 해제 게이트: 두 조건이 모두 충족되어야 화면을 내립니다.
	bool bSceneDone = false;     // 위젯 연출(OnSceneFinished) 완료
	bool bContentReady = false;  // 모든 방 로드 완료(NotifyContentReady)

	// 화면 해제 예약 타이머 핸들. 멤버로 보관해야 다음 층 재로딩 시 취소할 수 있습니다.
	FTimerHandle HideTimerHandle;

	void TryHideLoadingScreen();

public:
	// 1. 로딩 화면을 띄우고 맵 제너레이터와 연결하는 함수
	UFUNCTION(BlueprintCallable, Category = "Loading")
	void ShowLoadingScreen(TSubclassOf<UR1LoadingScreenWidget> WidgetClass, AR1MapGenerator* MapGenerator);

	// 2. 맵 제너레이터의 방송을 수신할 콜백 함수
	UFUNCTION()
	void OnProgressUpdated(float CurrentProgress);

	// 3. 로딩이 모두 끝났을 때 위젯을 치우고 페이드 인을 실행할 함수
	UFUNCTION()
	void HideLoadingScreen();

	UFUNCTION()
	void OnVisualsCompleted();

	// 맵 제너레이터가 "이 층의 모든 방 로드가 끝났다"고 알릴 때 호출. 게이트의 두 번째 조건.
	UFUNCTION(BlueprintCallable, Category = "Loading")
	void NotifyContentReady();

public:
	// 🌟 로딩 화면이 사라질 때 울릴 방송
	UPROPERTY(BlueprintAssignable)
	FOnLoadingScreenHiddenSignature OnLoadingScreenHidden;
};
