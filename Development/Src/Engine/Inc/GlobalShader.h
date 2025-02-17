/*=============================================================================
	GlobalShader.h: Shader manager definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __GLOBALSHADER_H__
#define __GLOBALSHADER_H__

#include "ShaderManager.h"

/**
 * A shader meta type for the simplest shaders; shaders which are not material or vertex factory linked.
 * There should only a single instance of each simple shader type.
 */
class FGlobalShaderType : public FShaderType
{
public:

	typedef FShader::CompiledShaderInitializerType CompiledShaderInitializerType;
	typedef FShader* (*ConstructCompiledType)(const CompiledShaderInitializerType&);
	typedef UBOOL (*ShouldCacheType)(EShaderPlatform);

	FGlobalShaderType(
		const TCHAR* InName,
		const TCHAR* InSourceFilename,
		const TCHAR* InFunctionName,
		DWORD InFrequency,
		INT InMinPackageVersion,
		INT InMinLicenseePackageVersion,
		ConstructSerializedType InConstructSerializedRef,
		ConstructCompiledType InConstructCompiledRef,
		ModifyCompilationEnvironmentType InModifyCompilationEnvironmentRef,
		ShouldCacheType InShouldCacheRef
		):
		FShaderType(InName,InSourceFilename,InFunctionName,InFrequency,InMinPackageVersion,InMinLicenseePackageVersion,InConstructSerializedRef,InModifyCompilationEnvironmentRef),
		ConstructCompiledRef(InConstructCompiledRef),
		ShouldCacheRef(InShouldCacheRef)
	{}

	/**
	 * Compiles a shader of this type.  After compiling the shader, either returns an equivalent existing shader of this type, or constructs
	 * a new instance.
	 * @param Compiler - The shader compiler to use.
	 * @param OutErrors - Upon compilation failure, OutErrors contains a list of the errors which occured.
	 * @param bDebugDump - Dump out the preprocessed and disassembled shader for debugging.
	 * @param ShaderSubDir - Sub directory for dumping out preprocessor output.
	 * @return NULL if the compilation failed.
	 */
	FShader* CompileShader(EShaderPlatform Platform,TArray<FString>& OutErrors, UBOOL bDebugDump = FALSE);

	/**
	 * Checks if the shader type should be cached for a particular platform.
	 * @param Platform - The platform to check.
	 * @return True if this shader type should be cached.
	 */
	UBOOL ShouldCache(EShaderPlatform Platform) const
	{
		return (*ShouldCacheRef)(Platform);
	}

	/**
	* Calculates a CRC based on this shader type's source code and includes
	*/
	virtual DWORD GetSourceCRC();

	// Dynamic casting.
	virtual FGlobalShaderType* GetGlobalShaderType() { return this; }

private:
	ConstructCompiledType ConstructCompiledRef;
	ShouldCacheType ShouldCacheRef;
};

/**
 * FGlobalShader
 * 
 * Global shaders derive from this class to set their default recompile group as a global one
 */
class FGlobalShader : public FShader
{
	DECLARE_SHADER_TYPE(FGlobalShader,Global);
public:
	FGlobalShader() : FShader() {}

	FGlobalShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer) : FShader(Initializer) {}

	virtual EShaderRecompileGroup GetRecompileGroup() const
	{
		return SRG_GLOBAL_MISC;
	}
};

/**
 * Vertex shader for rendering a single, constant color.
 */
class FOneColorVertexShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FOneColorVertexShader,Global);
public:
	FOneColorVertexShader( )	{ }
	FOneColorVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
	:	FGlobalShader( Initializer )
	{
	}

	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return TRUE;
	}
};

/**
 * Pixel shader for rendering a single, constant color.
 */
class FOneColorPixelShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FOneColorPixelShader,Global);
public:
	FOneColorPixelShader( )	{ }
	FOneColorPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
	:	FGlobalShader( Initializer )
	{
		ColorParameter.Bind( Initializer.ParameterMap, TEXT("DrawColor") );
	}

	// FShader interface.
	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FGlobalShader::Serialize(Ar);
		Ar << ColorParameter;
		return bShaderHasOutdatedParameters;
	}
	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return TRUE;
	}

	/** The parameter to use for setting the draw Color. */
	FShaderParameter ColorParameter;
};

/**
 * An internal dummy pixel shader to use when the user calls RHISetPixelShader(NULL).
 */
class FNULLPixelShader : public FShader
{
	DECLARE_SHADER_TYPE(FNULLPixelShader,Global);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return TRUE;
	}

	FNULLPixelShader( )	{ }
	FNULLPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FShader(Initializer)
	{
	}
};

/** Serializes the global shader map using the specified archive. */
extern void SerializeGlobalShaders(EShaderPlatform Platform,FArchive& Ar);

/**
 * Makes sure all global shaders are loaded and/or compiled for the passed in platform.
 *
 * @param	Platform	Platform to verify global shaders for
 */
extern void VerifyGlobalShaders(EShaderPlatform Platform=GRHIShaderPlatform);

/**
 * Accesses the global shader map.  This is a global TShaderMap<FGlobalShaderType> which contains an instance of each global shader type.
 *
 * @param Platform Which platform's global shader map to use
 * @return A reference to the global shader map.
 */
extern TShaderMap<FGlobalShaderType>* GetGlobalShaderMap(EShaderPlatform Platform=GRHIShaderPlatform);

/**
 * Forces a recompile of the global shaders.
 */
extern void RecompileGlobalShaders();

/**
* Recompiles the specified global shader types, and flushes their bound shader states.
*/
extern void RecompileGlobalShaders(const TArray<FShaderType*>& OutdatedShaderTypes);

/**
* Forces a recompile of only the specified group of global shaders. 
* Also invalidates global bound shader states so that the new shaders will be used.
*/
extern void RecompileGlobalShaderGroup(EShaderRecompileGroup FlushGroup);

#endif
