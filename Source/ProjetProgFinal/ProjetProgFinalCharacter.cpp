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
#include "Components/SceneComponent.h" // Assurez-vous que cet include est là

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

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// --- MODIFICATION POUR CAMÉRA STATIQUE ---

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	
	// Longueur de la "perche" de la caméra (distance par rapport au joueur)
	CameraBoom->TargetArmLength = 1000.0f; 
	
	// On fixe la rotation du boom pour qu'il regarde d'en haut (ex: -70 degrés)
	CameraBoom->SetRelativeRotation(FRotator(-70.0f, 0.0f, 0.0f));

	// On DÉSACTIVE la rotation du boom par la souris/contrôleur
	CameraBoom->bUsePawnControlRotation = false; 

	// On s'assure que le boom ne tourne pas bizarrement si le personnage s'incline
	CameraBoom->bInheritPitch = false;
	CameraBoom->bInheritYaw = false;
	CameraBoom->bInheritRoll = false;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	
	// La caméra elle-même ne doit pas tourner par rapport au boom
	FollowCamera->bUsePawnControlRotation = false; 

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	// --- Magnet range for exp pickup ---

	// --- Valeurs de base du character ---
	MaxHealth = 100;
	CurrentHealth = 120;
	MovementSpeed = 500.f;
	HealthRegenAmount = 0.0f;
	GlobalDamageMultiplier = 1.0f;

	CurrentEXP = 0.0f;
	EXPToNextLevel = 100.f;
	CurrentPlayerLevel = 1;

	// --- MODIFICATION : Création des 30 SceneComponents pour les spawn points ---
	
	// Créer les 15 points Small
	for (int32 i = 0; i < 15; ++i)
	{
		// Crée un nom unique comme "SpawnPoint_Small_0", "SpawnPoint_Small_1", etc.
		FName ComponentName = FName(TEXT("SpawnPoint_Small_%d"), i);
		USphereComponent* NewPoint = CreateDefaultSubobject<USphereComponent>(ComponentName);
		// --- CORRECTION : Attacher au RootComponent (la capsule) ---
		NewPoint->SetupAttachment(RootComponent);
		
		// Position initiale (ex: en cercle autour de la caméra)
		float Angle = (float)i / 15.0f * 360.0f;
		// Position sur le plan XY (Z=0 par rapport au joueur)
		FVector Location = FVector(FMath::Cos(Angle) * 1500.f, FMath::Sin(Angle) * 1500.f, 0.f);
		NewPoint->SetRelativeLocation(Location);

		// --- AJOUT : Configuration de la sphère ---
		NewPoint->InitSphereRadius(30.0f); // Taille de la sphère
		NewPoint->SetHiddenInGame(false); // On la rend visible en jeu (pour débogage)
		NewPoint->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Pas de collision
		NewPoint->ShapeColor = FColor::Green; // Couleur de la sphère (visible dans l'éditeur)
		// --- FIN AJOUT ---

		SmallSpawnPoints.Add(NewPoint);
	}

	// Créer les 10 points Medium
	for (int32 i = 0; i < 10; ++i)
	{
		FName ComponentName = FName(TEXT("SpawnPoint_Medium_%d"), i);
		USphereComponent* NewPoint = CreateDefaultSubobject<USphereComponent>(ComponentName);
		// --- CORRECTION : Attacher au RootComponent (la capsule) ---
		NewPoint->SetupAttachment(RootComponent);

		float Angle = (float)i / 10.0f * 360.0f;
		// Position sur le plan XY (Z=0 par rapport au joueur)
		FVector Location = FVector(FMath::Cos(Angle) * 2000.f, FMath::Sin(Angle) * 2000.f, 0.f);
		NewPoint->SetRelativeLocation(Location);

		// --- AJOUT : Configuration de la sphère ---
		NewPoint->InitSphereRadius(50.0f);
		NewPoint->SetHiddenInGame(false);
		NewPoint->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		NewPoint->ShapeColor = FColor::Yellow;
		// --- FIN AJOUT ---

		MediumSpawnPoints.Add(NewPoint);
	}

	// Créer les 5 points Large
	for (int32 i = 0; i < 5; ++i)
	{
		FName ComponentName = FName(TEXT("SpawnPoint_Large_%d"), i);
		USphereComponent* NewPoint = CreateDefaultSubobject<USphereComponent>(ComponentName);
		// --- CORRECTION : Attacher au RootComponent (la capsule) ---
		NewPoint->SetupAttachment(RootComponent);
		
		float Angle = (float)i / 5.0f * 360.0f;
		// Position sur le plan XY (Z=0 par rapport au joueur)
		FVector Location = FVector(FMath::Cos(Angle) * 2500.f, FMath::Sin(Angle) * 2500.f, 0.f);
		NewPoint->SetRelativeLocation(Location);

		// --- AJOUT : Configuration de la sphère ---
		NewPoint->InitSphereRadius(80.0f);
		NewPoint->SetHiddenInGame(false);
		NewPoint->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		NewPoint->ShapeColor = FColor::Red;
		// --- FIN AJOUT ---

		LargeSpawnPoints.Add(NewPoint);
	}
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

		// Looking
		// EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AProjetProgFinalCharacter::Look);
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
	// (Cette fonction n'est plus appelée, mais on la laisse au cas où)
	
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
	CurrentEXP += Amount;

	bool bDidLevelUp = false;

	while (CurrentEXP >= EXPToNextLevel)
	{
		bDidLevelUp = true;

		ShowLevelUpScreen();

		break;
	}

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

	// Mettre à jour le niveau
	const int32 CurrentUpgradeLevel = OwnedUpgrades.FindRef(ChosenUpgrade);
	const int32 NewUpgradeLevel = CurrentUpgradeLevel + 1; // Augmente de niveau
	OwnedUpgrades.Add(ChosenUpgrade, NewUpgradeLevel); // Met à jour la map

	// Gérer la logique de Level Up
	CurrentPlayerLevel++;
	CurrentEXP -= EXPToNextLevel;
	EXPToNextLevel *= 1.2; // Multiplicateur pour d'EXP à avoir pour level up

	// Trouver l'arme cible
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

	// Appliquer les stats
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

	// Relance la vérification d'EXP 
	AddEXP(0.0f);

	// Mettre à jour le UI
	UpdateEXP_UI(CurrentEXP, EXPToNextLevel, CurrentPlayerLevel);
}

void AProjetProgFinalCharacter::AddWeapon(TSubclassOf<AWeaponBase> WeaponClass, UPlayerDataAsset* InitData)
{
	if (!WeaponClass || !InitData) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	// On spawn l'arme (invisible, c'est juste un objet logique)
	AWeaponBase* NewWeapon = GetWorld()->SpawnActor<AWeaponBase>(WeaponClass, GetActorLocation(), FRotator::ZeroRotator, SpawnParams);

	if (NewWeapon)
	{
		NewWeapon->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
		NewWeapon->InitWeapon(InitData); // Ça lance le timer tout seul
		NewWeapon->CurrentDamage *= GlobalDamageMultiplier;
		ActiveWeapons.Add(NewWeapon);
	}
}
// --- MODIFICATION : Implémentation des nouvelles fonctions ---

FTransform AProjetProgFinalCharacter::GetSpawnTransformForPool(int32 PoolIndex, int32 SpawnPointIndex) const
{
	// --- CHNAGEMENT DE TYPE ---
	const TArray<TObjectPtr<USphereComponent>>* TargetArray = nullptr;
	// --- FIN CHNAGEMENT ---

	// L'ordre (0, 1, 2) est basé sur l'ordre dans le PoolManager
	switch (PoolIndex)
	{
	case 0:
		TargetArray = &SmallSpawnPoints;
		break;
	case 1:
		TargetArray = &MediumSpawnPoints;
		break;
	case 2:
		TargetArray = &LargeSpawnPoints;
		break;
	default:
		UE_LOG(LogTemp, Warning, TEXT("GetSpawnTransformForPool: Index de pool (%d) non valide !"), PoolIndex);
		return GetActorTransform();
	}

	// S'assurer que le tableau a des points et que l'index est valide
	if (TargetArray && TargetArray->IsValidIndex(SpawnPointIndex) && (*TargetArray)[SpawnPointIndex])
	{
		// Renvoyer le transform de ce point
		return (*TargetArray)[SpawnPointIndex]->GetComponentTransform();
	}

	// Fallback au cas où le tableau est vide ou l'index est mauvais
	UE_LOG(LogTemp, Warning, TEXT("GetSpawnTransformForPool: Le pool (%d) n'a pas pu trouver le point de spawn (%d) !"), PoolIndex, SpawnPointIndex);
	return GetActorTransform();
}

int32 AProjetProgFinalCharacter::GetSpawnPointCountForPool(int32 PoolIndex) const
{
	switch (PoolIndex)
	{
	case 0:
		return SmallSpawnPoints.Num();
	case 1:
		return MediumSpawnPoints.Num();
	case 2:
		return LargeSpawnPoints.Num();
	default:
		return 0;
	}
}
// --- FIN MODIFICATION ---