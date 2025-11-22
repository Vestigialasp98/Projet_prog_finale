// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyEnemyArchetype.h"
#include "DamagePopup.h"
#include "GameFramework/Character.h"
#include "EnemyBase.generated.h"

class AEnemyPoolManager;

UCLASS()
class PROJETPROGFINAL_API AEnemyBase : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemyBase();

protected:
	virtual void BeginPlay() override;

	virtual void NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

	// Fonction appelée quand l'acteur dépasse un offset du monde (Z)
	virtual void FellOutOfWorld(const UDamageType& dmgType) override;

public:	

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stats")
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float CurrentHealth;

	UPROPERTY(EditAnywhere, Category = "Combat")
	TSubclassOf<class ADamagePopup> DamagePopupClass;

	UPROPERTY()
	UMyEnemyArchetype* EnemyDataAsset;

	// Damage recu par les ennemis
	float AttackDamage = 0.0f;
	TSubclassOf<AActor> LootDropClass;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// La fonction override standard d'Unreal pour recevoir des coups
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	void Die();

	UPROPERTY()
	AEnemyPoolManager* PoolManagerRef;
};
