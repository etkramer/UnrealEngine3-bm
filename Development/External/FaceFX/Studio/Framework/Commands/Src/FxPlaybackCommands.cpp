//------------------------------------------------------------------------------
// Animation playback related commands.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxPlaybackCommands.h"
#include "FxUtil.h"
#include "FxSessionProxy.h"
#include "FxVM.h"

namespace OC3Ent
{

namespace Face
{

//------------------------------------------------------------------------------
// Play command.
//------------------------------------------------------------------------------
FX_IMPLEMENT_CLASS(FxPlayCommand, 0, FxCommand);

FxPlayCommand::FxPlayCommand()
{
}

FxPlayCommand::~FxPlayCommand()
{
}

FxCommandSyntax FxPlayCommand::CreateSyntax( void )
{
	FxCommandSyntax newSyntax;
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-a", "-all", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-s", "-start", CAT_Real));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-e", "-end", CAT_Real));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-l", "-loop", CAT_Flag));
	return newSyntax;
}

FxCommandError FxPlayCommand::Execute( const FxCommandArgumentList& argList )
{
	FxBool loop = FxFalse;
	if( argList.GetArgument("-loop") )
	{
		loop = FxTrue;
	}

	if( argList.GetArgument("-all") )
	{
		FxString animGroup;
		FxString anim;
		FxAnim* pAnim = NULL;
		if( FxSessionProxy::GetSelectedAnimGroup(animGroup) )
		{
			if( FxSessionProxy::GetSelectedAnim(anim) )
			{
				if( FxSessionProxy::GetAnim(animGroup, anim, &pAnim) )
				{
					if( pAnim )
					{
						if( FxSessionProxy::SetSessionTime(pAnim->GetStartTime()) )
						{
							if( FxSessionProxy::PlayAnimation(FxInvalidValue, FxInvalidValue, loop) )
							{
								return CE_Success;
							}
						}
					}
				}
			}
		}
	}
	else
	{
		FxReal startTime = FxInvalidValue;
		FxReal endTime = FxInvalidValue;
		argList.GetArgument("-start", startTime);
		argList.GetArgument("-end", endTime);
		
		if( (FxInvalidValue == startTime && FxInvalidValue != endTime) ||
			(FxInvalidValue != startTime && FxInvalidValue == endTime) )
		{
			FxVM::DisplayError("-start and -end must be specified together!");
			return CE_Failure;
		}
		else
		{
			if( FxSessionProxy::PlayAnimation(startTime, endTime, loop) )
			{
				return CE_Success;
			}
		}
	}

	FxVM::DisplayError("internal error!");
	return CE_Failure;
}

//------------------------------------------------------------------------------
// Stop command.
//------------------------------------------------------------------------------
FX_IMPLEMENT_CLASS(FxStopCommand, 0, FxCommand);

FxStopCommand::FxStopCommand()
{
}

FxStopCommand::~FxStopCommand()
{
}

FxCommandSyntax FxStopCommand::CreateSyntax( void )
{
	FxCommandSyntax newSyntax;
	return newSyntax;
}

FxCommandError FxStopCommand::Execute( const FxCommandArgumentList& FxUnused(argList) )
{
	if( FxSessionProxy::StopAnimation() )
	{
		return CE_Success;
	}
	return CE_Failure;
}

} // namespace Face

} //namespace OC3Ent
