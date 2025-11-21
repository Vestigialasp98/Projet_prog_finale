// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <Components/BoxComponent.h>
#include  "PlayerDataAsset.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AttackBox.generated.h"

UCLASS()
class PROJETPROGFINAL_API AAttackBox : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAttackBox();

	float DamageAmount;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Hitbox")
	UBoxComponent* Box;

	UFUNCTION()
	void SetupHitbox(const FVector& Scale, float Duration, float InDamage);

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY()
	TArray<AActor*> HitActors;
};
