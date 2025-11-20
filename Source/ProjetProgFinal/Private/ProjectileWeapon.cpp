// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"

void AProjectileWeapon::Attack()
{
    AActor* MyOwner = GetOwner();
    if (!MyOwner || !ProjectileClass) return;

    // Defaut: Spawn à partir du joueur avec sa rotation
    FVector SpawnLocation = MyOwner->GetActorLocation() + FVector(0.f, 0.f, 0.f);
    FRotator AttackRotation = MyOwner->GetActorRotation();

    AActor* Target = FindClosestEnemy(MyOwner->GetActorLocation());

    if (Target)
    {
        // Calcule la rotation pour regarder vers l'ennemi
        AttackRotation = UKismetMathLibrary::FindLookAtRotation(
            SpawnLocation,
            Target->GetActorLocation()
        );
        // Garder le tir a l'horizontale
        AttackRotation.Pitch = 0.0f;
    }
    else
    {
        FVector End = SpawnLocation + (AttackRotation.Vector() * 500.f);
    }

    // Spawn projectile
    FTransform SpawnTransform(AttackRotation, SpawnLocation);

    AProjectile* Bullet = GetWorld()->SpawnActorDeferred<AProjectile>(
        ProjectileClass,
        SpawnTransform,
        MyOwner,
        MyOwner->GetInstigator(),
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn
    );

    // Play attack sound
    if (AttackSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, AttackSound, GetActorLocation());
    }

    if (Bullet)
    {
        // (CurrentRange vient de WeaponBase si tu l'as ajouté, sinon utilise une valeur par défaut)
        float Range = (CurrentDuration > 0) ? (CurrentDuration * ProjectileSpeed) : 1000.0f; // Hack si tu n'as pas de variable Range

        Bullet->SetupProjectile(CurrentDamage, ProjectileSpeed, Range);

        // Lancer la balle !
        UGameplayStatics::FinishSpawningActor(Bullet, SpawnTransform);
    }
}

AActor* AProjectileWeapon::FindClosestEnemy(const FVector& Origin)
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