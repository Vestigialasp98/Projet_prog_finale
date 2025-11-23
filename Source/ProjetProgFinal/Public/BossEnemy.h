// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyBase.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BossEnemy.generated.h"

/**
 * 
 */
UCLASS()
class PROJETPROGFINAL_API ABossEnemy : public AEnemyBase
{
	GENERATED_BODY()

public:
	ABossEnemy();

	UPROPERTY(EditAnywhere, Category = "AI")
	UBehaviorTree* BossBehaviorTree;

	UPROPERTY(EditAnywhere, Category = "Stats")
	float BossDamage = 50.f;

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Boss")
	void PlayDeathSequence();

	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void UpdateBossHealthUI();

	void FreezeAllEnemies();

	virtual void Die() override;
	
};
