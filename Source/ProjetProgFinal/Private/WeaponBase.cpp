// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBase.h"

// Sets default values
AWeaponBase::AWeaponBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

void AWeaponBase::InitWeapon(UPlayerDataAsset* WeaponData)
{
    if (!WeaponData) return;

    SourceDataAsset = WeaponData;

    CurrentDamage = WeaponData->DMG;

    // On copie les valeurs du DataAsset dans nos variables locales
    CurrentDamage = WeaponData->DMG;
    CurrentCooldown = WeaponData->Cooldown;
    CurrentHitboxScale = WeaponData->HitboxScale;
    CurrentDuration = WeaponData->HitboxDuration;

    // On lance la boucle d'attaque
    GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &AWeaponBase::Attack, CurrentCooldown, true, 0.0f);
}

// Called when the game starts or when spawned
void AWeaponBase::BeginPlay()
{	
}

// Called every frame
void AWeaponBase::Tick(float DeltaTime)
{
}

void AWeaponBase::Attack()
{

}
