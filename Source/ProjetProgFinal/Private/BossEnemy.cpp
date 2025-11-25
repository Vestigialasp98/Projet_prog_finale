// Fill out your copyright notice in the Description page of Project Settings.


#include "BossEnemy.h"
#include "SurvivalGameState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"

ABossEnemy::ABossEnemy() 
{ 
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	GetCharacterMovement()->MaxWalkSpeed = 100.f;
}

void ABossEnemy::BeginPlay()
{
	Super::BeginPlay();

	AttackDamage = BossDamage;
	CurrentHealth = MaxHealth;

	if (BossBehaviorTree)
	{
		AAIController* AI = Cast<AAIController>(GetController());
		if (AI)
		{
			AI->RunBehaviorTree(BossBehaviorTree);
		}
	}
}

void ABossEnemy::Die()
{

	SetActorEnableCollision(false);
	if (GetController()) GetController()->StopMovement();

	FreezeAllEnemies();

	PlayDeathSequence();
}

void ABossEnemy::FreezeAllEnemies()
{
    TArray<AActor*> FoundEnemies;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemyBase::StaticClass(), FoundEnemies);

    for (AActor* Actor : FoundEnemies)
    {
        // Ne pas geler le boss
        if (Actor == this) continue;

        if (AEnemyBase* Enemy = Cast<AEnemyBase>(Actor))
        {
            // Coupe l'AI
            AAIController* AI = Cast<AAIController>(Enemy->GetController());
            if (AI && AI->GetBrainComponent())
            {
                AI->GetBrainComponent()->StopLogic("BossDied");
            }
            // Coupe le mouvement
            if (Enemy->GetCharacterMovement())
            {
                Enemy->GetCharacterMovement()->StopMovementImmediately();
                Enemy->GetCharacterMovement()->DisableMovement();
            }
            // Coupe les animations
            if (Enemy->GetMesh())
            {
                Enemy->GetMesh()->bPauseAnims = true;
            }
        }
    }

    ACharacter* Player = UGameplayStatics::GetPlayerCharacter(this, 0);
    if (Player)
    {
        // A. Désactiver les Inputs (Clavier/Souris ne répondent plus)
        if (APlayerController* PC = Cast<APlayerController>(Player->GetController()))
        {
            Player->DisableInput(PC);
        }
    }
}

float ABossEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	// Applique les dégâts
	
    UpdateBossHealthUI();

	return ActualDamage;
}