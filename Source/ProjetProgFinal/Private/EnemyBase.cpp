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
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("EnemyProfile"));

	GetCharacterMovement()->bUseRVOAvoidance = true;
	GetCharacterMovement()->AvoidanceWeight = 0.5f;

	GetCharacterMovement()->bEnablePhysicsInteraction = false;
	GetCapsuleComponent()->CanCharacterStepUpOn = ECB_No;

	GetCharacterMovement()->MaxDepenetrationWithGeometry = 500.0f;
	GetCharacterMovement()->MaxDepenetrationWithPawn = 100.0f;
	 
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;

	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &AEnemyBase::OnOverlapBegin);
	
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
		// Spawn un peu au-dessus de la tete
		FVector SpawnLocation = GetActorLocation() + FVector(0.f, 0.f, 100.f);

		// On ajoute un petit offset aleatoire
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
			// Passe le chiffre au Blueprint
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

void AEnemyBase::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Vérifie si on touche le Joueur
	if (OtherActor && OtherActor->IsA(AProjetProgFinalCharacter::StaticClass()))
	{
		// Applique les dégâts
		UGameplayStatics::ApplyDamage(
			OtherActor,         // La victime (Le joueur)
			AttackDamage,       // Le montant
			GetController(),    // L'instigateur (l'ennemi)
			this,               
			UDamageType::StaticClass()
		);
	}
}

void AEnemyBase::FellOutOfWorld(const UDamageType& dmgType)
{
	if (PoolManagerRef)
	{
		CurrentHealth = MaxHealth; // Reset PV

		// UE_LOG(LogTemp, Warning, TEXT("RECYCLAGE : The enemy '%s' fell of the world"), *GetName());

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

	if (DeathSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation());
	}

	if (LootDropClass)
	{
		// On spawn un peu au-dessus du sol pour eviter que ca passe à travers
		FVector SpawnLoc = GetActorLocation() + FVector(0.f, 0.f, 50.f);

		GetWorld()->SpawnActor<AActor>(LootDropClass, SpawnLoc, FRotator::ZeroRotator);
	}

	if (PoolManagerRef)
	{
		// On remet la vie a fond pour la prochaine fois
		CurrentHealth = MaxHealth;

		// On retourne dans la boite
		PoolManagerRef->RecycleEnemy(this);
	}
	else
	{
		// Filet de sécurité si le manager n'est pas trouve
		Destroy();
	}
}
