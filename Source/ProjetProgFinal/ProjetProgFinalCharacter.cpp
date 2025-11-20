// Copyright Epic Games, Inc. All RightsC reserved.

#include "ProjetProgFinalCharacter.h"
#include <AttackBox.h>
#include  "PlayerDataAsset.h"
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

DEFINE_LOG_CATEGORY(LogTemplateCharacter);
DECLARE_LOG_CATEGORY_EXTERN(LogPlayerAttack, Log, All);
DEFINE_LOG_CATEGORY(LogPlayerAttack);


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

	// --- Valeurs de base du character ---
	MaxHealth = 120;
	CurrentHealth = 120;
	MovementSpeed = 500.f;

	CurrentEXP = 0.0f;
	EXPToNextLevel = 100.f;
	CurrentPlayerLevel = 1;
	bIsChoosingUpgrade = false;
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

void AProjetProgFinalCharacter::AddEXP(float Amount)
{
	if (bIsChoosingUpgrade)
	{
		CurrentEXP += Amount;
		return;
	}

	CurrentEXP += Amount;
	bool bDidLevelUp = false;

	while (CurrentEXP >= EXPToNextLevel)
	{
		bDidLevelUp = true;
		bIsChoosingUpgrade = true;

		float OldEXPToNextLevel = EXPToNextLevel;

		// Gerer la logique de Level Up
		CurrentPlayerLevel++;
		EXPToNextLevel *= 1.2; // Multiplicateur pour d'EXP avoir pour level up

		UpdateEXP_UI(0.0f, EXPToNextLevel, CurrentPlayerLevel);
		ShowLevelUpScreen();

		CurrentEXP -= OldEXPToNextLevel;

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

	// Verifie chacun des upgrades
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

	// --- MELANGE ALEATOIRE ---
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

	// Mettre a jour le niveau
	const int32 CurrentUpgradeLevel = OwnedUpgrades.FindRef(ChosenUpgrade);
	const int32 NewUpgradeLevel = CurrentUpgradeLevel + 1; // Augmente de niveau
	OwnedUpgrades.Add(ChosenUpgrade, NewUpgradeLevel); // Met a jour la map

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

		// On cherche quels sont les valeurs a appliquer dans la map
		for (const TPair<EPlayerStatType, float>& StatPair : LevelData.StatsToApply)
		{
			EPlayerStatType Stat = StatPair.Key;
			float Value = StatPair.Value;       // La valeur trouve dans la bonne KEY de la map

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
			}
		}
	}

	bIsChoosingUpgrade = false;

	// Relance la verification d'EXP 
	AddEXP(0.0f);
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
		ActiveWeapons.Add(NewWeapon);
	}
}

