// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyTest.h"
#include "SurvivalGameState.h"

// Sets default values
AEnemyTest::AEnemyTest()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AEnemyTest::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;
}

// Called every frame
void AEnemyTest::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

float AEnemyTest::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	CurrentHealth -= ActualDamage;

	UE_LOG(LogTemp, Warning, TEXT("Degat: %f / Vie restante: %f"), ActualDamage, CurrentHealth);

    if (DamagePopupClass)
    {
        // Spawn un peu au-dessus de la tête (Z + 50 ou 100)
        FVector SpawnLocation = GetActorLocation() + FVector(0.f, 0.f, 100.f);

        // On ajoute un petit offset aléatoire en X/Y pour que les chiffres ne se superposent pas trop
        float RandomX = FMath::RandRange(-50.f, 50.f);
        float RandomY = FMath::RandRange(-50.f, 50.f);
        SpawnLocation += FVector(RandomX, RandomY, 0.f);

        ADamagePopup* Popup = GetWorld()->SpawnActor<ADamagePopup>(
            DamagePopupClass,
            SpawnLocation,
            FRotator::ZeroRotator
        );

        if (Popup)
        {
            // C'est ici qu'on passe le chiffre au Blueprint !
            Popup->UpdateDamageVisuals(ActualDamage);
        }
    }

	if (CurrentHealth <= 0.0f)
	{
        ASurvivalGameState* GS = GetWorld()->GetGameState<ASurvivalGameState>();

        if (GS)
        {
            GS->IncrementKillCount();
        }

		Destroy();
	}

	return ActualDamage;
}