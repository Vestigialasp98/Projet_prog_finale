// Fill out your copyright notice in the Description page of Project Settings.


#include "MyEnemySpawner.h"
#include "MyEnemyArchetype.h" // <-- IMPORTANT : Inclure la définition de votre Data Asset
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "AIController.h" // <-- AJOUT POUR L'IA
#include "BehaviorTree/BehaviorTree.h" // <-- AJOUT POUR L'IA

// Sets default values
AMyEnemySpawner::AMyEnemySpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
// Fonction appelée au début du jeu
void AMyEnemySpawner::BeginPlay()
{
	Super::BeginPlay();

	// --- 1. Validation de la "recette" ---
	// On vérifie si un Data Asset a bien été assigné dans l'éditeur.
	if (!IsValid(EnemyArchetype))
	{
		UE_LOG(LogTemp, Warning, TEXT("EnemySpawner (%s) n'a pas d'EnemyArchetype assigné."), *GetName());
		return;
	}

	// On vérifie si la recette contient bien une classe d'ennemi.
	if (!IsValid(EnemyArchetype->EnemyClass))
	{
		UE_LOG(LogTemp, Warning, TEXT("EnemyArchetype (%s) n'a pas d'EnemyClass valide."), *EnemyArchetype->GetName());
		return;
	}

	// --- 2. Préparation du Spawn ---
	FVector SpawnLocation = GetActorLocation();
	FRotator SpawnRotation = GetActorRotation();
	FActorSpawnParameters SpawnParams;
	
	// On s'assure que l'ennemi apparaisse même s'il y a une petite collision
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	SpawnParams.Owner = this;

	// --- 3. Apparition (Spawn) de l'acteur ---
	ACharacter* SpawnedEnemy = GetWorld()->SpawnActor<ACharacter>(EnemyArchetype->EnemyClass, SpawnLocation, SpawnRotation, SpawnParams);

	// --- 4. Configuration de l'ennemi ---
	if (IsValid(SpawnedEnemy))
	{
		// A. Appliquer le matériau
		if (IsValid(EnemyArchetype->OverrideMaterial) && IsValid(SpawnedEnemy->GetMesh()))
		{
			// On applique le matériau sur le premier slot (index 0) du mesh
			SpawnedEnemy->GetMesh()->SetMaterial(0, EnemyArchetype->OverrideMaterial);
		}

		// B. Appliquer les stats (vitesse)
		if (UCharacterMovementComponent* MoveComp = SpawnedEnemy->GetCharacterMovement())
		{
			MoveComp->MaxWalkSpeed = EnemyArchetype->MoveSpeed;
		}

		// C. On vérifie si la taille est différente de 1.0 (taille par défaut)
		/*if (EnemyArchetype->SpawnScale != FVector::OneVector)
		{
			SpawnedEnemy->SetActorScale3D(EnemyArchetype->SpawnScale);
		}*/

		// D. Appliquer les stats (vie)
		// NOTE : ACharacter n'a pas de variable "Health" par défaut.
		// Vous devez caster vers VOTRE classe d'ennemi (ex: AMyEnemyCharacter)
		// et appeler une fonction comme SetHealth().
		//
		// Exemple (si votre ennemi s'appelle 'AMyBaseEnemy'):
		/*
		if (AMyBaseEnemy* MyEnemy = Cast<AMyBaseEnemy>(SpawnedEnemy))
		{
			MyEnemy->SetHealth(EnemyArchetype->BaseHealth);
		}
		*/

		if (EnemyArchetype->BehaviorTree)
		{
			// On s'assure que le contrôleur d'IA par défaut est créé
			SpawnedEnemy->SpawnDefaultController();
			
			// On récupère le contrôleur et on le cast en AAIController
			AAIController* AIController = Cast<AAIController>(SpawnedEnemy->GetController());
			
			if (AIController)
			{
				// On lance l'arbre de comportement
				AIController->RunBehaviorTree(EnemyArchetype->BehaviorTree);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Échec du lancement du BT : Le contrôleur de %s n'est pas un AAIController."), *SpawnedEnemy->GetName());
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Échec du spawn pour l'archétype %s !"), *EnemyArchetype->GetName());
	}
}

