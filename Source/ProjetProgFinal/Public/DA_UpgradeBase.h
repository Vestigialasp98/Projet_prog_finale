// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LevelUpData.h"
#include "UpgradeType.h"
#include "PlayerDataAsset.h"
#include "DA_UpgradeBase.generated.h"

/**
 * 
 */
UCLASS()
class PROJETPROGFINAL_API UDA_UpgradeBase : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Upgrade Data")
	FText DisplayName;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Upgrade Data")
	UTexture2D* Icon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Upgrade Data")
	UTexture2D* CardBackground;

	// Type d'upgrades
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Upgrade Data")
	EUpgradeType UpgradeType;

	// Le niveau max de l'upgrade
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Upgrade Data")
	int32 MaxLevel;

	// L'array des détails de chaque niveau (Niv 1 = Index 0 ; Niv 5 = Index 4)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Upgrade Data")
	TArray<FLevelUpData> LevelDetails;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Upgrade Target")
	UPlayerDataAsset* WeaponToUpgrade;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Upgrade Target")
	TSubclassOf<class AWeaponBase> WeaponClassToSpawn;
};
