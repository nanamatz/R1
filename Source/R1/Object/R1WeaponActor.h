

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "R1WeaponActor.generated.h"

/**
 * 장착 무기 비주얼 컨테이너 액터. 게임플레이 로직 없음.
 * BP 자식에서 WeaponMesh에 메시를 지정하고 오라/글로우 등 VFX 컴포넌트를 뷰포트에서 정렬한다.
 * 규칙: WeaponMesh는 Root 기준 아이덴티티 트랜스폼 유지 (Root가 손 소켓에 스냅되므로 메시를 옮기면 그립이 어긋남).
 */
UCLASS()
class R1_API AR1WeaponActor : public AActor
{
	GENERATED_BODY()

public:
	AR1WeaponActor();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UStaticMeshComponent> WeaponMesh;
};
