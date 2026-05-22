# CLAUDE.md

Claude Code가 이 프로젝트에서 작업할 때 반드시 따라야 할 규칙입니다.
잘 모르겠으면 추측하지 말고 물어볼 것.

---

## 1. Project Overview

- **Engine**: Unreal Engine 5.3
- **Language**: C++ (primary), Blueprint (디자이너용 로직 / 에디터 조립)
- **IDE / Toolchain**: Visual Studio 2022, MSVC, UnrealBuildTool(UBT)
- **Core Framework**: Gameplay Ability System (GAS) — 적극 사용 중
- **Network Mode**: **싱글플레이 전용 (Standalone)**. 멀티플레이 미지원.
- **Target Platform**: Win64

> `<UE_PATH>`, `<ProjectName>`, `<ProjectPath>` 등은 실제 환경 값으로 채워서 사용하세요.

---

## 2. Build & Compile

UE 프로젝트는 일반 C++ 프로젝트와 빌드 방식이 다릅니다. 아래 명령을 사용하세요.

```bat
:: 에디터 타깃 빌드 (Development)
"<UE_PATH>\Engine\Build\BatchFiles\Build.bat" <ProjectName>Editor Win64 Development "<ProjectPath>\<ProjectName>.uproject" -waitmutex

:: 프로젝트 파일 재생성 (.sln 갱신)
"<UE_PATH>\Engine\Build\BatchFiles\GenerateProjectFiles.bat" -project="<ProjectPath>\<ProjectName>.uproject" -game -rocket
```

### Live Coding 주의
- 함수 본문(.cpp)만 수정 → Live Coding(Ctrl+Alt+F11) 가능
- **헤더 변경(`UCLASS`/`UPROPERTY`/`UFUNCTION` 추가·수정, 멤버 추가)은 Live Coding으로 반영되지 않음** → 에디터를 종료하고 VS2022에서 빌드 후 재실행
- 빌드 에러가 나면 `Binaries`, `Intermediate` 삭제 후 재빌드를 고려

---

## 3. Coding Conventions (Epic Standard 준수)

### Naming
- 클래스 접두사 필수:
  - `U` = UObject 상속, `A` = Actor 상속, `F` = struct / 일반 클래스
  - `I` = Interface, `E` = enum, `T` = template, `S` = Slate 위젯
- Boolean 멤버는 `b` 접두사: `bIsDead`, `bHasAuthority`
- 함수는 PascalCase, 동사로 시작: `ApplyDamage()`, `GetHealth()`

### Memory & Pointers (중요)
- **UObject 계열은 절대 `std::shared_ptr` / `std::unique_ptr`로 관리하지 말 것.** GC가 추적하지 못해 크래시 발생.
  - UObject 멤버 포인터는 raw pointer + `UPROPERTY()`로 선언해 GC가 추적하게 함
  - 약참조가 필요하면 `TWeakObjectPtr<>`, 에셋 비동기 로딩은 `TSoftObjectPtr<>` / `TObjectPtr<>` 사용
- 비-UObject(순수 C++ 객체)는 `TUniquePtr` / `TSharedPtr` 사용
- 객체 생성은 `NewObject<>()`, 컴포넌트는 생성자에서 `CreateDefaultSubobject<>()`. `new`/`delete` 직접 사용 금지.

### Containers / Strings
- `std::vector` → `TArray`, `std::map` → `TMap`, `std::set` → `TSet`
- `std::string` → `FString`(가변 텍스트) / `FName`(식별자·검색키) / `FText`(현지화 표시 텍스트)

### Headers
- Forward declaration 적극 사용, 헤더의 `#include` 최소화 (컴파일 시간 절감)
- `.cpp`에 필요한 include 몰기. `*.generated.h`는 항상 헤더의 **마지막** include여야 함

### Logging & Assertion
- `printf`/`std::cout` 금지 → `UE_LOG(LogTemp, Warning, TEXT("..."))` 사용 (전용 로그 카테고리 권장)
- 불변 조건은 `check()`, 복구 가능한 경고는 `ensure()` / `ensureMsgf()`

---

## 4. GAS (Gameplay Ability System) 규칙

이 프로젝트는 GAS를 핵심 프레임워크로 사용합니다. 신규 게임플레이 로직은 가급적 GAS 패턴으로 작성하세요.

### ASC 소유 구조 (본 프로젝트 — 반드시 인지) ⚠️ 중요
이 프로젝트는 **플레이어가 ASC를 두 곳에 보유하는 이중 구조**입니다. 코드 작성 시 어느 ASC/AttributeSet을 다루는지 반드시 먼저 확인할 것.

**① `AR1Character`(`ACharacter` 상속) — 캐릭터 베이스, 플레이어·몬스터 공통**
- ASC: `TObjectPtr<UR1AbilitySystemComponent> AbilitySystemComponent`
- AttributeSet: `TObjectPtr<UR1AttributeSet> CommonAttributeSet` (전투 단위 스탯: 체력 등)
- `IAbilitySystemInterface::GetAbilitySystemComponent()` 구현, 캐릭터 종류는 `CharacterRowName`으로 구분
- 스탯 초기화: `CharacterRowName`으로 `CharacterStatTable`(DataTable) 조회 → `InitStatEffectClass`(GE) 적용
- 초기화 흐름: `InitAbilitySystem()` → `InitAttributes()` → `AddCharacterAbility()` → `ApplyCharacterEffect()`

**② `AR1PlayerState`(`APlayerState` 상속) — 플레이어 전용**
- ASC: `TObjectPtr<UR1AbilitySystemComponent> AbilitySystemComponent` (캐릭터의 것과 **별개**)
- AttributeSet 2종:
  - `TObjectPtr<UR1AttributeSet> CoreAttributeSet` (캐릭터의 `CommonAttributeSet`과 **같은 타입이지만 다른 인스턴스** — PlayerState ASC에 등록된 별도 객체)
  - `TObjectPtr<UPlayerAttributeSet> PlayerAttributeSet`
- 다루는 영역(추정): 세션/진행 단위 영속 스탯 — 경험치(`OnExpChanged`, `GetCurrentExpRatio`), 런 레벨(`RunLevel`), 직업(`PlayerClass`, `ER1CharacterClass`), 메타 업그레이드(`ApplyMetaUpgrades`, `MetaUpgradeDataTable`, `MetaUpgradeEffectClass`)
- 메타 업그레이드 스탯: `MetaUpgradeDataTable`(`FR1MetaUpgradeData`) → `MetaUpgradeEffectClass`(GE) 경유 적용
- 접근자: `GetR1AbilitySystemComponent()`, `GetPlayerAttributeSet()`, `GetCommonAttributeSet()`, `GetRunUpgradeComponent()`
- 로그라이크 구조: `RunLevel`(이번 런 레벨), `UR1RunUpgradeComponent`(런 중 업그레이드), 메타 업그레이드(런 사이 영속 강화)

**ASC 접근 규칙 (반드시 준수)**
- **전투 스탯(체력 등) 또는 캐릭터 어빌리티** → ①(`AR1Character`)의 ASC/`CommonAttributeSet` 사용
- **경험치·런 레벨·직업·메타 업그레이드** → ②(`AR1PlayerState`)의 ASC/`CoreAttributeSet`/`PlayerAttributeSet` 사용
- 몬스터는 ①의 ASC에 `UR1AttributeSet` + `UMonsterAttributeSet`을 등록. ②(PlayerState)는 존재하지 않음.
- 두 ASC를 혼동해 엉뚱한 쪽에 GE를 적용하지 말 것.

### AttributeSet별 어트리뷰트 소속 (코드 확정 — 위치를 헷갈리지 말 것)
어떤 어트리뷰트가 어느 AttributeSet에 있는지 정확히 따를 것. 잘못된 Set에서 찾으면 컴파일은 되도 런타임에 값을 못 읽음.

**`UR1AttributeSet`** (공통/전투 — 플레이어·몬스터 공용, `AR1Character` ASC에 등록)
- 생존: `Health`, `MaxHealth`, `HealthRegeneration`, `MoveSpeed`
- 전투: `AttackRange`, `AttackRadius`, `AttackSpeed`, `BaseDamage`, `BaseDefence`, `CriticalHitChance`, `CriticalHitMultiplier`
- override: `PostGameplayEffectExecute`, `PreAttributeChange`, `PostAttributeChange` 구현됨 → 클램핑·파생 계산 로직이 여기 있으므로 임의 수정 금지

**`UPlayerAttributeSet`** (플레이어 전용 — `AR1PlayerState` ASC에 등록)
- 마나: `Mana`, `MaxMana`, `ManaRegeneration`
- 성장: `Exp`, `MaxExp`, `Level`
- 장비/배율: `WeaponDamage`, `DamageMultiplier`, `EquipDefence`, `DefenceMultiplier`
- 메타/로그라이크: `ExtraGold`, `Honor`, `Luck`, `Revive`
- override: `PostGameplayEffectExecute`, `PreAttributeChange` 구현됨

**`UMonsterAttributeSet`** (몬스터 전용 — `AR1Character` ASC에 등록, 몬스터만)
- `Xp`(처치 시 주는 경험치), `AggroRange`, `AttackAngle`
- 현재 override 함수들은 주석 처리 상태 (활성화 시 주석 해제 필요)

> 주의: `UR1AttributeSet`의 `BaseDamage`/`BaseDefence`와 `UPlayerAttributeSet`의 `WeaponDamage`/`EquipDefence`/`*Multiplier`는 **별개 어트리뷰트**다. 데미지/방어 계산식을 짤 때 둘을 혼동하지 말 것 (보통 최종값 = 공통 Base + 플레이어 장비/배율의 조합). 정확한 계산식이 불확실하면 `PostGameplayEffectExecute` 구현부를 확인할 것.

### 핵심 클래스 책임
- **AbilitySystemComponent (`UR1AbilitySystemComponent`)**: 어빌리티·이펙트·태그의 허브. 캐릭터·플레이어스테이트가 각각 별도 인스턴스 보유
- **GameplayAbility (`UGameplayAbility`)**: 행동 단위. `ActivateAbility` → `CommitAbility` → `EndAbility` 흐름 준수
- **AttributeSet**: `UR1AttributeSet`(공통/전투), `UPlayerAttributeSet`(플레이어 전용), `UMonsterAttributeSet`(몬스터 전용). 변경은 반드시 GameplayEffect를 통해서만 (직접 대입 금지)
- **GameplayEffect (GE)**: 스탯 변경/버프/디버프. Instant / Duration / Infinite 구분
- **GameplayTag**: 상태·분류 표현. 단, `ECreatureState` / `ER1CharacterClass` 등 기존 enum은 코드에 이미 존재하므로 임의로 GameplayTag로 갈아엎지 말 것

### 작성 규칙
- AttributeSet 값을 코드에서 직접 `Set` 하지 말 것. `ApplyGameplayEffectToSelf` 등 GE 경유. (단, 초기화/디버그 목적의 `Init*` setter는 예외적으로 허용되는 경우가 있으니 기존 패턴을 따를 것)
- 어빌리티 활성화는 직접 호출하지 말고 `TryActivateAbilityByTag` / `TryActivateAbilityByClass` 사용
- 새 어트리뷰트 추가 시 기존 패턴 유지: `FGameplayAttributeData` 멤버 + `ATTRIBUTE_ACCESSORS(ThisClass, Name)` 매크로 + 올바른 AttributeSet에 배치
- 어빌리티 종료 시 `EndAbility` 호출 누락 금지 (리소스/태그 누수)
- ASC/AttributeSet 접근 시 캐릭터(①)인지 플레이어스테이트(②)인지, 그리고 어느 AttributeSet 소속인지 먼저 확정할 것

---

## 5. 멀티플레이 / 리플리케이션 처리 정책 ⚠️

이 프로젝트는 **싱글플레이 전용**입니다. 다만 GAS는 본질적으로 네트워크를 전제로 설계된 프레임워크이므로, "보이는 멀티플레이 코드를 무조건 삭제"하면 GAS 내부 동작이 깨집니다. 아래 기준을 **반드시** 지키세요.

### 신규 코드 작성 시
- 새 게임플레이 코드에 네트워크 분기/리플리케이션 로직을 **새로 추가하지 말 것**
- `if (HasAuthority())` / `if (IsLocallyControlled())` 같은 권한 분기를 새로 만들지 말 것 (싱글플레이는 항상 authority)
- 새 RPC(`UFUNCTION(Server/Client/NetMulticast)`) 작성 금지

### 기존 멀티플레이 코드 정리 시 — 안전 우선
멀티플레이 코드가 보이면 **로직이 손상되지 않는 선에서만** 단순화/제거하세요. 다음은 안전하게 제거 가능:
- 순수하게 클라이언트-서버 동기화만을 위한 빈/중복 RPC 래퍼 (실제 로직이 없는 경우)
- 싱글플레이에서 항상 같은 분기로만 흐르는 `HasAuthority()` 가드 → 가드 제거 후 본문만 남기기 (단, 본문은 보존)
- 클라이언트 예측(prediction) 전용 보정 코드 중 게임 결과에 영향 없는 부분

### 절대 건드리지 말 것 (GAS 무결성 보존)
다음을 제거하면 GAS가 깨지므로 **그대로 둘 것**:
- `UAttributeSet`의 `GetLifetimeReplicatedProps()` 및 어트리뷰트의 `ReplicatedUsing`(OnRep) 구조
- `UGameplayAbility`의 `NetExecutionPolicy` / `NetSecurityPolicy` 설정
- `FGameplayAbilityActorInfo`, `FActiveGameplayEffectHandle` 등 GAS 내부 핸들 흐름
- ASC의 prediction key 관련 내부 호출

### 판단이 애매하면
**삭제하지 말고 그대로 둔 채, 해당 부분을 주석으로 표시하고 사용자에게 질문할 것.**
"동작이 바뀔 수 있다"고 판단되는 변경은 임의로 진행하지 않는다.

---

## 6. Architecture & Directory

```
Source/
  <ProjectName>/
    Public/      // 외부 모듈에 노출하는 헤더
    Private/     // 구현부
Content/         // 에셋 (직접 편집 금지, 에디터에서 조작)
Config/          // DefaultEngine.ini 등 설정
```

### 게임플레이 프레임워크 책임 분담
- `GameMode`: 규칙·승패·스폰 (싱글플레이라 서버 개념과 동일시)
- `PlayerController`: 입력 수신, UI 제어
- `AR1Character`(`ACharacter` 상속): **캐릭터 베이스. ASC·`UR1AttributeSet`(전투 스탯) 직접 소유**, `IAbilitySystemInterface` 구현. 플레이어·몬스터 모두 상속, 종류는 `CharacterRowName`으로 구분
- `AR1PlayerState`(`APlayerState` 상속): **플레이어 전용. 별도 ASC + `UR1AttributeSet`(Core) + `UPlayerAttributeSet`(진행 스탯: 경험치·런레벨·직업·메타업그레이드) 소유**, `UR1RunUpgradeComponent` 보유 (로그라이크 런/메타 진행)
- `Subsystem`: 전역 매니저는 `UGameInstanceSubsystem` / `UWorldSubsystem` 활용 (싱글톤 직접 구현 지양)

> ⚠️ 플레이어는 캐릭터(전투 스탯)와 PlayerState(진행 스탯)에 **ASC를 이중으로 보유**. 4번 ASC 소유 구조 반드시 참고.

### Asset 명명 규칙 (본 프로젝트에 이미 적용됨 — 반드시 준수)
이 프로젝트는 아래 명명 규칙이 이미 적용된 상태입니다. 신규 에셋 생성·참조 시 **반드시 이 규칙을 따를 것.**

| 접두사 | 에셋 타입 |
|--------|-----------|
| `GA_`  | GameplayAbility |
| `GE_`  | GameplayEffect |
| `DT_`  | DataTable |
| `PDA`  | PrimaryAssetData |
| `DA_`  | DataAsset (일반 데이터 에셋) |
| `WBP_` | Widget Blueprint |
| `BP_`  | Blueprint Class |
| `ABP_` | Animation Blueprint Class |

- 기존 에셋의 명명 규칙을 임의로 바꾸지 말 것. 위 표가 프로젝트의 기준이다.
- 표에 없는 타입의 접두사가 필요하면 임의로 정하지 말고 사용자에게 확인할 것.

---

## 7. Do Not (자주 발생하는 실수)

- `*.generated.h`, `Binaries/`, `Intermediate/`, `Saved/`, `DerivedDataCache/` 내 파일을 직접 편집·생성하지 말 것
- `Content/` 의 `.uasset` / `.umap`을 텍스트로 편집하려 하지 말 것 (바이너리)
- UObject 멤버를 `UPROPERTY()` 없이 raw 포인터로 보유하지 말 것 (dangling/GC 문제)
- AttributeSet 값을 GameplayEffect 없이 직접 대입하지 말 것
- 게임 스레드 외부(워커 스레드)에서 UObject/AActor에 직접 접근하지 말 것
- 싱글플레이 프로젝트에 새 리플리케이션/RPC 로직을 추가하지 말 것 (5번 정책 참고)
- 헤더에 불필요한 `#include`를 추가하지 말 것 (forward declaration 우선)

---

## 8. 작업 흐름 (Workflow)

1. 헤더(`UCLASS`/`UPROPERTY`)를 수정했다면 Live Coding이 아니라 VS2022 풀 빌드가 필요함을 인지
2. 새 게임플레이 기능은 GAS 패턴(Ability/Effect/Attribute/Tag)을 우선 검토
3. 멀티플레이 관련 코드를 만나면 5번 정책에 따라 처리하고, 애매하면 질문
4. 변경 후 컴파일 가능 여부를 우선 고려하고, 빌드 명령은 2번 섹션 참고

---

## 9. References

### 공식 문서 (Epic / 검증됨)
- Epic C++ Coding Standard: https://dev.epicgames.com/documentation/en-us/unreal-engine/epic-cplusplus-coding-standard-for-unreal-engine
  - 특정 버전은 URL 끝에 `?application_version=5.3` 추가
  - 코딩 표준 준수는 Epic이 "필수(mandatory)"로 명시. 다수 규칙은 크로스 컴파일러 호환성 때문에 요구됨
- Programming with C++ in Unreal Engine: https://dev.epicgames.com/documentation/en-us/unreal-engine/programming-with-cpp-in-unreal-engine
  - `UCLASS()`/`UFUNCTION()`/`UPROPERTY()` 매크로로 리플렉션·GC 등록 (3번 메모리 규칙의 근거)
- Gameplay Ability System: https://dev.epicgames.com/documentation/en-us/unreal-engine/gameplay-ability-system-for-unreal-engine
- Gameplay Effects (GAS): https://dev.epicgames.com/documentation/en-us/unreal-engine/gameplay-effects-for-the-gameplay-ability-system-in-unreal-engine
  - "Gameplay Effect는 GAS가 어트리뷰트를 변경하는 방식" → AttributeSet 직접 대입 금지(4·7번)의 근거
  - AttributeSet은 `UAttributeSet` 상속 + `UPROPERTY` 태그된 GameplayAttributeData 멤버로 구성
- C++ API Reference: https://dev.epicgames.com/documentation/unreal-engine/API

### 커뮤니티 관례 (비공식)
- 에셋 명명 규칙은 Epic 공식 표준이 아니라 커뮤니티 관례 기반이지만, **본 프로젝트에서는 6번 섹션의 표가 이미 확정 적용된 규칙**이므로 그대로 준수할 것.
- 빌드 배치 인자(`-waitmutex`, `-rocket` 등)와 Live Coding 헤더 제약은 일반적으로 통용되나 환경에 따라 다를 수 있으니, 의심되면 공식 문서/실제 동작으로 재확인할 것.

> 규칙의 근거가 의심스러우면 위 공식 문서를 우선 신뢰하고, 버전을 5.3으로 맞춰 확인할 것.
