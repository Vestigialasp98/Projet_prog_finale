// Fill out your copyright notice in the Description page of Project Settings.


#include "MyEnemyArchetype.h"

FPrimaryAssetId UMyEnemyArchetype::GetPrimaryAssetId() const
{
	// Type logique pour ce genre d’asset
	static const FPrimaryAssetType AssetType = TEXT("EnemyArchetype");
	return FPrimaryAssetId(AssetType, GetFName());
}

#if WITH_EDITOR
void UMyEnemyArchetype::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Sanity check simple : prévenir si la classe n’est pas définie
	if (!EnemyClass)
	{
		// UE_LOG(LogTemp, Warning, TEXT("UMyEnemyArchetype '%s' has no EnemyClass set."), *GetName());
	}
}
#endif