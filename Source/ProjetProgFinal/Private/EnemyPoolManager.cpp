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
	// --- MODIFICATION : On a besoin de Tick pour le système de courbes ---
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.1f; // Tick toutes les 0.1s (10 Hz) pour économiser du CPU
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

	// 2. Le spawn sera géré par Tick() avec les courbes
	// Plus besoin de timer fixe !
}

void AEnemyPoolManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!CachedPlayerCharacter) return;

	// Incrémenter le temps de jeu
	GameTimeElapsed += DeltaTime;

	// Traiter chaque pool
	for (int32 i = 0; i < EnemyPools.Num(); i++)
	{
		FEnemyPool& Pool = EnemyPools[i];
		UMyEnemyArchetype* Archetype = Pool.Archetype.Get();

		if (!Archetype) continue;

		// Vérifier si l'archetype a une courbe de spawn
		if (!Archetype->SpawnRateCurve)
		{
			// Pas de courbe = pas de spawn automatique pour cet archetype
			continue;
		}

		// Évaluer la courbe au temps actuel
		float SpawnRate = Archetype->SpawnRateCurve->GetFloatValue(GameTimeElapsed);

		// Si le taux de spawn est <= 0, ne rien faire
		if (SpawnRate <= 0.0f) continue;

		// Calculer combien d'ennemis spawner ce frame
		float EnemiesToSpawnThisFrame = SpawnRate * DeltaTime;
		Pool.EnemiesToSpawn += EnemiesToSpawnThisFrame;

		// Spawner les ennemis entiers (on garde la partie fractionnaire pour le prochain frame)
		while (Pool.EnemiesToSpawn >= 1.0f)
		{
			SpawnEnemyFromPool(i, Pool);
			Pool.EnemiesToSpawn -= 1.0f;
		}
	}
}

void AEnemyPoolManager::SpawnEnemyFromPool(int32 PoolIndex, FEnemyPool& Pool)
{
	int32 SpawnPointCount = CachedPlayerCharacter->GetSpawnPointCountForPool(PoolIndex);

	if (Pool.PooledEnemies.Num() == 0 || SpawnPointCount == 0) return;

	// Récupérer le prochain ennemi
	ACharacter* EnemyToActivate = Pool.PooledEnemies[Pool.NextIndex];

	// Obtenir le point de spawn
	FTransform TargetTransform = CachedPlayerCharacter->GetSpawnTransformForPool(PoolIndex, Pool.NextSpawnPointIndex);
	FVector TargetLocation = TargetTransform.GetLocation();

	// Activer l'ennemi
	if (IsValid(EnemyToActivate))
	{
		ActivateEnemy(EnemyToActivate, TargetLocation, Pool.Archetype.Get());
	}

	// Passer au suivant (en boucle)
	Pool.NextIndex = (Pool.NextIndex + 1) % Pool.PooledEnemies.Num();
	Pool.NextSpawnPointIndex = (Pool.NextSpawnPointIndex + 1) % SpawnPointCount;
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

	// --- DEBUG : Afficher le résumé de la pool ---
	int32 TotalEnemies = 0;
	for (const FEnemyPool& Pool : EnemyPools)
	{
		TotalEnemies += Pool.PooledEnemies.Num();
	}
	UE_LOG(LogTemp, Warning, TEXT("📦 POOL CRÉÉE: %d ennemis au total dans %d pools"), TotalEnemies, EnemyPools.Num());
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

	// 2. Boucler sur chaque pool et téléporter un ennemi
	// (L'index 'i' correspondra à 0=Small, 1=Medium, 2=Large, *SI* l'ordre est respecté)
	for (int32 i = 0; i < EnemyPools.Num(); i++)
	{
		FEnemyPool& Pool = EnemyPools[i];
		// Demande au joueur "Combien de points de spawn as-tu pour ce pool (i)?"
		int32 SpawnPointCount = CachedPlayerCharacter->GetSpawnPointCountForPool(i);

		// S'assurer qu'on a des ennemis ET des points de spawn
		if (Pool.PooledEnemies.Num() == 0 || SpawnPointCount == 0) continue;

		// Récupérer le prochain ennemi "endormi"
		ACharacter* EnemyToActivate = Pool.PooledEnemies[Pool.NextIndex];
		
		// Obtenir le point SÉQUENTIEL pour ce pool
		// "Donne-moi le transform pour le pool (i) au point (NextSpawnPointIndex)"
		FTransform TargetTransform = CachedPlayerCharacter->GetSpawnTransformForPool(i, Pool.NextSpawnPointIndex);
		FVector TargetLocation = TargetTransform.GetLocation();

		// Activer et téléporter l'ennemi
		if (IsValid(EnemyToActivate))
		{
			ActivateEnemy(EnemyToActivate, TargetLocation, Pool.Archetype.Get());
		}

		// Passer à l'ennemi suivant pour la prochaine fois (en boucle)
		Pool.NextIndex = (Pool.NextIndex + 1) % Pool.PooledEnemies.Num();

		// Passer au point de spawn suivant pour la prochaine fois (en boucle)
		Pool.NextSpawnPointIndex = (Pool.NextSpawnPointIndex + 1) % SpawnPointCount;
	}
	// --- FIN MODIFICATION ---
}

void AEnemyPoolManager::DeactivateEnemy(ACharacter* Enemy)
{
	if (!Enemy) return;

	// --- DEBUG : Log quand un ennemi est désactivé ---
	UE_LOG(LogTemp, Warning, TEXT("♻️ RECYCLAGE: Ennemi %s remis dans la pool (inactif)"), *Enemy->GetName());

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

	// --- OPTIMISATION CRITIQUE : Désactiver le tick de TOUS les composants ---
	// Désactiver le tick du mesh (animations)
	if (USkeletalMeshComponent* Mesh = Enemy->GetMesh())
	{
		Mesh->SetComponentTickEnabled(false);
	}
	
	// Désactiver le tick du movement
	if (UCharacterMovementComponent* MoveComp = Enemy->GetCharacterMovement())
	{
		MoveComp->SetComponentTickEnabled(false);
	}
	
	// Désactiver le tick de l'acteur lui-même
	Enemy->SetActorTickEnabled(false);
	
	// Désactiver le tick du contrôleur IA
	if (AI)
	{
		AI->SetActorTickEnabled(false);
	}

	// Téléporter à la cachette
	Enemy->SetActorLocation(HiddenSpawnLocation);
}

void AEnemyPoolManager::ActivateEnemy(ACharacter* Enemy, const FVector& TeleportLocation, UMyEnemyArchetype* Archetype)
{
	if (!Enemy || !Archetype) return;

	// --- DEBUG : Log quand un ennemi est activé ---
	UE_LOG(LogTemp, Warning, TEXT("⚡ ACTIVATION: Ennemi %s sorti de la pool (actif)"), *Enemy->GetName());

	// Téléporter à la position visible
	Enemy->SetActorLocation(TeleportLocation);
	
	// Rendre visible
	Enemy->SetActorHiddenInGame(false);
	// Activer les collisions
	Enemy->SetActorEnableCollision(true);

	// --- OPTIMISATION CRITIQUE : Réactiver le tick des composants ---
	// Réactiver le tick de l'acteur
	Enemy->SetActorTickEnabled(true);
	
	// --- OPTIMISATION IA : Réduire le tick rate du contrôleur IA ---
	AAIController* AI = Cast<AAIController>(Enemy->GetController());
	if (AI)
	{
		// Réactiver le tick du contrôleur IA
		AI->SetActorTickEnabled(true);
	}
	
	AI = Cast<AAIController>(Enemy->GetController());
	if (AI)
	{
		// Réduire la fréquence de tick de l'IA à 30 Hz (0.033s) pour un bon équilibre performance/fluidité
		AI->SetActorTickInterval(0.033f);
		
		// Démarrer le Behavior Tree
		if (Archetype->BehaviorTree)
		{
			AI->RunBehaviorTree(Archetype->BehaviorTree.Get());
		}
	}

	// --- OPTIMISATION ANIMATIONS : Réduire le tick des animations ---
	if (USkeletalMeshComponent* Mesh = Enemy->GetMesh())
	{
		// Réactiver le tick du mesh
		Mesh->SetComponentTickEnabled(true);
		
		// Ne tick les animations que quand l'ennemi est visible à l'écran
		Mesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
		
		// Réduire la fréquence de tick des animations à 20 Hz (0.05s)
		Mesh->SetComponentTickInterval(0.05f);
		
		// Désactiver les ombres dynamiques pour économiser du GPU
		Mesh->SetCastShadow(false);
	}

	// --- OPTIMISATION MOUVEMENT : Simplifier les calculs ---
	if (UCharacterMovementComponent* MoveComp = Enemy->GetCharacterMovement())
	{
		// Réactiver le tick du movement
		MoveComp->SetComponentTickEnabled(true);
		
		// Réduire la fréquence de mise à jour du mouvement à 30 Hz (0.033s)
		MoveComp->SetComponentTickInterval(0.033f); // 30 Hz pour un mouvement fluide
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