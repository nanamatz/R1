#include "Map/R1MapGrid.h"
#include "Map/R1MapGenerator.h" // ER1DoorDirection 실제 정의

namespace R1MapGrid
{
	FIntPoint GetGridOffset(ER1DoorDirection Dir)
	{
		switch (Dir)
		{
		case ER1DoorDirection::North: return FIntPoint(1, 0);  // 북쪽은 +X
		case ER1DoorDirection::South: return FIntPoint(-1, 0); // 남쪽은 -X
		case ER1DoorDirection::East:  return FIntPoint(0, 1);  // 동쪽은 +Y
		case ER1DoorDirection::West:  return FIntPoint(0, -1); // 서쪽은 -Y
		default:                      return FIntPoint::ZeroValue;
		}
	}

	ER1DoorDirection GetOppositeDirection(ER1DoorDirection Dir)
	{
		switch (Dir)
		{
		case ER1DoorDirection::North: return ER1DoorDirection::South;
		case ER1DoorDirection::South: return ER1DoorDirection::North;
		case ER1DoorDirection::East:  return ER1DoorDirection::West;
		case ER1DoorDirection::West:  return ER1DoorDirection::East;
		default:                      return ER1DoorDirection::None;
		}
	}
}
