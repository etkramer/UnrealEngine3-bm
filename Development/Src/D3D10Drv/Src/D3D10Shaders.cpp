/*=============================================================================
	D3D10Shaders.cpp: D3D shader RHI implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "D3D10DrvPrivate.h"

FVertexShaderRHIRef FD3D10DynamicRHI::CreateVertexShader(const TArray<BYTE>& Code)
{
	TRefCountPtr<ID3D10VertexShader> VertexShader;
	VERIFYD3D10RESULT(Direct3DDevice->CreateVertexShader((DWORD*)&Code(0),Code.Num(),VertexShader.GetInitReference()));
	return new FD3D10VertexShader(VertexShader,Code);
}

FPixelShaderRHIRef FD3D10DynamicRHI::CreatePixelShader(const TArray<BYTE>& Code)
{
	TRefCountPtr<FD3D10PixelShader> PixelShader;
	VERIFYD3D10RESULT(Direct3DDevice->CreatePixelShader((DWORD*)&Code(0),Code.Num(),(ID3D10PixelShader**)PixelShader.GetInitReference()));
	return PixelShader.GetReference();
}

FD3D10BoundShaderState::FD3D10BoundShaderState(
	FVertexDeclarationRHIParamRef InVertexDeclarationRHI,
	DWORD* InStreamStrides,
	FVertexShaderRHIParamRef InVertexShaderRHI,
	FPixelShaderRHIParamRef InPixelShaderRHI,
	ID3D10Device* Direct3DDevice
	):
	CacheLink(InVertexDeclarationRHI,InStreamStrides,InVertexShaderRHI,InPixelShaderRHI,this)
{
	DYNAMIC_CAST_D3D10RESOURCE(VertexDeclaration,InVertexDeclaration);
	DYNAMIC_CAST_D3D10RESOURCE(VertexShader,InVertexShader);
	DYNAMIC_CAST_D3D10RESOURCE(PixelShader,InPixelShader);

	// Create an input layout for this combination of vertex declaration and vertex shader.
	VERIFYD3D10RESULT(Direct3DDevice->CreateInputLayout(
		&InVertexDeclaration->VertexElements(0),
		InVertexDeclaration->VertexElements.Num(),
		&InVertexShader->Code(0),
		InVertexShader->Code.Num(),
		InputLayout.GetInitReference()
		));

	VertexShader = InVertexShader->Resource;
	PixelShader = InPixelShader;
}

/**
* Creates a bound shader state instance which encapsulates a decl, vertex shader, and pixel shader
* @param VertexDeclaration - existing vertex decl
* @param StreamStrides - optional stream strides
* @param VertexShader - existing vertex shader
* @param PixelShader - existing pixel shader
*/
FBoundShaderStateRHIRef FD3D10DynamicRHI::CreateBoundShaderState(
	FVertexDeclarationRHIParamRef VertexDeclarationRHI, 
	DWORD* StreamStrides,
	FVertexShaderRHIParamRef VertexShaderRHI, 
	FPixelShaderRHIParamRef PixelShaderRHI
	)
{
	SCOPE_CYCLE_COUNTER(STAT_D3D10CreateBoundShaderStateTime);

	checkf(GIsRHIInitialized && Direct3DDevice,(TEXT("Bound shader state RHI resource was created without initializing Direct3D first")));

	// Check for an existing bound shader state which matches the parameters
	FCachedBoundShaderStateLink* CachedBoundShaderStateLink = GetCachedBoundShaderState(
		VertexDeclarationRHI,
		StreamStrides,
		VertexShaderRHI,
		PixelShaderRHI
		);
	if(CachedBoundShaderStateLink)
	{
		// If we've already created a bound shader state with these parameters, reuse it.
		return CachedBoundShaderStateLink->BoundShaderState;
	}
	else
	{
		return new FD3D10BoundShaderState(VertexDeclarationRHI,StreamStrides,VertexShaderRHI,PixelShaderRHI,Direct3DDevice);
	}
}
