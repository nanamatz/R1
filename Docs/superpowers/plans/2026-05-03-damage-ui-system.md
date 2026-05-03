# Damage UI System Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Implement a floating damage text system that displays damage numbers above characters using a pooled World Subsystem.

**Architecture:** Use `UWorldSubsystem` to manage a pool of `UUserWidget` instances. Intercept health changes in GAS `AttributeSet` to trigger the UI events.

**Tech Stack:** C++, Unreal Engine 5.x, Gameplay Ability System (GAS), UMG.

---

### Task 1: Data Structures

**Files:**
- Modify: `Source/R1/R1Define.h`

- [ ] **Step 1: Add `EDamageType` and `FDamageInfo` to `R1Define.h`**

```cpp
// Source/R1/R1Define.h

UENUM(BlueprintType)
enum class ER1DamageType : uint8
{
	Normal,
	Critical,
	Heal
};

USTRUCT(BlueprintType)
struct FR1DamageInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageAmount = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector TargetLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ER1DamageType DamageType = ER1DamageType::Normal;
};
```

- [ ] **Step 2: Commit**

```bash
git add Source/R1/R1Define.h
git commit -m "feat: add damage info data structures"
```

---

### Task 2: Damage Text Widget Base Class

**Files:**
- Create: `Source/R1/UI/R1DamageTextWidget.h`
- Create: `Source/R1/UI/R1DamageTextWidget.cpp`

- [ ] **Step 1: Create `R1DamageTextWidget.h`**

```cpp
// Source/R1/UI/R1DamageTextWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "R1Define.h"
#include "R1DamageTextWidget.generated.h"

UCLASS()
class R1_API UR1DamageTextWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Damage UI")
	void SetDamageInfo(const FR1DamageInfo& Info);

	UFUNCTION(BlueprintImplementableEvent, Category = "Damage UI")
	void OnSetDamageInfo(const FR1DamageInfo& Info);

	void ReturnToPool();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage UI")
	TObjectPtr<class UWidgetAnimation> FloatAnim;

	// This function will be called by the animation finish or a timer in BP
	UFUNCTION(BlueprintCallable, Category = "Damage UI")
	void HandleAnimationFinished();
};
```

- [ ] **Step 2: Create `R1DamageTextWidget.cpp`**

```cpp
// Source/R1/UI/R1DamageTextWidget.cpp
#include "UI/R1DamageTextWidget.h"
#include "System/R1DamageUISubsystem.h"

void UR1DamageTextWidget::SetDamageInfo(const FR1DamageInfo& Info)
{
	OnSetDamageInfo(Info);
	// In BP, you would play FloatAnim here.
}

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

void UR1DamageTextWidget::HandleAnimationFinished()
{
	ReturnToPool();
}
```

- [ ] **Step 3: Commit**

```bash
git add Source/R1/UI/R1DamageTextWidget.h Source/R1/UI/R1DamageTextWidget.cpp
git commit -m "feat: add damage text widget base class"
```

---

### Task 3: Damage UI Subsystem

**Files:**
- Create: `Source/R1/System/R1DamageUISubsystem.h`
- Create: `Source/R1/System/R1DamageUISubsystem.cpp`

- [ ] **Step 1: Create `R1DamageUISubsystem.h`**

```cpp
// Source/R1/System/R1DamageUISubsystem.h
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

- [ ] **Step 2: Create `R1DamageUISubsystem.cpp`**

```cpp
// Source/R1/System/R1DamageUISubsystem.cpp
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
		
		// Optional: Screen projection logic if not using a 3D widget component
		// For screen widgets, we set the position later or use a HUD overlay.
		if (!Widget->IsInViewport())
		{
			Widget->AddToViewport();
		}

		// Basic positioning (can be refined in BP or here)
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

- [ ] **Step 3: Commit**

```bash
git add Source/R1/System/R1DamageUISubsystem.h Source/R1/System/R1DamageUISubsystem.cpp
git commit -m "feat: add damage UI subsystem with pooling"
```

---

### Task 4: GAS Integration

**Files:**
- Modify: `Source/R1/AbilitySystem/Attribute/R1AttributeSet.cpp`

- [ ] **Step 1: Trigger `ShowDamageText` in `PostGameplayEffectExecute`**

```cpp
// Source/R1/AbilitySystem/Attribute/R1AttributeSet.cpp
// ... existing includes ...
#include "System/R1DamageUISubsystem.h"

// Inside PostGameplayEffectExecute, where damage is handled:
if (Data.EvaluatedData.Magnitude < 0.0f && Character->GetCreatureState() != ECreatureState::Dead)
{
    float DamageAmount = FMath::Abs(Data.EvaluatedData.Magnitude);
    
    FR1DamageInfo DamageInfo;
    DamageInfo.DamageAmount = DamageAmount;
    DamageInfo.TargetLocation = Character->GetActorLocation();
    DamageInfo.DamageType = ER1DamageType::Normal; // Default for now, expand if crit logic added

    if (UWorld* World = Character->GetWorld())
    {
        if (UR1DamageUISubsystem* DamageSS = World->GetSubsystem<UR1DamageUISubsystem>())
        {
            DamageSS->ShowDamageText(DamageInfo);
        }
    }
}
```

- [ ] **Step 2: Commit**

```bash
git add Source/R1/AbilitySystem/Attribute/R1AttributeSet.cpp
git commit -m "feat: integrate damage UI with AttributeSet"
```

---

### Task 5: Final Review & Polish

- [ ] **Step 1: Verify all headers are correctly included and there are no circular dependencies.**
- [ ] **Step 2: Ensure `R1Define.h` changes don't break existing code.**
- [ ] **Step 3: Final Commit.**
