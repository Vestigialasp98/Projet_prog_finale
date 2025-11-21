// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/WidgetComponent.h"
#include "DamagePopup.generated.h"

UCLASS()
class PROJETPROGFINAL_API ADamagePopup : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADamagePopup();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UWidgetComponent* DamageWidgetComp;

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateDamageVisuals(float Amount);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
