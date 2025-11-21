// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemyPoolManager.h"
#include "MyEnemyArchetype.h" 
#include "TimerManager.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "AIController.h"
#include "EnemyBase.h"
#include "BehaviorTree/BehaviorTree.h"
//#include "BehaviorTree/BrainComponent.h" // <-- nécessaire pour StopLogic
#include "../ProjetProgFinalCharacter.h"
#include "GameFramework/PlayerController.h"

AEnemyPoolManager::AEnemyPoolManager()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.1f; // Tick toutes les 0.1s (10 Hz) pour économiser du CPU
}

void AEnemyPoolManager::BeginPlay()
{
	Super::BeginPlay();

	APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
	CachedPlayerCharacter = Cast<AProjetProgFinalCharacter>(PlayerPawn);

	if (!CachedPlayerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("EnemyPoolManager: Impossible de trouver 'AProjetProgFinalCharacter'. Le pooling va échouer."));
		return; // On arrête tout si on ne trouve pas le joueur
	}

	// Spawn tous les ennemis et les cache
	SpawnInitialPool();

	// Le spawn sera géré par Tick() avec les courbes
}

void AEnemyPoolManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!CachedPlayerCharacter) return;

	// Incrémente le temps de jeu
	GameTimeElapsed += DeltaTime;

	// Traite chaque pool
	for (int32 i = 0; i < EnemyPools.Num(); i++)
	{
		FEnemyPool& Pool = EnemyPools[i];
		UMyEnemyArchetype* Archetype = Pool.Archetype.Get();

		if (!Archetype) continue;

		// Vérifie si l'archetype a une courbe de spawn
		if (!Archetype->SpawnRateCurve)
		{
			// Pas de courbe = pas de spawn automatique pour cet archetype
			continue;
		}

		// Évalue la courbe au temps actuel
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

FVector AEnemyPoolManager::GetRandomSpawnLocationAroundPlayer(float Distance)
{
	if (!CachedPlayerCharacter) return FVector::ZeroVector;

	FVector PlayerPos = CachedPlayerCharacter->GetActorLocation();

	// Angle aléatoire (0 à 360 degrés en radians)
	float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);

	// Calcul trigonométrique simple (Cercle)
	float OffsetX = FMath::Cos(RandomAngle) * Distance;
	float OffsetY = FMath::Sin(RandomAngle) * Distance;

	FVector SpawnPos = PlayerPos + FVector(OffsetX, OffsetY, 0.0f);

	// Ajuster la hauteur (Z) pour ne pas spawn sous le sol
	// SpawnPos.Z = PlayerPos.Z; 

	return SpawnPos;
}

void AEnemyPoolManager::SpawnEnemyFromPool(int32 PoolIndex, FEnemyPool& Pool)
{
	if (Pool.PooledEnemies.Num() == 0) return;

	// Déterminer la distance selon le type de pool (0=Small, 1=Medium, 2=Large)
	float DistanceToUse = SpawnDistanceSmall;
	if (PoolIndex == 1) DistanceToUse = SpawnDistanceMedium;
	if (PoolIndex == 2) DistanceToUse = SpawnDistanceLarge;

	// Récupérer le prochain ennemi
	ACharacter* EnemyToActivate = Pool.PooledEnemies[Pool.NextIndex];

	// --- CHANGEMENT ICI : On calcule la position nous-mêmes ---
	FVector TargetLocation = GetRandomSpawnLocationAroundPlayer(DistanceToUse);

	// Activer l'ennemi
	if (IsValid(EnemyToActivate))
	{
		ActivateEnemy(EnemyToActivate, TargetLocation, Pool.Archetype.Get());
	}

	// Passer au suivant (en boucle)
	Pool.NextIndex = (Pool.NextIndex + 1) % Pool.PooledEnemies.Num();
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

				if (AEnemyBase* MyEnemy = Cast<AEnemyBase>(SpawnedEnemy))
				{
					MyEnemy->MaxHealth = Archetype->BaseHealth;
					MyEnemy->CurrentHealth = MyEnemy->MaxHealth;

					MyEnemy->AttackDamage = Archetype->ContactDamage;
					MyEnemy->LootDropClass = Archetype->LootDropClass;

					MyEnemy->PoolManagerRef = this;
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

void AEnemyPoolManager::DeactivateEnemy(ACharacter* Enemy)
{
	if (!Enemy) return;

	if (AEnemyBase* MyEnemy = Cast<AEnemyBase>(Enemy))
	{
		// On remet la vie au maximum
		MyEnemy->CurrentHealth = MyEnemy->MaxHealth;
	}

	UE_LOG(LogTemp, Warning, TEXT("♻️ RECYCLAGE: Ennemi %s remis dans la pool (inactif)"), *Enemy->GetName());

	// Cache l'acteur
	Enemy->SetActorHiddenInGame(true);
	// Désactive les collisions
	Enemy->SetActorEnableCollision(false);
	
	// Arrête son cerveau (le Behavior Tree)
	AAIController* AI = Cast<AAIController>(Enemy->GetController());
	if (AI && AI->GetBrainComponent())
	{
		AI->GetBrainComponent()->StopLogic(TEXT("Pooled"));
	}

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

	UE_LOG(LogTemp, Warning, TEXT("⚡ ACTIVATION: Ennemi %s sorti de la pool (actif)"), *Enemy->GetName());

	// Téléporte à la position visible
	Enemy->SetActorLocation(TeleportLocation);
	
	// Rendre visible
	Enemy->SetActorHiddenInGame(false);
	// Active les collisions
	Enemy->SetActorEnableCollision(true);

	// Réactive le tick de l'acteur
	Enemy->SetActorTickEnabled(true);
	
	// Réduire le tick rate du contrôleur IA
	AAIController* AI = Cast<AAIController>(Enemy->GetController());
	if (AI)
	{
		// Réactive le tick du contrôleur IA
		AI->SetActorTickEnabled(true);
	}
	
	AI = Cast<AAIController>(Enemy->GetController());
	if (AI)
	{
		// Réduire la fréquence de tick de l'IA à 30 Hz (0.033s) pour un bon équilibre performance/fluidité
		AI->SetActorTickInterval(0.033f);
		
		// Démarre le Behavior Tree
		if (Archetype->BehaviorTree)
		{
			AI->RunBehaviorTree(Archetype->BehaviorTree.Get());
		}
	}

	// Réduire le tick des animations
	if (USkeletalMeshComponent* Mesh = Enemy->GetMesh())
	{
		// Réactive le tick du mesh
		Mesh->SetComponentTickEnabled(true);
		
		// Ne tick les animations que quand l'ennemi est visible à l'écran
		Mesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
		
		// Réduire la fréquence de tick des animations à 20 Hz (0.05s)
		Mesh->SetComponentTickInterval(0.05f);
		
		// Désactive les ombres dynamiques pour économiser du GPU
		Mesh->SetCastShadow(false);
	}

	// Simplifie les calculs ---
	if (UCharacterMovementComponent* MoveComp = Enemy->GetCharacterMovement())
	{
		// Réactive le tick du movement
		MoveComp->SetComponentTickEnabled(true);
		
		// Réduire la fréquence de mise à jour du mouvement à 30 Hz (0.033s)
		MoveComp->SetComponentTickInterval(0.033f); // 30 Hz pour un mouvement fluide
	}
}

// Implémentation de la nouvelle fonction
void AEnemyPoolManager::RecycleEnemy(ACharacter* EnemyToRecycle)
{
	// On appelle simplement notre fonction privée de désactivation
	// pour cacher l'ennemi, arrêter son IA et le remettre dans le pool.
	DeactivateEnemy(EnemyToRecycle);
}