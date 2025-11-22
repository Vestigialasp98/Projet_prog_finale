// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyBase.h"
#include "Components/CapsuleComponent.h"
#include "SurvivalGameState.h"
#include "../ProjetProgFinalCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EnemyPoolManager.h"

AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;

	Tags.Add(FName("Enemy"));

	GetMesh()->SetCollisionProfileName(TEXT("NoCollision"));
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));

	GetCharacterMovement()->bUseRVOAvoidance = true;
	GetCharacterMovement()->AvoidanceWeight = 0.5f;

	GetCharacterMovement()->MaxDepenetrationWithGeometry = 500.0f;
	GetCharacterMovement()->MaxDepenetrationWithPawn = 100.0f;
	 
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;
	
	// Trouve le manager une bonne fois pour toutes
	AActor* ManagerActor = UGameplayStatics::GetActorOfClass(GetWorld(), AEnemyPoolManager::StaticClass());
	PoolManagerRef = Cast<AEnemyPoolManager>(ManagerActor);
}

// Called to bind functionality to input
void AEnemyBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

float AEnemyBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	// Applique les dégâts
	CurrentHealth -= ActualDamage;

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
			// C'est ici qu'on passe le chiffre au Blueprint
			Popup->UpdateDamageVisuals(ActualDamage);
		}
	}
	// Vérifie la mort
	if (CurrentHealth <= 0.0f)
	{
		Die();
	}

	return ActualDamage;
}

void AEnemyBase::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	// Vérifie si on touche le Joueur
	if (Other && Other->IsA(AProjetProgFinalCharacter::StaticClass()))
	{
		// Applique les dégâts
		UGameplayStatics::ApplyDamage(
			Other,              // La victime (Le joueur)
			AttackDamage,       // Le montant
			GetController(),    // L'instigateur (l'ennemi)
			this,               
			UDamageType::StaticClass()
		);
	}
}

void AEnemyBase::FellOutOfWorld(const UDamageType& dmgType)
{
	UE_LOG(LogTemp, Warning, TEXT("Ennemi tombé dans le vide -> Recyclage immédiat"));

	if (PoolManagerRef)
	{
		CurrentHealth = MaxHealth; // Reset PV
		PoolManagerRef->RecycleEnemy(this);
	}
	else
	{
		Destroy(); // Fallback
	}
}

void AEnemyBase::Die()
{
	ASurvivalGameState* GS = GetWorld()->GetGameState<ASurvivalGameState>();

	if (GS)
	{
		GS->IncrementKillCount();
	}

	if (LootDropClass)
	{
		// On spawn un peu au-dessus du sol pour éviter que ça passe à travers
		FVector SpawnLoc = GetActorLocation() + FVector(0.f, 0.f, 50.f);

		// SpawnActor crée l'objet dans le monde
		GetWorld()->SpawnActor<AActor>(LootDropClass, SpawnLoc, FRotator::ZeroRotator);
	}

	if (PoolManagerRef)
	{
		// On remet la vie à fond pour la prochaine fois
		CurrentHealth = MaxHealth;

		// On retourne dans la boite
		PoolManagerRef->RecycleEnemy(this);
	}
	else
	{
		// Filet de sécurité si le manager n'est pas trouvé : on détruit vraiment
		Destroy();
	}
}
