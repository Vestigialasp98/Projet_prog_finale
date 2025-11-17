// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
// --- CORRECTION ---
// On inclut la définition complète car TObjectPtr en a besoin
#include "MyEnemyArchetype.h"
// --- FIN CORRECTION ---
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

	// --- AJOUT : Nouvelle fonction publique ---
public:
	/**
	 * Demande au manager de recycler un ennemi (le remet dans le pool).
	 * Appelable depuis les Blueprints (ex: quand l'ennemi "meurt").
	 */
	UFUNCTION(BlueprintCallable, Category = "Enemy Pool Manager")
	void RecycleEnemy(ACharacter* EnemyToRecycle);
	// --- FIN AJOUT ---

protected:
	virtual void BeginPlay() override;

	/** Les 3 archétypes (Small, Medium, Big) que nous allons gérer.
	 * L'ordre est important ! */
	UPROPERTY(EditInstanceOnly, Category = "Config|Pool")
	TArray<TObjectPtr<UMyEnemyArchetype>> ArchetypesToPool;

	/** Le nombre d'ennemis à pré-spawner pour CHAQUE type. */
	UPROPERTY(EditInstanceOnly, Category = "Config|Pool")
	int32 InitialPoolSizePerArchetype = 300;

	/** La position cachée où les ennemis sont spawnés et attendent. */
	UPROPERTY(EditInstanceOnly, Category = "Config|Pool")
	FVector HiddenSpawnLocation = FVector(0.f, 0.f, -2000.f);

	// --- SUPPRESSION : Remplacé par la logique dynamique ---
	/** Les 3 acteurs cibles (ex: ATargetPoint) où téléporter les ennemis.
	 * DOIT correspondre à l'ordre de 'ArchetypesToPool'.
	 */
	// UPROPERTY(EditInstanceOnly, Category = "Config|Teleport")
	// TArray<AActor*> TeleportTargets;
	// --- FIN SUPPRESSION ---

	/** L'intervalle (en secondes) entre chaque téléportation. */
	UPROPERTY(EditInstanceOnly, Category = "Config|Teleport")
	float TeleportInterval = 5.0f;

private:
	/** La fonction qui s'exécute toutes les 10 secondes */
	void OnTeleportTimerFired();

	/** Fonction pour cacher et désactiver un ennemi */
	void DeactivateEnemy(ACharacter* Enemy);

	/** Fonction pour activer et téléporter un ennemi */
	void ActivateEnemy(ACharacter* Enemy, const FVector& TeleportLocation, UMyEnemyArchetype* Archetype);

	/** Fonction pour pré-spawner tous les ennemis au début */
	void SpawnInitialPool();

	/** Le handle pour notre timer de 10s */
	FTimerHandle TeleportTimerHandle;

	/** Nos pools d'ennemis (ex: 1 pool pour "Small", 1 pour "Medium", etc.) */
	UPROPERTY()
	TArray<FEnemyPool> EnemyPools;

	// --- AJOUT : Référence au joueur ---
	/** Référence au personnage joueur, mise en cache au BeginPlay */
	UPROPERTY()
	TObjectPtr<AProjetProgFinalCharacter> CachedPlayerCharacter;
	// --- FIN AJOUT ---
};