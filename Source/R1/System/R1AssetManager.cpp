// Fill out your copyright notice in the Description page of Project Settings.


#include "System/R1AssetManager.h"

UR1AssetManager::UR1AssetManager()
{

}

UR1AssetManager& UR1AssetManager::Get()
{
	if (UR1AssetManager* Singleton = Cast<UR1AssetManager>(GEngine->AssetManager))
	{
		return *Singleton;
	}
	UE_LOG(LogTemp, Fatal, TEXT("Can't find UR1AssetManager"));

	return *NewObject<UR1AssetManager>();
}