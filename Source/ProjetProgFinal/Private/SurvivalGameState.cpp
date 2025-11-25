// Fill out your copyright notice in the Description page of Project Settings.


#include "SurvivalGameState.h"
#include "Kismet/GameplayStatics.h"

ASurvivalGameState::ASurvivalGameState()
{
	PrimaryActorTick.bCanEverTick = true;

	TotalTimeElapsed = 0.0f;
	TimeSinceLastUIUpdate = 0.0f;
	EnemiesKilled = 0;

	bBossHasSpawned = false;
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

	if (!bBossHasSpawned && TotalTimeElapsed >= BossSpawnTime)
	{
		// Verrouiller `le spawn
		bBossHasSpawned = true;

		if (BossClass)
		{
			// Trouver le joueur pour savoir où spawner
			ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);

			if (PlayerChar)
			{
				// Calculer une position devant le joueur
				FVector PlayerLoc = PlayerChar->GetActorLocation();
				FVector PlayerFwd = PlayerChar->GetActorForwardVector();

				// Position = Joueur + (Devant * Distance)
				FVector SpawnLoc = PlayerLoc + (PlayerFwd * BossSpawnDistance);

				// On ajoute un peu de hauteur (Z) pour être sûr qu'il ne spawn pas dans le sol
				SpawnLoc.Z += 300.0f;

				// Paramètres de spawn (Force le spawn même s'il y a une petite collision)
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				// SPAWN !
				GetWorld()->SpawnActor<ABossEnemy>(
					BossClass,
					SpawnLoc,
					FRotator::ZeroRotator, // Face au monde (ou PlayerChar->GetActorRotation().GetInverse())
					SpawnParams
				);

				// UE_LOG(LogTemp, Warning, TEXT(" LE BOSS EST ARRIVÉ !"));
			}
		}
	}
}

void ASurvivalGameState::IncrementKillCount()
{
	EnemiesKilled++;

	OnKillsUpdated.Broadcast(EnemiesKilled);
}

void ASurvivalGameState::TriggerVictory(bool GameStatus)
{
	OnVictory(GameStatus);
}