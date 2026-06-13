#pragma once

#include "CoreMinimal.h"

// ER1DoorDirection은 R1MapGenerator.h의 UENUM. 무거운 액터 헤더를 끌어오지 않도록
// 동일한 underlying type(uint8)으로 전방 선언만 한다. (.cpp에서 실제 정의를 include)
enum class ER1DoorDirection : uint8;

// 맵 생성 알고리즘의 순수(부수효과·RNG 없음) 격자/방향 연산 모음.
// AR1MapGenerator God Class에서 떼어낸 첫 조각으로, 향후 FR1MapLayoutGenerator의 토대가 된다.
// 순수 함수라 단위 테스트가 가능하다.
namespace R1MapGrid
{
	// 문 방향 → 가상 2D 그리드 오프셋. (North=+X, South=-X, East=+Y, West=-Y, None=Zero)
	// 기존 GenerateMap / GetConnectedNodeInDirection의 switch와 동일한 매핑.
	FIntPoint GetGridOffset(ER1DoorDirection Dir);

	// 문 방향 → 반대 방향. (None은 None)
	ER1DoorDirection GetOppositeDirection(ER1DoorDirection Dir);
}
