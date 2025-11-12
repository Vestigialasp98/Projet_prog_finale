// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FindPlayer.generated.h"

/**
 * Tâche de Behavior Tree pour trouver le joueur et le stocker dans une clé de Blackboard.
 */
UCLASS()
class PROJETPROGFINAL_API UBTTask_FindPlayer : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_FindPlayer();

	/** Clé de Blackboard (de type Object ou Actor) dans laquelle stocker le joueur trouvé. */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;

protected:
	/** Fonction principale de la tâche */
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	/** Utilisé pour afficher un nom descriptif dans l'éditeur de Behavior Tree */
	virtual FString GetStaticDescription() const override;
};