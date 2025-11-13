// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerStatType.h"
#include "Engine/DataTable.h"
#include "LevelUpData.generated.h"

USTRUCT(BlueprintType)
struct FLevelUpData
{
	GENERATED_BODY()

public:
	UPROPERTY (EditAnywhere, BlueprintReadOnly, Category = "Level Data")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level Data")
	TMap <EPlayerStatType, float> StatsToApply;
};