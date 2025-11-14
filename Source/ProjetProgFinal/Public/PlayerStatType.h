// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerStatType.generated.h"

UENUM(BlueprintType)
enum class EPlayerStatType : uint8
{
	Health		UMETA(DisplayName = "Health"),
	Speed		UMETA(DisplayName = "Speed"),
	Damage		UMETA(DisplayName = "Damage"),
	Cooldown	UMETA(DisplayName = "Cooldown")
};