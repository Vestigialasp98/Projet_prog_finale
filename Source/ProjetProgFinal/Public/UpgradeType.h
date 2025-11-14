// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UpgradeType.generated.h"

UENUM(BlueprintType)
enum class EUpgradeType : uint8
{
	Combat		UMETA(DisplayName = "Combat"),
	Character		UMETA(DisplayName = "Character"),
	Miscellaneous		UMETA(DisplayName = "Miscellaneous"),
};
