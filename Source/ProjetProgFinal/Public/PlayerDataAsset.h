// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PlayerDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class PROJETPROGFINAL_API UPlayerDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Cooldown = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Range;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DMG;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector HitboxScale = FVector(1.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float HitboxDuration = 0.15f;


};
