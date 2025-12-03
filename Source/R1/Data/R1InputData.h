// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "R1InputData.generated.h"

class UInputAction;
class UInputMappingContext;

USTRUCT()
struct FR1InputAction
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag InputTag = FGameplayTag::EmptyTag;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UInputAction> InputAction = nullptr;

};

/**
 * 
 */
UCLASS()
class R1_API UR1InputData : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UInputMappingContext> InputMappingContext;

	UPROPERTY(EditDefaultsOnly) //editor에서 설정을 하려면
	TArray<FR1InputAction> InputActions;
};
