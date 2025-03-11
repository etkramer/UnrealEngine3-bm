// BM1
class ApexDestructibleAsset extends ApexAsset
    native;

var native Pointer ApexDestructibleAsset;
var() const editfixedsize array<MaterialInterface> Materials;
var() const array<Vector> CookingScales;
var() editconst ApexMeshParticleFactoryAsset CrumbleMeshParticleFactory;
var() editconst ApexMeshParticleFactoryAsset DustMeshParticleFactory;
var() editfixedsize array<PhysicalMaterial> FractureEffects;