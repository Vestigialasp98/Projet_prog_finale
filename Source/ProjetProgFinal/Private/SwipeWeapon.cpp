// Fill out your copyright notice in the Description page of Project Settings.


#include "SwipeWeapon.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "NiagaraFunctionLibrary.h"

void ASwipeWeapon::Attack()
{
    // Get player
    AActor* MyOwner = GetOwner();
    if (!MyOwner) return;
    
    // Defaut, devant le joueur
    FRotator AttackRotation = MyOwner->GetActorRotation();

    // On cherche un ennemi
    AActor* Target = FindClosestEnemy(MyOwner->GetActorLocation());

    if (Target)
    {
        // Si on en trouve un, on calcule la rotation vers lui
        // FindLookAtRotation calcule l'angle necessaire pour regarder
        AttackRotation = UKismetMathLibrary::FindLookAtRotation(
            MyOwner->GetActorLocation(),
            Target->GetActorLocation()
        );

        // On garde l'attaque à plat
        AttackRotation.Pitch = 0.0f;
        AttackRotation.Roll = 0.0f;
    }


    // Calcule la position en utilisant la nouvelle rotation 
    // la direction devant de l'attaque
    FVector SpawnLocation = MyOwner->GetActorLocation() + AttackRotation.Vector() * 150.f;
    FRotator SpawnRotation = AttackRotation;

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

    // Play attack sound
    if (AttackSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, AttackSound, GetActorLocation());
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

AActor* ASwipeWeapon::FindClosestEnemy(const FVector& Origin)
{
    // Quels types d'objets on cherche
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));

    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(GetOwner());

    // Ignore le joueur
    TArray<AActor*> OverlappedActors; 
    UKismetSystemLibrary::SphereOverlapActors(
        GetWorld(),
        Origin,
        AutoAimRadius,
        ObjectTypes,
        AActor::StaticClass(), // On filtre grossièrement sur Actor
        ActorsToIgnore,
        OverlappedActors
    );

    // Trouver le plus proche avec le tag "Enemy"
    AActor* ClosestActor = nullptr;
    float MinDistanceSq = FLT_MAX; // Distance infinie au début

    for (AActor* Actor : OverlappedActors)
    {
        if (Actor && Actor->ActorHasTag("Enemy"))
        {
            float DistSq = FVector::DistSquared(Origin, Actor->GetActorLocation());
            if (DistSq < MinDistanceSq)
            {
                MinDistanceSq = DistSq;
                ClosestActor = Actor;
            }
        }
    }

    return ClosestActor;
}