// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackBox.h"

// Sets default values
AAttackBox::AAttackBox()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	RootComponent = Box;

	Box->SetHiddenInGame(false);
	Box->SetBoxExtent(FVector(50.f, 50.f, 50.f));
	Box->SetCollisionProfileName("OverlapAll");
}


// Called when the game starts or when spawned
void AAttackBox::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAttackBox::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AAttackBox::SetupHitbox(const FVector& Scale, float Duration)
{
	Box->SetWorldScale3D(Scale);
	SetLifeSpan(Duration);

	DrawDebugBox(
		GetWorld(),
		GetActorLocation(),
		Box->GetScaledBoxExtent(),
		GetActorQuat(),
		FColor::Red,
		false,
		0.5f
	);
}

