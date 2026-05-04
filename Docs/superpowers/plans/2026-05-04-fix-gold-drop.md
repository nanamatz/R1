# Gold Drop Logic Fix Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Fix the gold dropping logic so that gold actors spawn at ground level, perform a "pop" animation, and settle on the floor instead of floating in the air.

**Architecture:** Use a line trace from the monster's location to the ground to determine the correct spawn height. Update the gold actor to handle physics settling correctly by ignoring irrelevant collisions.

**Tech Stack:** Unreal Engine 5 (C++), PhysX/Chaos Physics.

---

### Task 1: Update AR1Monster to Spawn Gold at Ground Level

**Files:**
- Modify: `Source/R1/Character/R1Monster.cpp:164-180`

- [ ] **Step 1: Modify DropGold to use line tracing**

Update `AR1Monster::DropGold` to trace downward from the monster's center to find the ground.

```cpp
void AR1Monster::DropGold()
{
	if (!GoldActorClass)
	{
		UE_LOG(LogTemp, Error, TEXT("몬스터 블루프린트에 GoldActorClass가 할당되지 않았습니다!"));
		return;
	}

	if (FMath::RandRange(0.0f, 1.0f) < GoldDropChance)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		FVector SpawnLocation = GetActorLocation();
		FRotator SpawnRotation = FRotator::ZeroRotator;

		// 🌟 Find the ground to avoid floating gold
		FHitResult HitResult;
		FVector Start = SpawnLocation;
		FVector End = Start - FVector(0, 0, 1000.0f); // Trace down 10m
		FCollisionQueryParams TraceParams;
		TraceParams.AddIgnoredActor(this);

		if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, TraceParams))
		{
			// Spawn at hit location + offset (GoldActor radius is 50, so 55 is safe)
			SpawnLocation = HitResult.Location + FVector(0, 0, 55.0f);
		}

		AR1GoldActor* DroppedGold = GetWorld()->SpawnActor<AR1GoldActor>(GoldActorClass, SpawnLocation, SpawnRotation, SpawnParams);

		if (DroppedGold)
		{
			int32 FinalAmount = FMath::RandRange(MinGoldDrop, MaxGoldDrop);
			DroppedGold->SetGoldAmount(FinalAmount);
		}
	}
}
```

- [ ] **Step 2: Commit changes**

```bash
git add Source/R1/Character/R1Monster.cpp
git commit -m "fix(gold): spawn gold at ground level using line trace"
```

---

### Task 2: Refine AR1GoldActor Collision Handling

**Files:**
- Modify: `Source/R1/Object/R1GoldActor.cpp:138-145`

- [ ] **Step 1: Update OnSphereHit to ignore Pawns**

Ensure the gold doesn't stop physics if it hits a monster or the player on the way down.

```cpp
void AR1GoldActor::OnSphereHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// 🌟 Ignore hits with pawns (monsters/players) to ensure it hits the ground
	if (OtherActor && OtherActor->IsA<APawn>())
	{
		return;
	}

	if (OtherComp)
	{
		// 2. 더 이상 Hit 이벤트가 연달아 터지지 않도록 즉시 차단
		SphereComp->SetNotifyRigidBodyCollision(false);
		SphereComp->OnComponentHit.RemoveDynamic(this, &AR1GoldActor::OnSphereHit);

		// 🌟 3. 미끄러짐이나 덜덜 떨리는 현상이 없도록 남아있는 관성과 회전력을 박살 냅니다!
		SphereComp->SetPhysicsLinearVelocity(FVector::ZeroVector);
		SphereComp->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);

		// 4. 관성을 죽인 후 안전하게 물리 연산 종료 및 오버랩 전환 (기존 함수 재활용)
		DisablePhysicsAndSetOverlap();

		if (WorldDropSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, WorldDropSound, GetActorLocation());
		}
	}
}
```

- [ ] **Step 2: Commit changes**

```bash
git add Source/R1/Object/R1GoldActor.cpp
git commit -m "fix(gold): ignore pawns in gold actor hit detection"
```
