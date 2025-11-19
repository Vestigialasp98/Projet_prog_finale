// Fill out your copyright notice in the Description page of Project Settings.


#include "SwipeWeapon.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"

void ASwipeWeapon::Attack()
{
    UE_LOG(LogTemp, Warning, TEXT("ATTACK LANCEE PAR : %s"), *GetName());
    // On récupère le joueur
    AActor* MyOwner = GetOwner();
    if (!MyOwner) return;

    // Logique de position (copiée de ton ancien Character)
    FVector SpawnLocation = MyOwner->GetActorLocation() + MyOwner->GetActorForwardVector() * 150.f;
    FRotator SpawnRotation = MyOwner->GetActorRotation();

    FRotator SpawnRotationVFX = MyOwner->GetActorRotation();
    SpawnRotationVFX.Yaw += 180.f;

    // Spawn VFX
    if (SlashVFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            SlashVFX,
            SpawnLocation,
            SpawnRotationVFX,
            CurrentHitboxScale - FVector(0.4f) // Utilise notre variable locale
        );
    }

    // Spawn Hitbox
    if (AttackBoxClass)
    {
        FTransform SpawnTransform(SpawnRotation, SpawnLocation);

        AAttackBox* HitBox = GetWorld()->SpawnActorDeferred<AAttackBox>(AttackBoxClass, SpawnTransform, GetOwner(), GetOwner()->GetInstigator(), ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
        if (HitBox)
        {
            // On configure la hitbox avec nos stats actuelles (qui peuvent avoir été upgradées)
            HitBox->SetupHitbox(CurrentHitboxScale, CurrentDuration, CurrentDamage);

            HitBox->SetOwner(GetOwner());
            HitBox->SetInstigator(GetOwner()->GetInstigator());

            UGameplayStatics::FinishSpawningActor(HitBox, SpawnTransform);
        }
    }

}