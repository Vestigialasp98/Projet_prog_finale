// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
// --- CORRECTION ---
// On inclut la définition complète au lieu d'une "forward declaration"
// car TObjectPtr en a besoin.
#include "MyEnemyArchetype.h" 
// --- FIN CORRECTION ---
#include "MyEnemySpawner.generated.h"

// La 'forward declaration' "class UMyEnemyArchetype;" n'est plus nécessaire.

UCLASS()
class PROJETPROGFINAL_API AMyEnemySpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMyEnemySpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/**
	 * C'est la "recette" de l'ennemi à faire apparaître.
	 * 'EditInstanceOnly' signifie que vous pouvez la changer sur chaque spawner placé dans le niveau.
	 * 'Category' la range proprement dans le panneau Details.
	 */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Spawner Config")
	TObjectPtr<UMyEnemyArchetype> EnemyArchetype;

};