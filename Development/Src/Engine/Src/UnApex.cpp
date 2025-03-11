/*=============================================================================
	UnApex.cpp: Implementations for BM1 Apex classes
=============================================================================*/

#include "EnginePrivate.h"

IMPLEMENT_CLASS(UApexAsset);
IMPLEMENT_CLASS(UApexComponentBase);
IMPLEMENT_CLASS(AApexDestructibleActor);
IMPLEMENT_CLASS(UApexDestructibleAsset);
IMPLEMENT_CLASS(UApexDynamicComponent);
IMPLEMENT_CLASS(UApexMeshParticleFactoryAsset);
IMPLEMENT_CLASS(UApexParticleSystemAsset);
IMPLEMENT_CLASS(UApexRenderMeshAsset);
IMPLEMENT_CLASS(UApexStaticComponent);
IMPLEMENT_CLASS(UApexStaticDestructibleComponent);

/*-----------------------------------------------------------------------------
	UApexDestructibleActor implementation
-----------------------------------------------------------------------------*/

void AApexDestructibleActor::CacheFractureEffects()
{
    // TODO: Implement
}

void AApexDestructibleActor::TakeDamage( INT Damage, AController * EventInstigator, FVector HitLocation, FVector Momentum, UClass * DamageType, FTraceHitInfo HitInfo, AActor * DamageCauser )
{
    // TODO: Implement
}

void AApexDestructibleActor::TakeRadiusDamage( AController* InstigatedBy, FLOAT BaseDamage, FLOAT DamageRadius, UClass* DamageType, FLOAT Momentum, FVector HurtOrigin, UBOOL bFullDamage, AActor* DamageCauser )
{
    // TODO: Implement
}