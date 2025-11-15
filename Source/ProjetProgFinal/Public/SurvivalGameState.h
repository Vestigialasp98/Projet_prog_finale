// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Stats")
	float TotalTimeElapsed;
	
	float TimeSinceLastUIUpdate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Stats")
	int32 EnemiesKilled;

	virtual void Tick(float DeltaSeconds) override;

	// Events dispatch qui peuvent etre appelés en blueprint pour le UI
	UPROPERTY(BlueprintAssignable, Category = "Events UI")
	FOnTimerUpdated OnTimerUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Events UI")
	FOnKillsUpdated OnKillsUpdated;

public:

	// Fonction pour augmenter le nombre de kills (avec le pooling)
	UFUNCTION(BlueprintCallable, Category = "Game Stats")
	void IncrementKillCount();
};
