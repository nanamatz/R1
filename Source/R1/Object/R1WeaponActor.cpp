#include "Object/R1WeaponActor.h"
#include "Components/StaticMeshComponent.h"

AR1WeaponActor::AR1WeaponActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(Root);
	// 콜리전을 끕니다 (무기가 캐릭터를 밀어내는 현상 방지 — 기존 무기 메시 경로와 동일)
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
