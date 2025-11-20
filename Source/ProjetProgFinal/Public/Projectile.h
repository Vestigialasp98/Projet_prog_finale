// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraComponent.h"
#include "Components/SphereComponent.h"
#include "Projectile.generated.h"

UCLASS()
class PROJETPROGFINAL_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectile();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	USphereComponent* CollisionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
	UNiagaraComponent* NiagaraComp;

	// Fonction pour initialiser la balle
	void SetupProjectile(float Damage, float Speed, float Range);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	float DamageAmount;
	TArray<AActor*> HitActors;

	FTimerHandle FadeTimerHandle;

	void StartFadeOut();

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
