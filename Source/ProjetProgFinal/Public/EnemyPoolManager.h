// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
// On inclut la définition complète car TObjectPtr en a besoin
#include "MyEnemyArchetype.h"
#include "EnemyPoolManager.generated.h"

// Forward declarations
class ACharacter;
class AProjetProgFinalCharacter; // Ajouté pour le CachedPlayerCharacter

/**
 * Structure pour organiser nos pools.
 * Associe un archétype à une liste d'acteurs.
 */
USTRUCT(BlueprintType)
struct FEnemyPool
{
	GENERATED_BODY()

	// L'archétype pour ce pool (ex: "Small")
	UPROPERTY()
	TObjectPtr<UMyEnemyArchetype> Archetype;

	// La liste des ennemis pré-spawnés pour ce type
	UPROPERTY()
	TArray<TObjectPtr<ACharacter>> PooledEnemies;

	// Index du prochain ennemi à utiliser
	int32 NextIndex = 0;

	// Index du prochain point de spawn à utiliser
	int32 NextSpawnPointIndex = 0;

	// Nombre d'ennemis à spawner (peut être fractionnaire pour accumulation)
	float EnemiesToSpawn = 0.0f;

	// Temps de jeu écoulé depuis le début (en secondes)
	float GameTimeElapsed = 0.0f;
};

/**
 * Acteur Manager qui pré-spawne les ennemis
 * et les téléporte à intervalle régulier.
 */
UCLASS()
class PROJETPROGFINAL_API AEnemyPoolManager : public AActor
{
	GENERATED_BODY()
	
public:	
	AEnemyPoolManager();

	virtual void Tick(float DeltaTime) override;

public:
	// Demande au manager de recycler un ennemi (le remet dans le pool) en BP
	UFUNCTION(BlueprintCallable, Category = "Enemy Pool Manager")
	void RecycleEnemy(ACharacter* EnemyToRecycle);

protected:
	virtual void BeginPlay() override;

	// Les 3 archétypes (Small, Medium, Big) que nous allons gérer
	// L'ordre est important !
	UPROPERTY(EditInstanceOnly, Category = "Config|Pool")
	TArray<TObjectPtr<UMyEnemyArchetype>> ArchetypesToPool;

	// Le nombre d'ennemis à pré-spawner pour chaque type
	UPROPERTY(EditInstanceOnly, Category = "Config|Pool")
	int32 InitialPoolSizePerArchetype = 300;

	// La position cachée où les ennemis sont spawnés et attendent
	UPROPERTY(EditInstanceOnly, Category = "Config|Pool")
	FVector HiddenSpawnLocation = FVector(0.f, 0.f, -2000.f);

	// L'intervalle (en secondes) entre chaque téléportation
	UPROPERTY(EditInstanceOnly, Category = "Config|Teleport")
	float TeleportInterval = 5.0f;

	// Distances de spawn (Remplacent les positions des sphères)
	UPROPERTY(EditDefaultsOnly, Category = "Config|Spawning")
	float SpawnDistanceSmall = 1500.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Config|Spawning")
	float SpawnDistanceMedium = 2000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Config|Spawning")
	float SpawnDistanceLarge = 2500.0f;

private:
	// La fonction qui s'exécute toutes les 10 secondes
	void OnTeleportTimerFired();

	// Fonction pour cacher et désactiver un ennemi
	void DeactivateEnemy(ACharacter* Enemy);

	// Fonction pour activer et téléporter un ennemi
	void ActivateEnemy(ACharacter* Enemy, const FVector& TeleportLocation, UMyEnemyArchetype* Archetype);

	// Fonction pour pré-spawner tous les ennemis au début
	void SpawnInitialPool();

	// Spawn un ennemi depuis un pool spécifique
	void SpawnEnemyFromPool(int32 PoolIndex, FEnemyPool& Pool);

	// Le handle pour le timer de 10s
	FTimerHandle TeleportTimerHandle;

	// Pools d'ennemis
	UPROPERTY()
	TArray<FEnemyPool> EnemyPools;

	// Référence au personnage joueur, mise en cache au BeginPlay
	UPROPERTY()
	TObjectPtr<AProjetProgFinalCharacter> CachedPlayerCharacter;


	// Temps de jeu écoulé depuis le début (en secondes)
	float GameTimeElapsed = 0.0f;

	// Nouvelle fonction pour calculer une position sans aide du Character
	FVector GetRandomSpawnLocationAroundPlayer(float Distance);
};