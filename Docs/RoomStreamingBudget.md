# UE5.3 룸 스트리밍 운영 예산안 (Diablo2 스타일)

## 고정 정책
- 선로딩 상한: `현재 + 인접 3개` (총 4개)
- 하이브리드 전환:
  - 큰 전환(런/층 시작): 로딩 화면 또는 대량 Async + 완료 대기
  - 잦은 전환(문/룸 이동): Async 선로딩 기본
  - 예외: 선로딩 실패 시 1초 미만 페이드/문 잠금 연출

## 저예산(Runtime Budget) 제안값
- `MaxPreloadedRooms = 4`
- `MaxAliveMonstersPerRoom = 32`
- `MaxProjectilesPerRoom = 48`
- `MaxNiagaraSystemsPerRoom = 24`
- `MaxConcurrentSfxVoices = 20`
- `UnloadGraceSeconds = 8.0`

> 초반 안정성 기준으로 낮게 시작하고, Unreal Insights/Stat Unit/Stat Niagara로 측정 후 점진 상향.

## 데이터 구조(기존 Data 폴더와 결합)
- `UR1RoomDefinitionData` (PrimaryDataAsset)
  - RoomLevel(.umap soft reference)
  - PreloadPrimaryAssets(FPrimaryAssetId 배열)
  - PreloadAssetLabels(FName 배열, 기존 AssetData 라벨 체계 보조)
  - SpawnBatchSizePerFrame(룸 진입 후 스폰 분산 크기)

## 서브시스템 뼈대
- `UR1RoomStreamingSubsystem` (GameInstanceSubsystem)
  - 룸 상태: `Cold / Preloading / Warm / Hot`
  - API:
    - `QueuePreloadRooms` : 다음 후보 룸 2~4개 preloading
    - `MarkRoomGameplayReady` : 스폰 분산 완료 후 Hot 전환
    - `CanOpenDoorImmediately` : 문 즉시 개방 가능 여부
    - `TickRoomCachePolicy` : grace time + 상한 초과 정리

## 적용 순서
1. 룸 생성/랜덤 결과를 `UR1RoomDefinitionData`로 확정
2. 플레이어 이동 예측 시 `QueuePreloadRooms` 호출
3. 문 상호작용 시
   - Hot: 즉시 오픈
   - Warm/Preloading: 짧은 연출 후 진입
   - Cold: 최후 fallback 처리
4. 룸 언로드는 `TickRoomCachePolicy`에서 grace 기반 수행

## 참고
- 기존 `UR1AssetData`/`UR1AssetManager` 파이프라인은 유지.
- 룸별 컨텐츠는 PrimaryAsset 단위로 묶고, 공통 UI/기본 FX/SFX는 기존 Preload 라벨로 상시 유지.
