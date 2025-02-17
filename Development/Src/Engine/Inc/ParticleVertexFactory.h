/*=============================================================================
	ParticleVertexFactory.h: Particle vertex factory definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/**
 *	
 */
class FParticleVertexFactory : public FVertexFactory
{
	DECLARE_VERTEX_FACTORY_TYPE(FParticleVertexFactory);

public:
	// FRenderResource interface.
	virtual void InitRHI();

	/**
	 * Should we cache the material's shadertype on this platform with this vertex factory? 
	 */
	static UBOOL ShouldCache(EShaderPlatform Platform, const class FMaterial* Material, const class FShaderType* ShaderType);

	/**
	 * Can be overridden by FVertexFactory subclasses to modify their compile environment just before compilation occurs.
	 */
	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment);

	//
	void	SetScreenAlignment(BYTE InScreenAlignment)
	{
		ScreenAlignment = InScreenAlignment;
	}

	void	SetLockAxesFlag(BYTE InLockAxisFlag)
	{
		LockAxisFlag = InLockAxisFlag;
	}

	void	SetLockAxes(FVector& InLockAxisUp, FVector& InLockAxisRight)
	{
		LockAxisUp		= InLockAxisUp;
		LockAxisRight	= InLockAxisRight;
	}

	BYTE		GetScreenAlignment()				{	return ScreenAlignment;	}
	BYTE		GetLockAxisFlag()					{	return LockAxisFlag;	}
	FVector&	GetLockAxisUp()						{	return LockAxisUp;		}
	FVector&	GetLockAxisRight()					{	return LockAxisRight;	}

private:
	BYTE		ScreenAlignment;
	BYTE		LockAxisFlag;
	FVector		LockAxisUp;
	FVector		LockAxisRight;
};

/**
 *	
 */
class FParticleDynamicParameterVertexFactory : public FParticleVertexFactory
{
	DECLARE_VERTEX_FACTORY_TYPE(FParticleDynamicParameterVertexFactory);

public:
	// FRenderResource interface.
	virtual void InitRHI();

	/**
	 * Should we cache the material's shadertype on this platform with this vertex factory? 
	 */
	static UBOOL ShouldCache(EShaderPlatform Platform, const class FMaterial* Material, const class FShaderType* ShaderType);

	/**
	 * Can be overridden by FVertexFactory subclasses to modify their compile environment just before compilation occurs.
	 */
	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment);
};

/**
 *	
 */
class FParticleVertexFactoryShaderParameters : public FVertexFactoryShaderParameters
{
public:
	virtual void Bind(const FShaderParameterMap& ParameterMap)
	{
		CameraWorldPositionParameter.Bind(ParameterMap,TEXT("CameraWorldPosition"),TRUE);
		CameraRightParameter.Bind(ParameterMap,TEXT("CameraRight"),TRUE);
		CameraUpParameter.Bind(ParameterMap,TEXT("CameraUp"),TRUE);
		ScreenAlignmentParameter.Bind(ParameterMap,TEXT("ScreenAlignment"),TRUE);
		LocalToWorldParameter.Bind(ParameterMap,TEXT("LocalToWorld"));
		AxisRotationVectorSourceIndexParameter.Bind(ParameterMap, TEXT("AxisRotationVectorSourceIndex"));
		AxisRotationVectorsArrayParameter.Bind(ParameterMap, TEXT("AxisRotationVectors"));
		ParticleUpRightResultScalarsParameter.Bind(ParameterMap, TEXT("ParticleUpRightResultScalars"));
	}

	virtual void Serialize(FArchive& Ar)
	{
		Ar << CameraWorldPositionParameter;
		Ar << CameraRightParameter;
		Ar << CameraUpParameter;
		Ar << ScreenAlignmentParameter;
		Ar << LocalToWorldParameter;
		Ar << AxisRotationVectorSourceIndexParameter;
		Ar << AxisRotationVectorsArrayParameter;
		Ar << ParticleUpRightResultScalarsParameter;
	}

	virtual void Set(FShader* VertexShader,const FVertexFactory* VertexFactory,const FSceneView& View) const;

	virtual void SetMesh(FShader* VertexShader,const FMeshElement& Mesh,const FSceneView& View) const;

private:
	FShaderParameter CameraWorldPositionParameter;
	FShaderParameter CameraRightParameter;
	FShaderParameter CameraUpParameter;
	FShaderParameter ScreenAlignmentParameter;
	FShaderParameter LocalToWorldParameter;
	FShaderParameter AxisRotationVectorSourceIndexParameter;
	FShaderParameter AxisRotationVectorsArrayParameter;
	FShaderParameter ParticleUpRightResultScalarsParameter;
};
