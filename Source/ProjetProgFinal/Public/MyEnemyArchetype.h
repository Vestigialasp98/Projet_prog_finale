// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameFramework/Character.h"
#include "BehaviorTree/BehaviorTree.h"
#include "MyEnemyArchetype.generated.h"

/**
 * Archétype d'ennemi.
 * On y met la classe à instancier (Blueprint) + des stats/paramètres par défaut.
 */
UCLASS()
class PROJETPROGFINAL_API UMyEnemyArchetype : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public :
	//uproperty pawn blueprint override
	
	
	// Classe Character de l’ennemi (souvent un Blueprint dérivé de BP_ThirdPersonCharacter)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Enemy")
	TSubclassOf<ACharacter> EnemyClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Enemy|Visuals")
	TObjectPtr<UMaterialInterface> OverrideMaterial;

	// Exemples de paramètres que tu peux réutiliser à l’instantiation
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Enemy|Stats")
	float BaseHealth = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Enemy|Stats")
	float MoveSpeed = 600.f;

	// Taille (échelle) de l'ennemi au spawn. (1.0, 1.0, 1.0) est la taille normale.
	/*UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Enemy|Stats")
	FVector SpawnScale = FVector::OneVector;*/

	// Arbre de comportement à lancer pour cet ennemi
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Enemy|AI")
	TObjectPtr<UBehaviorTree> BehaviorTree;

	// Tag(s) de gameplay, icône, etc. (optionnels)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Enemy|Meta")
	FName EnemyId = NAME_None;

public:
	// Permet de classer ce type d’asset dans le Primary Asset System (facultatif mais propre)
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

#if WITH_EDITOR
	// Petite validation côté éditeur
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
};
