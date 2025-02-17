/*=============================================================================
	D3D9Query.cpp: D3D query RHI implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "D3D9DrvPrivate.h"

FOcclusionQueryRHIRef FD3D9DynamicRHI::CreateOcclusionQuery()
{
	TRefCountPtr<IDirect3DQuery9> OcclusionQuery;
	VERIFYD3D9RESULT(Direct3DDevice->CreateQuery(D3DQUERYTYPE_OCCLUSION,(IDirect3DQuery9**)OcclusionQuery.GetInitReference()));
	return new FD3D9OcclusionQuery(OcclusionQuery);
}

void FD3D9DynamicRHI::ResetOcclusionQuery(FOcclusionQueryRHIParamRef OcclusionQueryRHI)
{
	DYNAMIC_CAST_D3D9RESOURCE(OcclusionQuery,OcclusionQuery);

	OcclusionQuery->bResultIsCached = FALSE;
}

UBOOL FD3D9DynamicRHI::GetOcclusionQueryResult(FOcclusionQueryRHIParamRef OcclusionQueryRHI,DWORD& OutNumPixels,UBOOL bWait)
{
	DYNAMIC_CAST_D3D9RESOURCE(OcclusionQuery,OcclusionQuery);

	UBOOL bSuccess = TRUE;
	if(!OcclusionQuery->bResultIsCached)
	{
		bSuccess = GetQueryData(OcclusionQuery->Resource,&OcclusionQuery->Result,sizeof(OcclusionQuery->Result),bWait);
		OcclusionQuery->bResultIsCached = bSuccess;
	}

	OutNumPixels = OcclusionQuery->Result;

	return bSuccess;
}

UBOOL FD3D9DynamicRHI::GetQueryData(IDirect3DQuery9* Query,void* Data,SIZE_T DataSize,UBOOL bWait)
{
	if( !Query )
	{
		return FALSE;
	}

	// Request the data from the query.
	HRESULT Result = Query->GetData(Data,DataSize,D3DGETDATA_FLUSH);

	// Isn't the query finished yet, and can we wait for it?
	if ( Result == S_FALSE && bWait )
	{
		SCOPE_CYCLE_COUNTER( STAT_OcclusionResultTime );
		DWORD IdleStart = appCycles();
		DOUBLE StartTime = appSeconds();
		do 
		{
			Result = Query->GetData(Data,DataSize,D3DGETDATA_FLUSH);

			if((appSeconds() - StartTime) > 0.5)
			{
				debugf(TEXT("Timed out while waiting for GPU to catch up. (500 ms)"));
				return FALSE;
			}
		} while ( Result == S_FALSE );
		GRenderThreadIdle += appCycles() - IdleStart;
	}

	if ( SUCCEEDED( Result ) )
	{
		return TRUE;
	}
	else if ( Result == S_FALSE && !bWait )
	{
		// Return failure if the query isn't complete, and waiting wasn't requested.
		return FALSE;
	}
	else if ( Result == D3DERR_DEVICELOST )
	{
		bDeviceLost = 1;
		return FALSE;
	}
	else
	{
		VERIFYD3D9RESULT(Result);
		return FALSE;
	}
}

void FD3D9EventQuery::IssueEvent()
{
	if( Query )
	{
		Query->Issue(D3DISSUE_END);
	}
}

void FD3D9EventQuery::WaitForCompletion()
{
	if( Query )
	{
		UBOOL bRenderingIsFinished = FALSE;
		while(
			D3DRHI->GetQueryData(Query,&bRenderingIsFinished,sizeof(bRenderingIsFinished),TRUE) &&
			!bRenderingIsFinished
			)
		{};
	}
}

void FD3D9EventQuery::InitDynamicRHI()
{
	VERIFYD3D9RESULT(D3DRHI->GetDevice()->CreateQuery(D3DQUERYTYPE_EVENT,Query.GetInitReference()));
	// Initialize the query by issuing an initial event.
	IssueEvent();
}
void FD3D9EventQuery::ReleaseDynamicRHI()
{
	Query = NULL;
}
