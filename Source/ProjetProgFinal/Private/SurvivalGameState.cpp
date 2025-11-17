// Fill out your copyright notice in the Description page of Project Settings.


#include "SurvivalGameState.h"
#include "Kismet/GameplayStatics.h"

ASurvivalGameState::ASurvivalGameState()
{
	PrimaryActorTick.bCanEverTick = true;

	TotalTimeElapsed = 0.0f;
	TimeSinceLastUIUpdate = 0.0f;
	EnemiesKilled = 0;
}

void ASurvivalGameState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (UGameplayStatics::IsGamePaused(GetWorld()))
	{
		return;
	}

	TotalTimeElapsed += DeltaSeconds;
	TimeSinceLastUIUpdate += DeltaSeconds;

	if (TimeSinceLastUIUpdate >= 1.0f)
	{
		TimeSinceLastUIUpdate -= 1.0f;
		
		OnTimerUpdated.Broadcast(TotalTimeElapsed);
	}
}

void ASurvivalGameState::IncrementKillCount()
{
	EnemiesKilled++;

	OnKillsUpdated.Broadcast(EnemiesKilled);
}
