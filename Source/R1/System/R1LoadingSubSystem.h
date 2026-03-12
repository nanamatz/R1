

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
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

public:
	// 🌟 로딩 화면이 사라질 때 울릴 방송
	UPROPERTY(BlueprintAssignable)
	FOnLoadingScreenHiddenSignature OnLoadingScreenHidden;
};
