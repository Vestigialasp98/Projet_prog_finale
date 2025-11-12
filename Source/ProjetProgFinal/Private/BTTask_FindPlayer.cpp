// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_FindPlayer.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h" // On peut utiliser GetPlayerCharacter aussi
#include "AIController.h"

UBTTask_FindPlayer::UBTTask_FindPlayer()
{
	// Nomme le nœud dans l'éditeur pour qu'il soit facile à lire
	NodeName = "Find Player and Set Key";

	// S'assure que la clé qu'on sélectionne est bien de type "Object" (Actor hérite de Object)
	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_FindPlayer, TargetActorKey), AActor::StaticClass());
}

EBTNodeResult::Type UBTTask_FindPlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// --- 1. Obtenir les composants nécessaires ---
	AAIController* AIController = OwnerComp.GetAIOwner();
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

	if (!IsValid(AIController) || !IsValid(BlackboardComp))
	{
		// Si on n'a pas de contrôleur ou de blackboard, la tâche échoue
		return EBTNodeResult::Failed;
	}

	// --- 2. Trouver le joueur ---
	// On utilise GetPlayerPawn pour être générique (pourrait être une voiture, etc.)
	// On utilise GetWorld() depuis le contrôleur
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(AIController->GetWorld(), 0);

	if (!IsValid(PlayerPawn))
	{
		// Si on ne trouve pas de joueur, la tâche échoue
		UE_LOG(LogTemp, Warning, TEXT("BTTask_FindPlayer: Impossible de trouver le Player Pawn."));
		BlackboardComp->ClearValue(TargetActorKey.SelectedKeyName); // On efface la clé
		return EBTNodeResult::Failed;
	}

	// --- 3. Mettre à jour le Blackboard ---
	// On stocke le joueur (un UObject) dans la clé de Blackboard
	BlackboardComp->SetValueAsObject(TargetActorKey.SelectedKeyName, PlayerPawn);

	// On dit au Behavior Tree que la tâche a réussi
	return EBTNodeResult::Succeeded;
}

FString UBTTask_FindPlayer::GetStaticDescription() const
{
	// Personnalise la description de l'éditeur
	return FString::Printf(TEXT("Set Blackboard Key '%s' to Player"), *TargetActorKey.SelectedKeyName.ToString());
}

