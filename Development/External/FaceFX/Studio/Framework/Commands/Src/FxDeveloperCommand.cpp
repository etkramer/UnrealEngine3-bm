//------------------------------------------------------------------------------
// The developer command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxDeveloperCommand.h"
#include "FxVM.h"
#include "FxSessionProxy.h"
#include "FxFaceGraphBaker.h"
#include "FxAnimSetManager.h"
#include "FxAudioFile.h"
//@todo This is included to get access to FxMsg() used in the -dumpbonepose and
//      -dumprefpose cases.  We might want to remove this later on if we 
//      actually implement the plug-in architecture.
#include "FxConsole.h"

namespace OC3Ent
{

namespace Face
{

FX_IMPLEMENT_CLASS(FxDeveloperCommand, 0, FxCommand);

FxDeveloperCommand::FxDeveloperCommand()
{
}

FxDeveloperCommand::~FxDeveloperCommand()
{
}

FxCommandSyntax FxDeveloperCommand::CreateSyntax( void )
{
	FxCommandSyntax newSyntax;
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-dwm", "-debugwidgetmesssages", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-dnt", "-dumpnametable", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-dct", "-dumpclasstable", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-dcc", "-dumpconsolecommands", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-dcv", "-dumpconsolevariables", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-dms", "-dumpmemorystats", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-dma", "-dumpmemoryallocs", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-crash", "-crash", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-scs", "-spamcommandsystem", CAT_Size));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-dbp", "-dumpbonepose", CAT_String));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-drp", "-dumprefpose", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-tb", "-testbaking", CAT_String));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-dmas", "-dumpmountedanimationsets", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-da", "-dumpanims", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-saf", "-saveaudiofile", CAT_String));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-about", "-about", CAT_Flag));
	return newSyntax;
}

FxCommandError FxDeveloperCommand::Execute( const FxCommandArgumentList& argList )
{
	if( argList.GetArgument("-debugwidgetmesssages") )
	{
		FxSessionProxy::ToggleWidgetMessageDebugging();
	}
	if( argList.GetArgument("-dumpnametable") )
	{
		FxVM::DisplayInfo("<b>----start name table----</b>");
		FxSize memSavings = 0;
		FxSize numNames = FxName::GetNumNames();
		for( FxSize i = 0; i < numNames; ++i )
		{
			FxString msg("(");
			msg << i << ") <b>" << FxName::GetNameAsString(i)->GetData() << "</b> <i>[" << FxName::GetNumReferences(i) << " refs]</i>";
			FxVM::DisplayInfo(msg);
			memSavings += ((FxName::GetNameAsString(i)->Length()+1)*sizeof(FxChar))*(FxName::GetNumReferences(i)-1);
		}
		memSavings -= numNames*sizeof(FxRefString*)*2;
		FxString msg("Approx. name table memory savings: <b>");
		msg << memSavings << " bytes.</b>";
		FxVM::DisplayInfo(msg);
		FxSize numCollisions   = 0;
		FxSize numNamesAdded   = 0;
		FxSize numProbes       = 0;
		FxSize numHashBins     = 0;
		FxSize numUsedHashBins = 0;
		FxSize memUsed         = 0;
		FxName::GetNameStats(numCollisions, numNamesAdded, numProbes, numHashBins, numUsedHashBins, memUsed);
		msg = "Name Hash: <b>";
		msg << numCollisions << "</b> peak collisions, <b>";
		msg << numNames << "</b> unique names, <b>";
		msg << numNamesAdded << "</b> peak unique names, <b>";
		msg << numProbes << "</b> peak probes, </b>";
		msg << numUsedHashBins << "</b> / <b>" << numHashBins << "</b> hash bins, ";
		msg << "mem used (in bytes: <b>" << memUsed << "</b>";
		FxVM::DisplayInfo(msg);
		FxVM::DisplayInfo("<b>----end name table----</b>");
	}
	if( argList.GetArgument("-dumpclasstable") )
	{
		FxVM::DisplayInfo("<b>----start class info----</b>");
		for( FxSize i = 0; i < FxClass::GetNumClasses(); ++i )
		{
			const FxClass* pClass = FxClass::GetClass(i);
			if( pClass )
			{
				FxString derivedFrom("");
				if( pClass->GetBaseClassDesc() )
				{
					derivedFrom = FxString::Concat(" : <i>", pClass->GetBaseClassDesc()->GetName().GetAsString());
					derivedFrom = FxString::Concat(derivedFrom, "</i>");
				}
				FxString msg("<b>");
				msg << pClass->GetName().GetAsCstr() << "</b>" << derivedFrom.GetData();
				FxVM::DisplayInfo(msg);
				msg = "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<i>sizeof(";
				msg << pClass->GetName().GetAsCstr() << "): <b>" << pClass->GetSize() 
					<< "bytes</b> version: <b>" << pClass->GetCurrentVersion() << "</b></i>";
				FxVM::DisplayInfo(msg);
				if( pClass->GetNumChildren() )
				{
					msg = "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<i>number of children: <b>";
					msg << pClass->GetNumChildren() << "</b></i>";
					FxVM::DisplayInfo(msg);
				}
			}
		}
		FxString msg("Number of classes: <b>");
		msg << FxClass::GetNumClasses() << "</b>";
        FxVM::DisplayInfo(msg);
		FxVM::DisplayInfo("<b>----end class info----</b>");
	}
	if( argList.GetArgument("-dumpconsolecommands") )
	{
		FxVM::DisplayInfo("<b>----start console command info----</b>");
		FxSize numCommands = FxVM::GetNumCommands();
		FxString msg("<b><i>");
		msg << numCommands << "</i></b> console commands:";
		FxVM::DisplayInfo(msg);
		for( FxSize i = 0; i < numCommands; ++i )
		{
			FxVM::FxCommandDesc* pCommand = FxVM::GetCommandDesc(i);
			if( pCommand )
			{
				FxVM::DisplayInfo(pCommand->CommandName.GetData());
			}
		}
		FxVM::DisplayInfo("<b>----end console command info----</b>");
	}
	if( argList.GetArgument("-dumpconsolevariables") )
	{
		FxVM::DisplayInfo("<b>----start console variable info----</b>");
		FxSize numConsoleVariables = FxVM::GetNumConsoleVariables();
		FxString msg("<b><i>");
		msg << numConsoleVariables << "</i></b> console variables:";
		FxVM::DisplayInfo(msg);
		for( FxSize i = 0; i < numConsoleVariables; ++i )
		{
			FxConsoleVariable* pConsoleVar = FxVM::GetConsoleVariable(i);
			if( pConsoleVar )
			{
				msg = "<b>";
				msg << pConsoleVar->GetNameAsCstr() << "</b>  ->  default value: <b><i>";
				msg << pConsoleVar->GetDefaultValue().GetData() << "</i></b>  value: <b><i>";
				msg << pConsoleVar->GetString().GetData() << "</i></b>  flags: <b><i>";
				msg << pConsoleVar->GetFlags() << "</i></b>";
				FxVM::DisplayInfo(msg);
			}
		}
		FxVM::DisplayInfo("<b>----end console variable info----</b>");
	}
	if( argList.GetArgument("-crash") )
	{
		FxVM::DisplayInfo("Attempting to crash with a NULL pointer dereference...");
		FxInt32* pCrashPointer = NULL;
		*pCrashPointer = 0;
	}
	if( argList.GetArgument("-spamcommandsystem") )
	{
		FxSize numCommands;
		argList.GetArgument("-spamcommandsystem", numCommands);
		for( FxSize i = 0; i < numCommands; ++i )
		{
			FxVM::ExecuteCommand("eatMem -size 1024");
		}
	}
	FxString bonePoseName;
	if( argList.GetArgument("-dumpbonepose", bonePoseName) )
	{
		FxFaceGraphNode* pNode = NULL;
		if( FxSessionProxy::GetFaceGraphNode(bonePoseName, &pNode) )
		{
			FxBonePoseNode* pBonePoseNode = FxCast<FxBonePoseNode>(pNode);
			if( pBonePoseNode )
			{
				FxString header("Bone Pose <b>");
				header << pBonePoseNode->GetNameAsCstr() << "</b> contains <b><i>"
					   << pBonePoseNode->GetNumBones() << "</i></b> bones";
				FxVM::DisplayInfo(header);
				for( FxSize i = 0; i < pBonePoseNode->GetNumBones(); ++i )
				{
					const FxBone& poseBone = pBonePoseNode->GetBone(i);
					FxString boneInfo("&nbsp;&nbsp;&nbsp;&nbsp;");
					boneInfo << poseBone.GetNameAsCstr();
					FxVM::DisplayInfo(boneInfo);
					FxVec3 bonePos = poseBone.GetPos();
					boneInfo = "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;bonePos = ";
					FxMsg("%s [%f, %f, %f]", boneInfo.GetData(), bonePos.x, bonePos.y, bonePos.z);
					FxQuat boneRot = poseBone.GetRot();
					boneInfo = "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;boneRot = ";
					FxMsg("%s [%f, %f, %f, %f]", boneInfo.GetData(), boneRot.w, boneRot.x, boneRot.y, boneRot.z);
					FxVec3 boneScale = poseBone.GetScale();
					boneInfo = "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;boneScale = ";
					FxMsg("%s [%f, %f, %f]", boneInfo.GetData(), boneScale.x, boneScale.y, boneScale.z);
				}
			}
			else
			{
				FxVM::DisplayWarning("node is not a bone pose node!");
			}
		}
		else
		{
			FxVM::DisplayError("node not found!");
		}
	}
	if( argList.GetArgument("-dumprefpose") )
	{
		FxActor* pActor = NULL;
		FxSessionProxy::GetActor(&pActor);
		if( pActor )
		{
			FxMasterBoneList& masterBoneList = pActor->GetMasterBoneList();
			FxSize numRefBones = masterBoneList.GetNumRefBones();
			FxString header("<b>Reference Pose</b> contains <b><i>");
			header << numRefBones << "</i></b> bones";
			FxVM::DisplayInfo(header);
			for( FxSize i = 0; i < numRefBones; ++i )
			{
				const FxBone& refBone = masterBoneList.GetRefBone(i);
				FxString boneInfo("&nbsp;&nbsp;&nbsp;&nbsp;");
				boneInfo << refBone.GetNameAsCstr();
				FxVM::DisplayInfo(boneInfo);
				FxVec3 refPos = refBone.GetPos();
				boneInfo = "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;refPos = ";
				FxMsg("%s [%f, %f, %f]", boneInfo.GetData(), refPos.x, refPos.y, refPos.z);
				FxQuat refRot = refBone.GetRot();
				boneInfo = "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;refRot = ";
				FxMsg("%s [%f, %f, %f, %f]", boneInfo.GetData(), refRot.w, refRot.x, refRot.y, refRot.z);
				FxVec3 refScale = refBone.GetScale();
				boneInfo = "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;refScale = ";
				FxMsg("%s [%f, %f, %f]", boneInfo.GetData(), refScale.x, refScale.y, refScale.z);
			}
		}
	}
	if( argList.GetArgument("-dumpmemorystats") )
	{
		FxString stats = "Current allocations: ";
		stats << FxGetCurrentBytesAllocated() << " bytes in "
			  << FxGetCurrentNumAllocations() << " unique allocations.";
		FxVM::DisplayInfo(stats);

		stats = "Peak allocations: ";
		stats << FxGetPeakBytesAllocated() << " bytes in "
			  << FxGetPeakNumAllocations() << " unique allocations.";
		FxVM::DisplayInfo(stats);

		stats = "Min allocation size: ";
		stats << FxGetMinAllocationSize() << " bytes.";
		FxVM::DisplayInfo(stats);

		stats = "Median allocation size: ";
		stats << FxGetMedianAllocationSize() << " bytes.";
		FxVM::DisplayInfo(stats);

		stats = "Max allocation size: ";
		stats << FxGetMaxAllocationSize() << " bytes.";
		FxVM::DisplayInfo(stats);

		stats = "Avg allocation size: ";
		stats << FxGetAvgAllocationSize() << " bytes.";
		FxVM::DisplayInfo(stats);
	}
	if( argList.GetArgument("-dumpmemoryallocs") )
	{
		FxSize allocNum = 0;
		FxSize allocSize = 0;
		FxChar* allocSystem = 0;
		FxAllocIterator curr = FxGetAllocStart();
		while( curr )
		{
			if( FxGetAllocInfo(curr, allocNum, allocSize, allocSystem) )
			{
				FxString allocInfo = "Allocation #";
				allocInfo << allocNum << ": " << allocSize << " bytes from " << allocSystem;
				FxVM::DisplayInfo(allocInfo);
			}

			curr = FxGetAllocNext(curr);
		}
	}
	if( argList.GetArgument("-testbaking") )
	{
		FxString nodeToBake;
		argList.GetArgument("-testbaking", nodeToBake);
		FxString animGroup;
		FxString anim;
		if(	FxSessionProxy::GetSelectedAnimGroup(animGroup) )
		{
			if( FxSessionProxy::GetSelectedAnim(anim) )
			{
				FxArray<FxName> nodesToBake;
				nodesToBake.PushBack(nodeToBake.GetData());

				FxActor* actor = NULL;
				FxSessionProxy::GetActor(&actor);
				FxFaceGraphBaker baker(actor);
				baker.SetAnim(animGroup.GetData(), anim.GetData());
				if( baker.Bake(nodesToBake) )
				{
					if( 1 != baker.GetNumResultAnimCurves() )
					{
						FxVM::DisplayError("No curves resulted from bake!");
					}
					else
					{
						// This is an intentional copy.
						FxAnimCurve animCurve = baker.GetResultAnimCurve(0);
						FxString newCurveName = animCurve.GetNameAsString();
						newCurveName += "_Baked";
						animCurve.SetName(newCurveName.GetData());

						FxAnim* pAnim = actor->GetAnimPtr(animGroup.GetData(), anim.GetData());
						if( !pAnim->AddAnimCurve(animCurve) )
						{
							pAnim->RemoveAnimCurve(animCurve.GetName());
							pAnim->AddAnimCurve(animCurve);
						}
						FxSessionProxy::SetSelectedAnimGroup(animGroup);
						FxSessionProxy::SetSelectedAnim(anim);
					}
				}
				else
				{
					FxVM::DisplayError("Bake failed!");
				}
			}
		}
	}
	if( argList.GetArgument("-dumpmountedanimationsets") )
	{
		FxSize numMountedAnimSets = FxAnimSetManager::GetNumMountedAnimSets();
		FxString info = "Dumping ";
		info << numMountedAnimSets << " mounted animation sets...";
		FxVM::DisplayInfo(info);
		for( FxSize i = 0; i < numMountedAnimSets; ++i )
		{
			info = "[";
			info << i << "] - " << FxAnimSetManager::GetMountedAnimSetName(i).GetData();
			FxVM::DisplayInfo(info);
		}
	}
	if( argList.GetArgument("-dumpanims") )
	{
		FxActor* actor = NULL;
		FxSessionProxy::GetActor(&actor);
		if( actor )
		{
			FxSize numGroups = actor->GetNumAnimGroups();
			for( FxSize i = 0; i < numGroups; ++i )
			{
				FxAnimGroup& animGroup = actor->GetAnimGroup(i);
                FxSize numAnims = animGroup.GetNumAnims();
				FxString info = "Dumping animation group ";
				info << animGroup.GetNameAsCstr() << "(<b>" << numAnims << "</b>) animations...";
				FxVM::DisplayInfo(info);
				for( FxSize j = 0; j < numAnims; ++j )
				{
					info = "[";
					info << j << "] - " << animGroup.GetAnim(j).GetNameAsCstr();
					FxVM::DisplayInfo(info);
				}
			}
		}
	}
	FxString audioFilename;
	if( argList.GetArgument("-saveaudiofile", audioFilename) )
	{
		FxString selectedAnimGroup;
		FxString selectedAnim;
		if( FxSessionProxy::GetSelectedAnimGroup(selectedAnimGroup) && 
			FxSessionProxy::GetSelectedAnim(selectedAnim) )
		{
			FxAnim* pAnim = NULL;
			FxSessionProxy::GetAnim(selectedAnimGroup, selectedAnim, &pAnim);
			if( pAnim )
			{
				FxAnimUserData* pAnimUserData = reinterpret_cast<FxAnimUserData*>(pAnim->GetUserData());
				if( pAnimUserData )
				{
					FxAudio* pAudio = pAnimUserData->GetAudioPointer();
					if( pAudio )
					{
						FxAudioFile waveFile(pAudio);
						if( waveFile.Save(audioFilename) )
						{
							FxVM::DisplayInfo("saved audio file!");
						}
						else
						{
							FxVM::DisplayError("failed to save audio file!");
						}
					}
					else
					{
						FxVM::DisplayError("no audio to save!");
					}
				}
				else
				{
					FxVM::DisplayError("no anim user data!");
				}
			}
			else
			{
				FxVM::DisplayError("no anim!");
			}
		}
		else
		{
			FxVM::DisplayError("no valid session!");
		}
	}
	if( argList.GetArgument("-about") )
	{
		FxMsg("Copyright (c) 2002-2008 OC3 Entertainment, Inc. - FaceFX was painstakingly developed by Jamie Redmond, John Briggs, and Doug Perkowski. - Copyright (c) 2002-2008 OC3 Entertainment, Inc.");
		FxMsg("Copyright (c) 2002-2008 OC3 Entertainment, Inc. - Many late nights and lots of blood, sweat, and tears went into the making of FaceFX, and only a few small babies were harmed. - Copyright (c) 2002-2008 OC3 Entertainment, Inc.");
		FxMsg("Copyright (c) 2002-2008 OC3 Entertainment, Inc. - All your LIPS are belong to us.  w00t. - Copyright (c) 2002-2008 OC3 Entertainment, Inc.");
	}
	return CE_Success;
}

FX_IMPLEMENT_CLASS(FxEatMemCommand, 0, FxCommand);

FxEatMemCommand::FxEatMemCommand()
{
	_isUndoable    = FxTrue;
	_memChunkSize  = 1;
	_pMemChunk     = NULL;
}

FxEatMemCommand::~FxEatMemCommand()
{
}

FxCommandSyntax FxEatMemCommand::CreateSyntax( void )
{
	FxCommandSyntax newSyntax;
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-s", "-size", CAT_Size));
	return newSyntax;
}

FxCommandError FxEatMemCommand::Execute( const FxCommandArgumentList& argList )
{
	if( argList.GetArgument("-size", _memChunkSize) )
	{
		return Redo();
	}
	FxVM::DisplayError("-size was not specified!");
	return CE_Failure;
}

FxCommandError FxEatMemCommand::Undo( void )
{
	if( _pMemChunk )
	{
		delete [] _pMemChunk;
		_pMemChunk = NULL;
	}
	return CE_Success;
}

FxCommandError FxEatMemCommand::Redo( void )
{
	_pMemChunk = new FxByte[_memChunkSize];
	if( _pMemChunk )
	{
		FxString infoString = "ate ";
		infoString << _memChunkSize << " bytes of memory!";
		FxVM::DisplayWarning(infoString);
		return CE_Success;
	}
	return CE_Failure;
}

} // namespace Face

} // namespace OC3Ent
