# Project R1: 게임 설정(Options) 시스템 요구사항 및 플로우 명세서

## 1. 설정 데이터 아키텍처 (Backend Spec)

데이터는 언리얼 엔진이 OS 단에서 관리하는 네이티브 그래픽 설정과 게임 플레이에 특화된 커스텀 세이브 설정 두 가지로 완벽하게 분리하여 관리합니다.

### A. 시스템 그래픽 설정 (UGameUserSettings 활용)
엔진 내장 클래스를 사용하여 ini 파일(Saved/Config/WindowsEditor/GameUserSettings.ini 또는 Saved/Config/Windows/GameUserSettings.ini)에 자동 저장 및 로드되도록 처리합니다.

* **해상도 (Resolution):** `SetScreenResolution()` / 데이터 타입: `FIntPoint` / 기본값: `1920 x 1080` (표준 16:9 해상도 목록 필터링 적용)
* **화면 모드 (Window Mode):** `SetFullscreenMode()` / 데이터 타입: `EWindowMode` / 기본값: `WindowedFullscreen` (테두리 없는 창)
* **프레임 제한 (FPS Limit):** `SetFrameRateLimit()` / 데이터 타입: `float` / 기본값: `60.0f` (또는 0.0f = 무제한)
* **수직 동기화 (V-Sync):** `SetVSyncEnabled()` / 데이터 타입: `bool` / 기본값: `true`

### B. 게임 고유 설정 (UR1SaveGame_Settings 커스텀 세이브 클래스)
`USaveGame`을 상속받은 커스텀 클래스를 만들어 `Saved/SaveGames/Settings.sav` 파일로 직렬화(Serialization)하여 관리합니다.

* **사운드 - 마스터 볼륨 (MasterVolume):** 데이터 타입: `float` / 범위: `0.0 ~ 1.0` / 기본값: `1.0`
* **사운드 - BGM 볼륨 (BGMVolume):** 데이터 타입: `float` / 범위: `0.0 ~ 1.0` / 기본값: `1.0`
* **사운드 - SFX 볼륨 (SFXVolume):** 데이터 타입: `float` / 범위: `0.0 ~ 1.0` / 기본값: `1.0`
* **게임플레이 - 데미지 텍스트 표시 (bShowDamageText):** 데이터 타입: `bool` / 기본값: `true`
* **게임플레이 - 미니맵 투명도 (MinimapOpacity):** 데이터 타입: `float` / 범위: `0.1 ~ 1.0` / 기본값: `0.5`
* **조작/접근성 - 마우스 커서 가두기 (bConfineMouseToWindow):** 데이터 타입: `bool` / 기본값: `true`
* **조작/접근성 - 화면 흔들림 강도 (CameraShakeIntensity):** 데이터 타입: `float` / 범위: `0.0 ~ 1.0` / 기본값: `1.0`

---

## 2. UMG UI 레이아웃 설계 (Frontend Spec)

UI는 유지보수가 쉽도록 단일 메인 위젯(WBP_OptionsMenu) 안에 Widget Switcher를 두고, 각 탭의 화면은 C++ 클래스를 상속받은 개별 모듈 위젯(User Widget)으로 분리하여 조립하는 아키텍처를 채택합니다.

### A. 전체 화면 계층 구조 (Hierarchy)
* [Canvas Panel] (Root)
    * [Background Blur] (Strength: 5.0, 화면 전체 적용)
    * [Border] (메인 메뉴 판넬, 가운데 정렬)
        * [Vertical Box] (메뉴 항목 세로 정렬)
            * [Horizontal Box] (상단 탭 버튼 컨테이너)
                * Button_Tab_Graphics (그래픽)
                * Button_Tab_Audio (사운드)
                * Button_Tab_Gameplay (게임플레이)
                * Button_Tab_Controls (조작)
            * [Widget Switcher] (이름: ContentSwitcher)
                * WBP_Category_Graphics (Index 0) -> ComboBox (String) 활용
                * WBP_Category_Audio (Index 1) -> WBP_SettingRow_Slider 모듈 활용
                * WBP_Category_Gameplay (Index 2)
                * WBP_Category_Controls (Index 3)
            * [Horizontal Box] (하단 컨트롤 버튼 컨테이너)
                * Button_Defaults (기본값)
                * Button_Apply (적용)
                * Button_Cancel (취소)
                * Button_OK (확인)

---

## 3. 하단 공통 컨트롤 (Bottom Actions) 스펙

모든 조작은 C++의 Temp 변수(임시 상태)와 Settings 객체(영구 상태) 간의 상호작용으로 이루어집니다.

| 버튼명 (UI 표기) | 기능 요약 | 디스크 저장 | 창 닫기 (OnClose) | 상세 로직 |
| :--- | :--- | :--- | :--- | :--- |
| **기본값 (Defaults)** | 초기화 | 저장 안 함 | 닫지 않음 | 1. 모든 Temp 변수에 기획된 하드코딩 기본값을 덮어씌움.<br>2. SyncUIFromSettings() 형태의 함수를 호출하여 UI 컨트롤(슬라이더, 콤보박스 등) 위치를 기본값으로 갱신. |
| **적용 (Apply)** | 중간 저장 | 저장함 | 닫지 않음 | 1. ApplyAndSaveSettings() 호출 (Temp -> Settings 복사 및 .ini/.sav 저장).<br>2. "설정이 적용되었습니다" 시각/청각적 피드백 제공. |
| **취소 (Cancel)** | 변경 무시 | 저장 안 함 | 닫음 | 1. 미저장 변경사항 검사(하단 4항 참조).<br>2. 변경사항 무시 후 OnCloseRequested 디스패처 호출. |
| **확인 (OK)** | 최종 저장 및 닫기| 저장함 | 닫음 | 1. ApplyAndSaveSettings() 호출.<br>2. 저장이 완료되면 즉시 OnCloseRequested 디스패처 호출. |

---

## 4. 미저장 변경사항 (Unsaved Changes) 안전 로직

유저가 실수로 취소(Cancel) 버튼을 누르거나 ESC 키를 눌러 창을 닫으려 할 때, 데이터 유실을 방지하기 위한 검증 파이프라인입니다.

1. **상태 비교:** 닫기 요청 발생 시, 현재 메모리의 Temp 변수 세트와 파일과 동기화된 Settings 변수 세트를 비교합니다.
2. **분기 처리:**
    * **일치함 (변경 없음):** 즉시 OnCloseRequested 호출 (창 닫기 진행).
    * **불일치 (변경 있음):** 경고 모달(Modal) 팝업 호출.
3. **경고 모달 팝업 구성:**
    * **텍스트:** "저장되지 않은 변경사항이 있습니다. 적용하시겠습니까?"
    * **[예]:** Apply 수행 후 창 닫기.
    * **[아니오]:** 변경사항 파기(Temp 무시) 후 창 닫기.
    * **[취소]:** 모달만 닫고 옵션 메뉴 화면으로 복귀.

---

## 5. 컨텍스트 기반 화면 전환 플로우 (Wrapper Pattern)

WBP_OptionsMenu는 독립적인 모듈로서 스스로를 파괴(Remove From Parent)하지 않습니다. 대신 OnCloseRequested 이벤트 디스패처를 호출하며, 이를 호출한 부모 래퍼(Wrapper) 위젯이 상황(인게임/아웃게임)에 맞춰 화면을 제어합니다.

### A. 아웃게임 흐름 (타이틀 메인 메뉴)
* **주관 클래스:** WBP_MainMenu
* **옵션 진입 흐름:**
    1. WBP_MainMenu 내부의 UI 요소(버튼, 로고 등)를 숨김 (Hidden).
    2. WBP_OptionsMenu 위젯을 생성(Create Widget)하고 뷰포트에 추가.
    3. 옵션 위젯의 OnCloseRequested 이벤트에 복귀 로직(Bind) 연결.
* **복귀 로직 (OnCloseRequested 수신 시):**
    1. WBP_OptionsMenu를 파괴 (Remove From Parent).
    2. 숨겨두었던 WBP_MainMenu 내부 UI 요소를 다시 표시 (Visible).

### B. 인게임 흐름 (일시정지 메뉴)
* **주관 클래스:** WBP_PauseMenu
* **옵션 진입 흐름:**
    1. 유저가 게임 중 ESC 입력 -> 게임 시간 정지 (Set Game Paused = True).
    2. WBP_PauseMenu 생성 및 표시.
    3. 유저가 '옵션' 클릭 -> WBP_PauseMenu 패널 숨김 (Hidden).
    4. WBP_OptionsMenu 위젯을 생성하고 뷰포트에 추가, OnCloseRequested 바인딩.
* **복귀 로직 (OnCloseRequested 수신 시):**
    1. WBP_OptionsMenu를 파괴 (Remove From Parent).
    2. 숨겨두었던 WBP_PauseMenu 패널을 다시 표시 (Visible).
    3. **주의사항:** 이때 게임의 일시정지 상태(Set Game Paused = True)는 절대 해제하지 않음. (유저가 일시정지 메뉴에서 '게임 계속하기'를 직접 누를 때만 해제).
