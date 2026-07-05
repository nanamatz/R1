#include "System/R1DamageUISubsystem.h"
#include "System/R1DamageTextActor.h"

UR1DamageUISubsystem::UR1DamageUISubsystem()
{
	DamageActorClass = AR1DamageTextActor::StaticClass();
}

void UR1DamageUISubsystem::ShowDamageText(const FR1DamageInfo& DamageInfo)
{
	if (!DamageActorClass)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// 액터는 BeginPlay에서 SetLifeSpan으로 스스로 파괴됨 (풀링 없이 스폰-파괴)
	AR1DamageTextActor* DamageActor = GetWorld()->SpawnActor<AR1DamageTextActor>(DamageActorClass, DamageInfo.TargetLocation, FRotator::ZeroRotator, SpawnParams);
	if (DamageActor)
	{
		DamageActor->SetDamageInfo(DamageInfo);
	}
}
