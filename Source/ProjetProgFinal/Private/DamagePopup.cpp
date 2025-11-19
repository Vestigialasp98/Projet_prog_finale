// Fill out your copyright notice in the Description page of Project Settings.


#include "DamagePopup.h"

// Sets default values
ADamagePopup::ADamagePopup()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	DamageWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("DamageWidget"));
	DamageWidgetComp->SetupAttachment(RootComponent);

	DamageWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
	DamageWidgetComp->SetDrawAtDesiredSize(true);

}

// Called when the game starts or when spawned
void ADamagePopup::BeginPlay()
{
	Super::BeginPlay();
	SetLifeSpan(1.5f);
}


