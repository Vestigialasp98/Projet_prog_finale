// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerStatType.generated.h"

UENUM(BlueprintType)
enum class EPlayerStatType : uint8
{
	// --- STATS DU PERSONNAGE ---
	Health		    UMETA(DisplayName = "Max Health"),
	Speed			UMETA(DisplayName = "Movement Speed"),
	HealthRegen		UMETA(DisplayName = "Health Regeneration"),
	PickUpRange		UMETA(DisplayName = "Pick Up Range"),

	// --- STATS DE L'ARME ---
	WeaponDamage	UMETA(DisplayName = "Weapon Damage"),
	WeaponCooldown	UMETA(DisplayName = "Weapon Cooldown Speed"),
	WeaponArea      UMETA(DisplayName = "Weapon Area/Size"),
	GlobalDamage	UMETA(DisplayName = "Global Damage Multiplier")
};