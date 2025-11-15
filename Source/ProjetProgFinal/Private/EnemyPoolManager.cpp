// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemyPoolManager.h"
#include "MyEnemyArchetype.h" 
#include "TimerManager.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
//#include "BehaviorTree/BrainComponent.h" // <-- nécessaire pour StopLogic
#include "../ProjetProgFinalCharacter.h" // <-- AJOUT
#include "GameFramework/PlayerController.h" // <-- AJOUT

AEnemyPoolManager::AEnemyPoolManager()
{
	// On n'a pas besoin de Tick
	PrimaryActorTick.bCanEverTick = false;
}

void AEnemyPoolManager::BeginPlay()
{
	Super::BeginPlay();

	// --- MODIFICATION : Trouver et mettre en cache le joueur ---
	APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
	CachedPlayerCharacter = Cast<AProjetProgFinalCharacter>(PlayerPawn);

	if (!CachedPlayerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("EnemyPoolManager: Impossible de trouver 'AProjetProgFinalCharacter'. Le pooling va échouer."));
		return; // On arrête tout si on ne trouve pas le joueur
	}
	// --- FIN MODIFICATION ---

	// 1. Spawner tous les ennemis et les cacher
	SpawnInitialPool();

	// 2. Démarrer le timer de 10s pour téléporter les ennemis
	// --- MODIFICATION : On vérifie juste EnemyPools, plus TeleportTargets ---
	if (TeleportInterval > 0.f && EnemyPools.Num() > 0)
	{
		GetWorld()->GetTimerManager().SetTimer(
			TeleportTimerHandle, 
			this, 
			&AEnemyPoolManager::OnTeleportTimerFired, 
			TeleportInterval, 
			true, // 'true' pour que le timer se répète
			TeleportInterval // Délai initial avant le premier tir
		);
	}
}

void AEnemyPoolManager::SpawnInitialPool()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.Owner = this;

	// On itère sur le TArray de TObjectPtr
	for (const TObjectPtr<UMyEnemyArchetype>& ArchetypePtr : ArchetypesToPool)
	{
		// On récupère le pointeur brut (*.Get()) pour l'utiliser
		UMyEnemyArchetype* Archetype = ArchetypePtr.Get();
		
		if (!Archetype || !IsValid(Archetype->EnemyClass)) continue;

		FEnemyPool NewPool;
		NewPool.Archetype = ArchetypePtr; // On stocke le TObjectPtr dans le pool

		for (int32 i = 0; i < InitialPoolSizePerArchetype; i++)
		{
			// Spawner à la position cachée
			ACharacter* SpawnedEnemy = GetWorld()->SpawnActor<ACharacter>(
				Archetype->EnemyClass, 
				HiddenSpawnLocation, 
				FRotator::ZeroRotator, 
				SpawnParams
			);

			if (IsValid(SpawnedEnemy))
			{
				// --- CONFIGURATION INITIALE ---
				// 1. Appliquer le matériau
				if (Archetype->OverrideMaterial && IsValid(SpawnedEnemy->GetMesh()))
				{
					SpawnedEnemy->GetMesh()->SetMaterial(0, Archetype->OverrideMaterial.Get());
				}

				// 2. Appliquer la vitesse
				if (UCharacterMovementComponent* MoveComp = SpawnedEnemy->GetCharacterMovement())
				{
					MoveComp->MaxWalkSpeed = Archetype->MoveSpeed;
				}

				// 3. S'assurer qu'il a un contrôleur IA
				SpawnedEnemy->SpawnDefaultController();
				
				// 4. Désactiver l'ennemi et l'ajouter au pool
				DeactivateEnemy(SpawnedEnemy);
				NewPool.PooledEnemies.Add(SpawnedEnemy);
			}
		}

		EnemyPools.Add(NewPool);
	}
}

void AEnemyPoolManager::OnTeleportTimerFired()
{
	// --- MODIFICATION : Nouvelle logique de téléportation ---
	
	// 1. S'assurer qu'on a toujours le joueur
	if (!CachedPlayerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("EnemyPoolManager: Perte de la référence au joueur."));
		GetWorld()->GetTimerManager().ClearTimer(TeleportTimerHandle); // Arrêter le timer
		return;
	}

	// 2. Obtenir les positions de spawn DYNAMIQUES actuelles du joueur
	TArray<FTransform> SpawnTransforms;
	CachedPlayerCharacter->GetEnemySpawnTransforms(SpawnTransforms);

	// 3. S'assurer que le nombre de points de spawn correspond au nombre de pools
	if (SpawnTransforms.Num() != EnemyPools.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("EnemyPoolManager: Le nombre de pools (%d) ne correspond pas au nombre de points de spawn du joueur (%d)"), EnemyPools.Num(), SpawnTransforms.Num());
		return;
	}

	// 4. Boucler sur chaque pool et téléporter un ennemi
	for (int32 i = 0; i < EnemyPools.Num(); i++)
	{
		FEnemyPool& Pool = EnemyPools[i];
		if (Pool.PooledEnemies.Num() == 0) continue;

		// Récupérer le prochain ennemi "endormi"
		ACharacter* EnemyToActivate = Pool.PooledEnemies[Pool.NextIndex];
		
		// Obtenir la position de la cible DEPUIS LE JOUEUR
		FVector TargetLocation = SpawnTransforms[i].GetLocation();

		// Activer et téléporter l'ennemi
		if (IsValid(EnemyToActivate))
		{
			ActivateEnemy(EnemyToActivate, TargetLocation, Pool.Archetype.Get());
		}

		// Passer au suivant pour la prochaine fois (en boucle)
		Pool.NextIndex = (Pool.NextIndex + 1) % Pool.PooledEnemies.Num();
	}
	// --- FIN MODIFICATION ---
}

void AEnemyPoolManager::DeactivateEnemy(ACharacter* Enemy)
{
	if (!Enemy) return;

	// Cacher l'acteur
	Enemy->SetActorHiddenInGame(true);
	// Désactiver les collisions
	Enemy->SetActorEnableCollision(false);
	
	// Arrêter son cerveau (le Behavior Tree)
	AAIController* AI = Cast<AAIController>(Enemy->GetController());
	if (AI && AI->GetBrainComponent())
	{
		AI->GetBrainComponent()->StopLogic(TEXT("Pooled"));
	}

	// Téléporter à la cachette
	Enemy->SetActorLocation(HiddenSpawnLocation);
}

void AEnemyPoolManager::ActivateEnemy(ACharacter* Enemy, const FVector& TeleportLocation, UMyEnemyArchetype* Archetype)
{
	if (!Enemy || !Archetype) return;

	// Téléporter à la position visible
	Enemy->SetActorLocation(TeleportLocation);
	
	// Rendre visible
	Enemy->SetActorHiddenInGame(false);
	// Activer les collisions
	Enemy->SetActorEnableCollision(true);

	// Démarrer le cerveau (le Behavior Tree)
	AAIController* AI = Cast<AAIController>(Enemy->GetController());
	if (AI && Archetype->BehaviorTree)
	{
		AI->RunBehaviorTree(Archetype->BehaviorTree.Get());
	}
}

// --- AJOUT : Implémentation de la nouvelle fonction ---
void AEnemyPoolManager::RecycleEnemy(ACharacter* EnemyToRecycle)
{
	// On appelle simplement notre fonction privée de désactivation
	// pour cacher l'ennemi, arrêter son IA et le remettre dans le pool.
	DeactivateEnemy(EnemyToRecycle);
}
// --- FIN AJOUT ---