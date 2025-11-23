// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "BossEnemy.h"
#include "SurvivalGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimerUpdated, float, NewTotalSeconds);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnKillsUpdated, int32, NewKillCount);

/**
 * 
 */
UCLASS()
class PROJETPROGFINAL_API ASurvivalGameState : public AGameState
{
	GENERATED_BODY()

public:

	ASurvivalGameState();

protected:
	float TimeSinceLastUIUpdate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Stats")
	int32 EnemiesKilled;

	virtual void Tick(float DeltaSeconds) override;

	// Events dispatch qui peuvent etre appelés en blueprint pour le UI
	UPROPERTY(BlueprintAssignable, Category = "Events UI")
	FOnTimerUpdated OnTimerUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Events UI")
	FOnKillsUpdated OnKillsUpdated;

	UFUNCTION(BlueprintImplementableEvent)
	void OnVictory(bool IsBossDead);

	// Stats du boss:

	// Classe a apparaitre
	UPROPERTY(EditDefaultsOnly, Category = "Boss Config")
	TSubclassOf<ABossEnemy> BossClass;

	// Le temps d'apparition en secondes 
	UPROPERTY(EditDefaultsOnly, Category = "Boss Config")
	float BossSpawnTime = 300.0f; // 5 mins

	// Distance devant le joueur et le boss
	UPROPERTY(EditDefaultsOnly, Category = "Boss Config")
	float BossSpawnDistance = 1200.0f;

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Stats")
	float TotalTimeElapsed;

	// Fonction pour augmenter le nombre de kills (avec le pooling)
	UFUNCTION(BlueprintCallable, Category = "Game Stats")
	void IncrementKillCount();

	UFUNCTION(BlueprintCallable)
	void TriggerVictory(bool GameStatus);

private:
	bool bBossHasSpawned = false;
};
