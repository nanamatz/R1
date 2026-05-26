#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "R1PlayerSpawnMarker.generated.h"

/**
 * Lightweight editor marker. Place one in a start room sublevel to designate
 * where the player character spawns when beginning a new run or a new floor.
 * AR1MapGenerator reads this actor's world transform in RegisterRoomManager().
 * No runtime logic — no tick, no collision.
 */
UCLASS(Blueprintable)
class R1_API AR1PlayerSpawnMarker : public AActor
{
	GENERATED_BODY()

public:
	AR1PlayerSpawnMarker();

protected:
	/** Shows player facing direction in the editor viewport. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawn Marker")
	TObjectPtr<class UArrowComponent> ArrowComponent;

	/** Editor-only sprite for easy selection in the viewport. Hidden in-game. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawn Marker")
	TObjectPtr<class UBillboardComponent> BillboardComponent;
};
