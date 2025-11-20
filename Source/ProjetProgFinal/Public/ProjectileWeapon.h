// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponBase.h"
#include "Projectile.h"
#include "ProjectileWeapon.generated.h"

/**
 * 
 */
UCLASS()
class PROJETPROGFINAL_API AProjectileWeapon : public AWeaponBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TSubclassOf<AProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	USoundBase* AttackSound;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	float ProjectileSpeed = 800.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	float AutoAimRadius = 1500.0f;

protected:
	virtual void Attack() override;

	AActor* FindClosestEnemy(const FVector& Origin);
};
