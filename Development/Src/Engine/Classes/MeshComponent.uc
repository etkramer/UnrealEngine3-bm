class MeshComponent extends PrimitiveComponent
    abstract
    native
    noexport;

var(Rendering) const array<MaterialInterface> Materials;

// Export UMeshComponent::execGetUseSimpleBoxCollision(FFrame&, void* const)
native function bool GetUseSimpleBoxCollision();

// Export UMeshComponent::execGetUseSimpleLineCollision(FFrame&, void* const)
native function bool GetUseSimpleLineCollision();

// Export UMeshComponent::execGetMaterial(FFrame&, void* const)
native function MaterialInterface GetMaterial(int ElementIndex);

// Export UMeshComponent::execSetMaterial(FFrame&, void* const)
native function SetMaterial(int ElementIndex, MaterialInterface Material);

// Export UMeshComponent::execGetNumElements(FFrame&, void* const)
native function int GetNumElements();

// Export UMeshComponent::execPrestreamTextures(FFrame&, void* const)
native final function PrestreamTextures(float Seconds, bool bPrioritizeCharacterTextures);

function MaterialInstanceConstant CreateAndSetMaterialInstanceConstant(int ElementIndex)
{
    local MaterialInstanceConstant Instance;

    Instance = new (Outer) Class'MaterialInstanceConstant';
    Instance.SetParent(GetMaterial(ElementIndex));
    SetMaterial(ElementIndex, Instance);
    return Instance;
    //return ReturnValue;    
}

function MaterialInstanceTimeVarying CreateAndSetMaterialInstanceTimeVarying(int ElementIndex)
{
    local MaterialInstanceTimeVarying Instance;

    Instance = new (Outer) Class'MaterialInstanceTimeVarying';
    Instance.SetParent(GetMaterial(ElementIndex));
    SetMaterial(ElementIndex, Instance);
    return Instance;
    //return ReturnValue;    
}

defaultproperties
{
    bUseAsOccluder=true
    CastShadow=true
    bAcceptsLights=true
    bCullModulatedShadowOnBackfaces=true
    bCullModulatedShadowOnEmissive=true
}
