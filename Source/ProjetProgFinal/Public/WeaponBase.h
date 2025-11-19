// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlayerDataAsset.h"
#include "WeaponBase.generated.h"

UCLASS()
class PROJETPROGFINAL_API AWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponBase();

	virtual void InitWeapon(UPlayerDataAsset* WeaponData);

	float CurrentDamage;
	float CurrentCooldown;
	FVector CurrentHitboxScale;
	float CurrentDuration;

	UPlayerDataAsset* GetSourceDataAsset() const { return SourceDataAsset; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY()
	UPlayerDataAsset* SourceDataAsset;

	FTimerHandle AttackTimerHandle;

	virtual void Attack();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
