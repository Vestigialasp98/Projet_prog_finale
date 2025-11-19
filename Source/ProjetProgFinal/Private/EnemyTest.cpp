// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyTest.h"

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

	if (CurrentHealth <= 0.0f)
	{
		Destroy();
	}

	return ActualDamage;
}