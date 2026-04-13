# [PR] Meta Progression 시스템 초기화 및 맵 설정 오류 수정 (2026-04-10)

## 📝 개요
본 PR은 메타 프로그레션(Meta Progression) 시스템의 레벨 및 최대 경험치 초기화 로직을 수정하고, 맵 로드 및 설정 과정에서 발생하던 오류를 해결하는 것을 목적으로 합니다.

## 🚀 주요 변경 사항

### 1. Meta Progression 시스템 안정화
*   **Level 및 Max Exp 초기화 로직 수정**: `AR1PlayerState::ApplyMetaUpgrades`에서 세이브 데이터 로드 시 레벨과 경험치가 올바르게 반영되지 않던 문제를 수정했습니다.
    *   `PlayerStatTable`의 커브 데이터를 활용하여 레벨에 따른 `MaxExp`를 동적으로 계산하도록 변경했습니다.
    *   경험치 변경 시 UI 갱신을 위한 `OnExpChanged` 브로드캐스트 로직을 추가했습니다.
*   **SaveSystem 연동 강화**: `UR1SaveSystem`에서 런 데이터 종료 시 메타 데이터를 추출하고 저장하는 `ExtractAndSaveMetaProgression` 로직을 보완했습니다.
*   **Gameplay Effect 적용**: 메타 업그레이드 수치를 `GameplayEffect`를 통해 ASC(Ability System Component)에 올바르게 전달하도록 `SetSetByCallerMagnitude`를 적용했습니다.

### 2. Map 및 레벨 디자인 수정
*   **Map Setting Error 해결**: 맵 로드 시 플레이어 위치나 스폰 관련 설정 오류를 수정했습니다. (`0b87bdee`)
*   **R1Door 및 DungeonManager 개선**: 구역 이동 간의 안정성을 위해 `R1Door`의 로직을 보강하고, `DungeonManager`를 통해 맵 생성 및 관리를 최적화했습니다.
*   **Asset 및 Map 업데이트**: `1F_B`, `1F_M`, `1F_R` 등 주요 맵 파일의 바이너리 업데이트 및 관련 블루프린트(`BP_PlayerState`, `BP_R1Player`, `BP_ItemActor`) 설정을 변경했습니다.

### 3. 코드 리팩토링 및 기타
*   `AR1PlayerController`에서 초기화 시점 및 맵 설정 관련 로직을 정교화했습니다.
*   `R1MetaSaveGame` 구조에 필요한 필드를 추가하여 데이터 영속성을 보장했습니다.

## ✅ 테스트 및 검증 결과
*   **메타 업그레이드 검증**: 업그레이드 적용 후 게임 재시작 시 플레이어 레벨 및 보너스 스탯이 정상적으로 유지됨을 확인했습니다.
*   **맵 로드 안정성**: 맵 전환 및 런 데이터 재시작 시 발생하던 위치 오류 및 세팅 누락 현상이 해결되었습니다.

---
**관련 커밋:**
- `6ef7643` Fix : Meta progression Level 및 Max Exp 초기화
- `0b87bde` Fix : Map Setting Error
