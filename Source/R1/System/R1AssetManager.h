// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "R1AssetManager.generated.h"

/**
 * 
 */
UCLASS()
class R1_API UR1AssetManager : public UAssetManager
{
	GENERATED_BODY()
public:
	UR1AssetManager();

	static UR1AssetManager& Get();

	//TODO
	// 실제로 로드한 모든 오브젝트들을 다 들고 있는 작업을 여기서 한다.
};
