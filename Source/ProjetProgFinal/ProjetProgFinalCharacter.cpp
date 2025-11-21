// Copyright Epic Games, Inc. All RightsC reserved.

#include "ProjetProgFinalCharacter.h"
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

	// --- Valeurs de base du character ---
	MaxHealth = 120;
	CurrentHealth = 120;
	MovementSpeed = 500.f;
	BaseDamage = 10.f;

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

		// --- MODIFICATION : On désactive l'input de la souris pour la caméra ---
		// Looking
		// EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AProjetProgFinalCharacter::Look);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
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

		// Check dans les upgrades déjà prises
		const int32 CurrentUpgradeLevel = OwnedUpgrades.FindRef(Upgrade);

		// S'il est pas déjà trouvé ou pas niveau max
		if (CurrentUpgradeLevel < Upgrade->MaxLevel)
		{
			// Ajout aux options valides
			ValidOptions.Add(Upgrade);
		}
	}

	// --- MÉLANGE ALÉATOIRE ---
	TArray<UDA_UpgradeBase*> FinalOptions;

	// Copie de nos options valides pour pouvoir les modifie
	TArray<UDA_UpgradeBase*> TempOptions = ValidOptions;

	// Pour ne pas avoir plus d'options qu'il y en a
	int32 NumToPick = FMath::Min(NumOptions, TempOptions.Num());

	// Pige le nombre d'options désiré
	for (int32 i = 0; i < NumToPick; ++i)
	{
		// On pige un index aléatoire
		int32 RandIndex = FMath::RandRange(0, TempOptions.Num() - 1);

		// On ajoute l'option à l'array final
		FinalOptions.Add(TempOptions[RandIndex]);

		// On retire l'option de l'array temporaire pour ne pas la reprendre
		TempOptions.RemoveAt(RandIndex);
	}

	// On le retourne
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
				MaxHealth += Value;
				CurrentHealth += Value;

				// S'assure que la vie ne dépasse pas le max
				if (CurrentHealth > MaxHealth)
				{
					CurrentHealth = MaxHealth;
				}
				break;

			case EPlayerStatType::Speed:
				GetCharacterMovement()->MaxWalkSpeed *= Value;
				break;

			case EPlayerStatType::Damage:
				BaseDamage *= Value;
				break;
			}
		}
	}

	// Relance la vérification d'EXP 
	AddEXP(0.0f);

	// Mettre à jour le UI
	UpdateEXP_UI(CurrentEXP, EXPToNextLevel, CurrentPlayerLevel);
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