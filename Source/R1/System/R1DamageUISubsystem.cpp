#include "System/R1DamageUISubsystem.h"
#include "System/R1DamageTextActor.h"

UR1DamageUISubsystem::UR1DamageUISubsystem()
{
	DamageActorClass = AR1DamageTextActor::StaticClass();
}

void UR1DamageUISubsystem::ShowDamageText(const FR1DamageInfo& DamageInfo)
{
	AR1DamageTextActor* DamageActor = GetActorFromPool();
	if (DamageActor)
	{
		DamageActor->SetActorLocation(DamageInfo.TargetLocation);
		DamageActor->SetActorHiddenInGame(false);
		DamageActor->SetDamageInfo(DamageInfo);
	}
}

void UR1DamageUISubsystem::ReturnActorToPool(AR1DamageTextActor* Actor)
{
	if (Actor)
	{
		Actor->SetActorHiddenInGame(true);
		ActorPool.Add(Actor);
	}
}

AR1DamageTextActor* UR1DamageUISubsystem::GetActorFromPool()
{
	if (ActorPool.Num() > 0)
	{
		return ActorPool.Pop();
	}

	if (DamageActorClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
		AR1DamageTextActor* NewActor = GetWorld()->SpawnActor<AR1DamageTextActor>(DamageActorClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		return NewActor;
	}

	return nullptr;
}