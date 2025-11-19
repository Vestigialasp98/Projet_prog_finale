// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponBase.h"
#include "AttackBox.h"
#include "NiagaraSystem.h"
#include "SwipeWeapon.generated.h"

/**
 * 
 */
UCLASS()
class PROJETPROGFINAL_API ASwipeWeapon : public AWeaponBase
{
	GENERATED_BODY()
	
protected:
	virtual void Attack() override;

public:

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TSubclassOf<AAttackBox> AttackBoxClass;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	UNiagaraSystem* SlashVFX;
};
