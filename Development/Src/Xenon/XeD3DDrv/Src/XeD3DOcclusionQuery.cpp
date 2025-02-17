/*=============================================================================
	D3DQuery.cpp: D3D query RHI implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "XeD3DDrvPrivate.h"

#if USE_XeD3D_RHI

FOcclusionQueryRHIRef RHICreateOcclusionQuery()
{
	FOcclusionQueryRHIRef OcclusionQuery;
	VERIFYD3DRESULT(GDirect3DDevice->CreateQuery(D3DQUERYTYPE_OCCLUSION,OcclusionQuery.GetInitReference()));
	return OcclusionQuery;
}

void RHIResetOcclusionQuery(FOcclusionQueryRHIParamRef OcclusionQueryRHI)
{
}

UBOOL RHIGetOcclusionQueryResult(FOcclusionQueryRHIParamRef OcclusionQuery,DWORD& OutNumPixels,UBOOL bWait)
{
	// Query occlusion query object.
	HRESULT Result = OcclusionQuery->GetData( &OutNumPixels, sizeof(OutNumPixels), 0 );

	// Isn't the query finished yet, and can we wait for it?
	if ( bWait && (Result != S_OK) )
	{
		SCOPE_CYCLE_COUNTER( STAT_OcclusionResultTime );
		DWORD IdleStart = appCycles();
		do 
		{
			Result = OcclusionQuery->GetData( &OutNumPixels, sizeof(OutNumPixels), D3DGETDATA_FLUSH );
			// Assert if we try to fetch a query that has not been issued yet, which will cause an infinite loop.
			check(Result != D3DERR_NOTAVAILABLE);
		} while ( Result != S_OK );
		GRenderThreadIdle += appCycles() - IdleStart;
	}

	return (Result == S_OK);
}

#endif
