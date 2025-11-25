// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include <AttackBox.h>
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "PlayerDataAsset.h"
#include "WeaponBase.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "PlayerStatType.h" // Enum des types de stats
#include "DA_UpgradeBase.h" // DataAsset de base pour les upgrades
#include "Components/SphereComponent.h"
#include "ProjetProgFinalCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UDA_UpgradeBase; // Idem pour DataAsset en class
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class PROJETPROGFINAL_API AProjetProgFinalCharacter : public ACharacter
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
	
protected:
	virtual void BeginPlay();

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
			

protected:

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

	// Health Regen
	float HealthRegenAmount;
	FTimerHandle RegenTimerHandle;
	void TriggerHealthRegen();

	// --- SYSTEME D'EXP ---

	bool bIsChoosingUpgrade;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats|XP")
	float CurrentEXP;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats|XP")
	float EXPToNextLevel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats|XP")
	int32 CurrentPlayerLevel;

	// --- SYSTÈME D'UPGRADE ---

	// La liste des upgrades que le character à déjà
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

	// Update la health bar
	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void UpdateHealthUI();

	// Armes actives
	UPROPERTY(VisibleInstanceOnly, Category = "Combat")
	TArray<AWeaponBase*> ActiveWeapons;

	float GlobalDamageMultiplier;

	// Le timer pour arrêter l'invincibilité
	FTimerHandle InvincibilityTimerHandle;

	// Fonction appelée après 2 secondes
	void EndInvincibility();

	// Événement pour le Blueprint (pour faire clignoter le perso rouge/transparent)
	UFUNCTION(BlueprintImplementableEvent, Category = "Visuals")
	void OnInvincibilityChanged(bool bIsInvincibleNow);

	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void OnGameOver();

	// SOUNDS
	FTimerHandle GameOverTimerHandle;	

	void TriggerGameOver();

public:
	// --- STATS DU CHARACTER POUR L'ATTAQUE ---
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void AddWeapon(TSubclassOf<AWeaponBase> WeaponClass, UPlayerDataAsset* InitData);

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TSubclassOf<class AWeaponBase> StartingWeaponClass;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	UPlayerDataAsset* StartingWeaponData;

	// --- SYSTEME D'INVICIBILITE APRES HIT ---
	bool bIsInvincible = false;

	// --- SOUNDS ---
	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	USoundBase* HitSound;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	USoundBase* DeathSound;

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

	// --- Recoit des degats ---
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
};