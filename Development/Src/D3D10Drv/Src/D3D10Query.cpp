/*=============================================================================
	D3D10Query.cpp: D3D query RHI implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "D3D10DrvPrivate.h"

FOcclusionQueryRHIRef FD3D10DynamicRHI::CreateOcclusionQuery()
{
	TRefCountPtr<ID3D10Query> OcclusionQuery;
	D3D10_QUERY_DESC Desc;
	Desc.Query = D3D10_QUERY_OCCLUSION;
	Desc.MiscFlags = 0;
	VERIFYD3D10RESULT(Direct3DDevice->CreateQuery(&Desc,OcclusionQuery.GetInitReference()));
	return new FD3D10OcclusionQuery(OcclusionQuery);
}

void FD3D10DynamicRHI::ResetOcclusionQuery(FOcclusionQueryRHIParamRef OcclusionQueryRHI)
{
	DYNAMIC_CAST_D3D10RESOURCE(OcclusionQuery,OcclusionQuery);

	OcclusionQuery->bResultIsCached = FALSE;
}

UBOOL FD3D10DynamicRHI::GetOcclusionQueryResult(FOcclusionQueryRHIParamRef OcclusionQueryRHI,DWORD& OutNumPixels,UBOOL bWait)
{
	DYNAMIC_CAST_D3D10RESOURCE(OcclusionQuery,OcclusionQuery);

	UBOOL bSuccess = TRUE;
	if(!OcclusionQuery->bResultIsCached)
	{
		bSuccess = GetQueryData(OcclusionQuery->Resource,&OcclusionQuery->Result,sizeof(OcclusionQuery->Result),bWait);
		OcclusionQuery->bResultIsCached = bSuccess;
	}

	OutNumPixels = (DWORD)OcclusionQuery->Result;

	return bSuccess;
}

UBOOL FD3D10DynamicRHI::GetQueryData(ID3D10Query* Query,void* Data,SIZE_T DataSize,UBOOL bWait)
{
	// Request the data from the query.
	HRESULT Result = Query->GetData(Data,DataSize,0);

	// Isn't the query finished yet, and can we wait for it?
	if ( Result == S_FALSE && bWait )
	{
		SCOPE_CYCLE_COUNTER( STAT_OcclusionResultTime );
		DWORD IdleStart = appCycles();
		DOUBLE StartTime = appSeconds();
		do 
		{
			Result = Query->GetData(Data,DataSize,0);

			if((appSeconds() - StartTime) > 0.5)
			{
				debugf(TEXT("Timed out while waiting for GPU to catch up. (500 ms)"));
				return FALSE;
			}
		} while ( Result == S_FALSE );
		GRenderThreadIdle += appCycles() - IdleStart;
	}

	if( SUCCEEDED(Result) )
	{
		return TRUE;
	}
	else if(Result == S_FALSE && !bWait)
	{
		// Return failure if the query isn't complete, and waiting wasn't requested.
		return FALSE;
	}
	else if( Result == DXGI_ERROR_DEVICE_REMOVED || Result == DXGI_ERROR_DEVICE_RESET || Result == DXGI_ERROR_DRIVER_INTERNAL_ERROR )
	{
		bDeviceRemoved = TRUE;
		return FALSE;
	}
	else
	{
		VERIFYD3D10RESULT(Result);
		return FALSE;
	}
}

void FD3D10EventQuery::IssueEvent()
{
	Query->End();
}

void FD3D10EventQuery::WaitForCompletion()
{
	UBOOL bRenderingIsFinished = FALSE;
	while(
		D3DRHI->GetQueryData(Query,&bRenderingIsFinished,sizeof(bRenderingIsFinished),TRUE) &&
		!bRenderingIsFinished
		)
	{};
}

void FD3D10EventQuery::InitDynamicRHI()
{
	D3D10_QUERY_DESC QueryDesc;
	QueryDesc.Query = D3D10_QUERY_EVENT;
	QueryDesc.MiscFlags = 0;
	VERIFYD3D10RESULT(D3DRHI->GetDevice()->CreateQuery(&QueryDesc,Query.GetInitReference()));

	// Initialize the query by issuing an initial event.
	IssueEvent();
}
void FD3D10EventQuery::ReleaseDynamicRHI()
{
	Query = NULL;
}
