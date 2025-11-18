// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include <AttackBox.h>
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include  "PlayerDataAsset.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "PlayerStatType.h" // Enum des types de stats
#include "DA_UpgradeBase.h" // DataAsset de base pour les upgrades
#include "ProjetProgFinalCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UDA_UpgradeBase; // Idem pour DataAsset en class
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class AProjetProgFinalCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

public:
	AProjetProgFinalCharacter();

	// Setup the AttackBox
	UPROPERTY(EditAnywhere, BlueprintReadOnly ,Category="AttackBox")
	TSubclassOf<AAttackBox> AttackBoxClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AttackBox")
	UPlayerDataAsset* AttackBoxData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	UNiagaraSystem* SlashVFX;

	FTimerHandle AttackLoopHandle;
	

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
			

protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void NotifyControllerChanged() override;

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// End of APawn interface

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

protected:
	// --- STATS DU CHARACTER ---

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	int32 MaxHealth;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	int32 CurrentHealth;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	float MovementSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	float BaseDamage;


	// --- SYSTEME D'EXP ---

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats|XP")
	float CurrentEXP;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats|XP")
	float EXPToNextLevel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats|XP")
	int32 CurrentPlayerLevel;

	bool bIsChoosingUpgrade;

	// --- SYSTEME D'UPGRADE ---

	// La liste des upgrades que le character � d�j�
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Upgrades")
	TMap<UDA_UpgradeBase*, int32> OwnedUpgrades;

	// La liste de tous les upgrades possibles (Remplie dans le BP)
	UPROPERTY(EditDefaultsOnly, Category = "Upgrades")
	TArray<UDA_UpgradeBase*> AllAvailableUpgrades;

	// --- EVENEMENTS BLUEPRINT ---

	// Affiche le widget de level up
	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void ShowLevelUpScreen();

	// Update l'EXP actuel
	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void UpdateEXP_UI(float NewXP, float NewMaxXP, int32 NewLvl);

public:
	// --- STATS DU CHARCACTER POUR L'ATTAQUE ---
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UPlayerDataAsset> PlayerInfo;


	// --- FONCTIONS PUBLIQUES ---

	// Ajout d'EXP
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void AddEXP(float Amount);

	// Filtre et renvoie les options d'upgrades valides
	UFUNCTION(BlueprintCallable, Category = "Upgrades")
	TArray<UDA_UpgradeBase*> GetUpgradeOptions(int32 NumOptions);

	// Applique l'upgrade choisie au character
	UFUNCTION(BlueprintCallable, Category = "Upgrades")
	void ApplyUpgrade(UDA_UpgradeBase* ChosenUpgrade);

	//Fonction d'attaque du player
	void Attack();
	void StartAttacking();

};

