# Damage UI Subsystem Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Implement Task 3: Damage UI Subsystem with widget pooling for performance.

**Architecture:** Create a World Subsystem `UR1DamageUISubsystem` that manages a pool of `UR1DamageTextWidget`. It provides an interface to show damage text at a world location and handles returning widgets to the pool when they finish their animation.

**Tech Stack:** C++, Unreal Engine 5, UWorldSubsystem, UUserWidget.

---

### Task 1: Create Damage UI Subsystem Header

**Files:**
- Create: `Source/R1/System/R1DamageUISubsystem.h`

- [ ] **Step 1: Create the header file with the provided definition**

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "R1Define.h"
#include "R1DamageUISubsystem.generated.h"

class UR1DamageTextWidget;

UCLASS()
class R1_API UR1DamageUISubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Damage UI")
	void ShowDamageText(const FR1DamageInfo& DamageInfo);

	void ReturnWidgetToPool(UR1DamageTextWidget* Widget);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Damage UI")
	TSubclassOf<UR1DamageTextWidget> DamageWidgetClass;

private:
	UPROPERTY()
	TArray<TObjectPtr<UR1DamageTextWidget>> WidgetPool;

	UR1DamageTextWidget* GetWidgetFromPool();
};
```

### Task 2: Create Damage UI Subsystem Implementation

**Files:**
- Create: `Source/R1/System/R1DamageUISubsystem.cpp`

- [ ] **Step 1: Create the source file with the provided implementation**

```cpp
#include "System/R1DamageUISubsystem.h"
#include "UI/R1DamageTextWidget.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

void UR1DamageUISubsystem::ShowDamageText(const FR1DamageInfo& DamageInfo)
{
	UR1DamageTextWidget* Widget = GetWidgetFromPool();
	if (Widget)
	{
		Widget->SetDamageInfo(DamageInfo);
		
		if (!Widget->IsInViewport())
		{
			Widget->AddToViewport();
		}

		FVector2D ScreenPosition;
		if (UGameplayStatics::ProjectWorldToScreen(GetWorld()->GetFirstPlayerController(), DamageInfo.TargetLocation + FVector(0,0,100), ScreenPosition))
		{
			Widget->SetPositionInViewport(ScreenPosition);
		}
	}
}

void UR1DamageUISubsystem::ReturnWidgetToPool(UR1DamageTextWidget* Widget)
{
	if (Widget)
	{
		Widget->RemoveFromParent();
		WidgetPool.Add(Widget);
	}
}

UR1DamageTextWidget* UR1DamageUISubsystem::GetWidgetFromPool()
{
	if (WidgetPool.Num() > 0)
	{
		return WidgetPool.Pop();
	}

	if (DamageWidgetClass)
	{
		return CreateWidget<UR1DamageTextWidget>(GetWorld(), DamageWidgetClass);
	}

	return nullptr;
}
```

### Task 3: Update Damage Text Widget

**Files:**
- Modify: `Source/R1/UI/R1DamageTextWidget.cpp`

- [ ] **Step 1: Uncomment ReturnToPool implementation**

```cpp
void UR1DamageTextWidget::ReturnToPool()
{
	if (UWorld* World = GetWorld())
	{
		if (UR1DamageUISubsystem* DamageSS = World->GetSubsystem<UR1DamageUISubsystem>())
		{
			DamageSS->ReturnWidgetToPool(this);
		}
	}
}
```

### Task 4: Finalize and Commit

- [ ] **Step 1: Verify changes (no compilation errors in logic)**
- [ ] **Step 2: Commit the changes**

```bash
git add Source/R1/System/R1DamageUISubsystem.h Source/R1/System/R1DamageUISubsystem.cpp Source/R1/UI/R1DamageTextWidget.cpp
git commit -m "feat: add damage UI subsystem with pooling"
```
