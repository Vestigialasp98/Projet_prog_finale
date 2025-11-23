// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyEnemyArchetype.h" 
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

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Spawner Config")
	TObjectPtr<UMyEnemyArchetype> EnemyArchetype;

};