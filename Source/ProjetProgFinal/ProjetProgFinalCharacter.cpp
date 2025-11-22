// Copyright Epic Games, Inc. All RightsC reserved.

#include "ProjetProgFinalCharacter.h"
#include <AttackBox.h>
#include "PlayerDataAsset.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Components/SphereComponent.h"
#include "Components/SceneComponent.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AProjetProgFinalCharacter

AProjetProgFinalCharacter::AProjetProgFinalCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	   
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...  
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	
	// Longueur de la "perche" de la caméra
	CameraBoom->TargetArmLength = 1000.0f; 
	
	// On fixe la rotation du boom pour qu'il regarde d'en haut (ex: -70 degrés)
	CameraBoom->SetRelativeRotation(FRotator(-70.0f, 0.0f, 0.0f));

	// Désactive la rotation du boom par la souris/contrôleur
	CameraBoom->bUsePawnControlRotation = false; 

	// S'assure que le boom ne tourne pas bizarrement si le personnage s'incline
	CameraBoom->bInheritPitch = false;
	CameraBoom->bInheritYaw = false;
	CameraBoom->bInheritRoll = false;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	
	// La caméra elle-même ne doit pas tourner par rapport au boom
	FollowCamera->bUsePawnControlRotation = false; 


	// --- Valeurs de base du character ---
	MaxHealth = 100;
	CurrentHealth = 100;
	MovementSpeed = 500.f;
	HealthRegenAmount = 0.0f;
	GlobalDamageMultiplier = 1.0f;
	bIsInvincible = false;

	CurrentEXP = 0.0f;
	EXPToNextLevel = 100.f;
	CurrentPlayerLevel = 1;	
}

//////////////////////////////////////////////////////////////////////////

void AProjetProgFinalCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (StartingWeaponClass && StartingWeaponData)
	{
		// Crée l'arme si elle existe
		AddWeapon(StartingWeaponClass, StartingWeaponData);
	}

	GetWorldTimerManager().SetTimer(RegenTimerHandle, this, &AProjetProgFinalCharacter::TriggerHealthRegen, 1.0f, true); // Tout les 1 secondes
}

// Input
void AProjetProgFinalCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void AProjetProgFinalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AProjetProgFinalCharacter::Move);
	}
}

void AProjetProgFinalCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AProjetProgFinalCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AProjetProgFinalCharacter::TriggerHealthRegen()
{
	if (HealthRegenAmount > 0 && CurrentHealth < MaxHealth)
	{
		CurrentHealth += HealthRegenAmount;

		if (CurrentHealth > MaxHealth)
		{
			CurrentHealth = MaxHealth;
		}

		UpdateHealthUI();
	}
}

void AProjetProgFinalCharacter::AddEXP(float Amount)
{
	// Si on choisit deja l'upgrade, prend l'exp mais ne level up pas
	if (bIsChoosingUpgrade)
	{
		CurrentEXP += Amount;
		return;
	}

	CurrentEXP += Amount;
	bool bDidLevelUp = false;

	// Level up
	while (CurrentEXP >= EXPToNextLevel)
	{
		bDidLevelUp = true;
		bIsChoosingUpgrade = true;

		UpdateEXP_UI(0.0f, EXPToNextLevel * 1.2f, CurrentPlayerLevel + 1);
		ShowLevelUpScreen();

		break;
	}

	// Augmente l'EXP
	if (!bDidLevelUp)
	{
		UpdateEXP_UI(CurrentEXP, EXPToNextLevel, CurrentPlayerLevel);
	}
}

TArray<UDA_UpgradeBase*> AProjetProgFinalCharacter::GetUpgradeOptions(int32 NumOptions)
{
	// Variable locale
	TArray<UDA_UpgradeBase*> ValidOptions;

	// Vérifie chacun des upgrades
	for (UDA_UpgradeBase* Upgrade : AllAvailableUpgrades)
	{
		if (!Upgrade) continue;

		// Est-ce que je possede deja l'arme?
		bool bHasRequirement = true;

		if (Upgrade->WeaponToUpgrade)
		{
			bool bPlayerHasWeapon = false;
			// DEBUG [1]
			FString WeaponName = Upgrade->WeaponToUpgrade->GetName();
			// Verifie si on possede deja l'arme
			for (AWeaponBase* W : ActiveWeapons)
			{
				// DEBUG 2 : On vérifie ce qu'on a
				if (W && W->GetSourceDataAsset()->GetName() == WeaponName)
				{
					bPlayerHasWeapon = true;
					break;
				}
			}
			// Si on a deja l'arme
			if (!bPlayerHasWeapon)
			{
				bool bIsUnlockCard = (Upgrade->WeaponClassToSpawn != nullptr);

				if (bIsUnlockCard == false)
				{
					// C'est une stat, et je n'ai pas l'arme -> CACHER
					bHasRequirement = false;
				}
			}
		}

		if (!bHasRequirement) continue;

		// Check dans les upgrades deja prises
		const int32 CurrentUpgradeLevel = OwnedUpgrades.FindRef(Upgrade);
		// S'il est pas deja trouve ou pas niveau max
		if (CurrentUpgradeLevel < Upgrade->MaxLevel)
		{
			// Ajout aux options valides
			ValidOptions.Add(Upgrade);
		}
	}

	// --- MÉLANGE ALÉATOIRE ---
	TArray<UDA_UpgradeBase*> FinalOptions;

	for (int32 i = 0; i < NumOptions; ++i)
	{
		if (ValidOptions.Num() == 0) break;

		// Calcule la somme totale des poids des options restantes
		float TotalWeight = 0.0f;
		for (UDA_UpgradeBase* Option : ValidOptions)
		{
			TotalWeight += Option->ProbabilityWeight;
		}

		// Tire un nombre aleatoire dans cette somme
		float RandomValue = FMath::FRandRange(0.0f, TotalWeight);

		// Trouver qui gagne
		float CurrentSum = 0.0f;
		UDA_UpgradeBase* SelectedUpgrade = nullptr;
		int32 SelectedIndex = -1;

		for (int32 j = 0; j < ValidOptions.Num(); ++j)
		{
			CurrentSum += ValidOptions[j]->ProbabilityWeight;
			if (RandomValue <= CurrentSum)
			{
				SelectedUpgrade = ValidOptions[j];
				SelectedIndex = j;
				break;
			}
		}

		// Ajoute et retire du pool
		if (SelectedUpgrade)
		{
			FinalOptions.Add(SelectedUpgrade);
			ValidOptions.RemoveAt(SelectedIndex); // On ne peut pas la repiocher
		}
	}

	return FinalOptions;
}

void AProjetProgFinalCharacter::ApplyUpgrade(UDA_UpgradeBase* ChosenUpgrade)
{
	if (!ChosenUpgrade) return;

	// Met à jour le niveau
	const int32 CurrentUpgradeLevel = OwnedUpgrades.FindRef(ChosenUpgrade);
	const int32 NewUpgradeLevel = CurrentUpgradeLevel + 1; // Augmente de niveau
	OwnedUpgrades.Add(ChosenUpgrade, NewUpgradeLevel); // Met à jour la map

	// Gére la logique de Level Up
	CurrentPlayerLevel++;
	CurrentEXP -= EXPToNextLevel;
	EXPToNextLevel *= 1.2; // Multiplicateur pour l'EXP à avoir pour level up

	// Trouve l'arme cible
	AWeaponBase* TargetWeapon = nullptr;
	if (ChosenUpgrade->WeaponToUpgrade)
	{
		for (AWeaponBase* Weapon : ActiveWeapons)
		{
			// On compare les pointeurs de DataAsset pour identifier l'arme
			if (Weapon->GetSourceDataAsset() == ChosenUpgrade->WeaponToUpgrade)
			{
				TargetWeapon = Weapon;
				break; // Trouvé !
			}
		}
	}

	// Si c'est une nouvelle arme
	if (NewUpgradeLevel == 1 && TargetWeapon == nullptr && ChosenUpgrade->WeaponClassToSpawn)
	{
		AddWeapon(ChosenUpgrade->WeaponClassToSpawn, ChosenUpgrade->WeaponToUpgrade);

		// On arrête ici pour le niveau 1 (ou on continue si on veut appliquer des stats bonus tout de suite)
		bIsChoosingUpgrade = false;
		AddEXP(0.0f);
		return;
	}

	// Applique les stats
	if (ChosenUpgrade->LevelDetails.IsValidIndex(NewUpgradeLevel - 1))
	{
		const FLevelUpData& LevelData = ChosenUpgrade->LevelDetails[NewUpgradeLevel - 1];

		// On cherche quels sont les valeurs à appliquer dans la map
		for (const TPair<EPlayerStatType, float>& StatPair : LevelData.StatsToApply)
		{
			EPlayerStatType Stat = StatPair.Key;
			float Value = StatPair.Value;       // La valeur trouvé dans la bonne KEY de la map

			// Update les valeurs dependant de la KEY dans StatsToApply
			switch (Stat)
			{
			case EPlayerStatType::Health:
				MaxHealth += (int32)Value;
				CurrentHealth += (int32)Value;
				if (CurrentHealth > MaxHealth) CurrentHealth = MaxHealth; // S'assure que la vie ne depasse pas le max
				break;

			case EPlayerStatType::Speed:
				GetCharacterMovement()->MaxWalkSpeed *= Value;
				break;

			case EPlayerStatType::HealthRegen:
				HealthRegenAmount += Value;
				break;

			case EPlayerStatType::WeaponDamage:
				if (TargetWeapon)
				{
					TargetWeapon->CurrentDamage *= Value;
				}
				break;

			case EPlayerStatType::WeaponCooldown:
				if (TargetWeapon)
				{
					TargetWeapon->CurrentCooldown *= Value;
					// Le timer se mettra à jour au prochain cycle d'attaque
				}
				break;

			case EPlayerStatType::WeaponArea:
				if (TargetWeapon)
				{
					TargetWeapon->CurrentHitboxScale *= Value;
				}
				break;

			case EPlayerStatType::GlobalDamage:
				GlobalDamageMultiplier *= Value;
				
				for (AWeaponBase* Weapon : ActiveWeapons)
				{
					if (Weapon)
					{
						Weapon->CurrentDamage *= Value;
					}
				}
				break;

			}
		}
	}

	bIsChoosingUpgrade = false;

	// Relance la vérification d'EXP 
	AddEXP(0.0f);
	UpdateEXP_UI(CurrentEXP, EXPToNextLevel, CurrentPlayerLevel);
}

void AProjetProgFinalCharacter::AddWeapon(TSubclassOf<AWeaponBase> WeaponClass, UPlayerDataAsset* InitData)
{
	if (!WeaponClass || !InitData) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	// On spawn l'arme
	AWeaponBase* NewWeapon = GetWorld()->SpawnActor<AWeaponBase>(WeaponClass, GetActorLocation(), FRotator::ZeroRotator, SpawnParams);

	if (NewWeapon)
	{
		NewWeapon->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
		NewWeapon->InitWeapon(InitData); // Ça lance le timer tout seul
		NewWeapon->CurrentDamage *= GlobalDamageMultiplier;
		ActiveWeapons.Add(NewWeapon);
	}
}

float AProjetProgFinalCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// Si invincible, refuse les dégats
	if (bIsInvincible)
	{
		return 0.0f;
	}

	// Appel au parent
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	// Appliquer les dégats
	int32 DamageInt = FMath::RoundToInt(ActualDamage);
	CurrentHealth -= DamageInt;

	// Debug pour voir si ça marche
	UE_LOG(LogTemp, Warning, TEXT("OUCH! Pris %d degats. PV restants : %d"), DamageInt, CurrentHealth);

	// Vérifie s'il est mort
	if (CurrentHealth <= 0)
	{
		CurrentHealth = 0;
		// GameOver()
		
	}
	else
	{
		// Active l'invicibilité
		bIsInvincible = true;

		// Lance le timer de 2 secondes
		GetWorldTimerManager().SetTimer(InvincibilityTimerHandle, this, &AProjetProgFinalCharacter::EndInvincibility, 2.0f, false);

		// Prévient le Blueprint
		OnInvincibilityChanged(true);
	}

	// Update Health UI
	UpdateHealthUI();

	return ActualDamage;
}

void AProjetProgFinalCharacter::EndInvincibility()
{
	// Le temps est écoulé, on redevient vulnérable
	bIsInvincible = false;

	// Prévenir le Blueprint (Arrêter le clignotement)
	OnInvincibilityChanged(false);
}