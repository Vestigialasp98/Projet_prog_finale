// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackBox.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AAttackBox::AAttackBox()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	RootComponent = Box;

	Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Box->SetHiddenInGame(false);
	Box->SetBoxExtent(FVector(50.f, 50.f, 50.f));
	Box->SetCollisionProfileName("OverlapAllDynamic");
}


// Called when the game starts or when spawned
void AAttackBox::BeginPlay()
{
	Super::BeginPlay();
	Box->OnComponentBeginOverlap.AddDynamic(this, &AAttackBox::OnOverlapBegin);
}

// Called every frame
void AAttackBox::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AAttackBox::SetupHitbox(const FVector& Scale, float Duration, float InDamage)
{

	Box->SetWorldScale3D(Scale);
	SetLifeSpan(Duration);
	DamageAmount = InDamage;
	Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AAttackBox::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Si valide
	if (!OtherActor || OtherActor == this || OtherActor == GetOwner()) return;

	if (HitActors.Contains(OtherActor))
	{
		// Si l'acteur a deja ete touche (evite le double damage)
		return;
	}

	HitActors.Add(OtherActor);
	
	// Verifie si c'est un ennemi (avec un tag)
	if (OtherActor->ActorHasTag("Enemy"))
	{
		// Applique les degats standards d'Unreal
		UGameplayStatics::ApplyDamage(
			OtherActor,
			DamageAmount,
			GetInstigatorController(),
			this,
			UDamageType::StaticClass()
		);
	}
}