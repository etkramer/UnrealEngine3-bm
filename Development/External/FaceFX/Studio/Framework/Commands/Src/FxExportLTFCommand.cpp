//------------------------------------------------------------------------------
// The export LTF command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxExportLTFCommand.h"
#include "FxVM.h"
#include "FxSessionProxy.h"
#include "FxFaceGraphBaker.h"

#include <fstream>
using std::ofstream;
using std::endl;

namespace OC3Ent
{

namespace Face
{

FX_IMPLEMENT_CLASS(FxExportLTFCommand, 0, FxCommand);

FxExportLTFCommand::FxExportLTFCommand()
{
}

FxExportLTFCommand::~FxExportLTFCommand()
{
}

FxCommandSyntax FxExportLTFCommand::CreateSyntax( void )
{
	FxCommandSyntax newSyntax;
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-ag", "-animgroup", CAT_String));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-an", "-animname", CAT_String));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-a", "-all", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-d", "-dir", CAT_String));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-f", "-file", CAT_String));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-ex", "-extratracks", CAT_StringArray));
	return newSyntax;
}

FxCommandError FxExportLTFCommand::Execute( const FxCommandArgumentList& argList )
{
	FxArray<FxString> extraTracks;
	argList.GetArgument("-extratracks", extraTracks);

	FxString filePath;
	if( argList.GetArgument("-file", filePath) )
	{
		FxString animGroupName;
		FxString animName;
		if( argList.GetArgument("-animgroup", animGroupName) &&
			argList.GetArgument("-animname", animName) )
		{
			if( ExportLTF(animGroupName, animName, extraTracks, filePath) )
			{
				return CE_Success;
			}
		}
		else
		{
			FxVM::DisplayError("-animgroup and -animname must both be specified!");
		}
	}	
	else if( argList.GetArgument("-all") )
	{
		FxString outputDirectory;
		if( argList.GetArgument("-dir", outputDirectory) )
		{
			if( ExportAll(extraTracks, outputDirectory) )
			{
				return CE_Success;
			}
		}
	}
	return CE_Failure;
}


FxBool FxExportLTFCommand::
ExportLTF( const FxString& groupName, const FxString& animName, 
 		   const FxArray<FxString>& extraTracks, const FxString& filePath )
{
#ifndef NO_SAVE_VERSION
	FxActor* pActor = NULL;
	FxSessionProxy::GetActor(&pActor);
	if( pActor )
	{
		FxArray<FxName> targetNames;
		FxSessionProxy::GetTrackNamesInMapping(targetNames);
		// Now targetNames contains the speech target node names
		// to bake, so add the speech gesture node names.
		//@todo At some point do not have these hard coded.
		targetNames.PushBack("Orientation Head Pitch");
		targetNames.PushBack("Orientation Head Roll");
		targetNames.PushBack("Orientation Head Yaw");
		targetNames.PushBack("Emphasis Head Pitch");
		targetNames.PushBack("Emphasis Head Roll");
		targetNames.PushBack("Emphasis Head Yaw");
		targetNames.PushBack("Gaze Eye Pitch");
		targetNames.PushBack("Gaze Eye Yaw");
		targetNames.PushBack("Eyebrow Raise");
		targetNames.PushBack("Blink");
		targetNames.PushBack("Head Pitch");
		targetNames.PushBack("Head Roll");
		targetNames.PushBack("Head Yaw");
		targetNames.PushBack("Eye Pitch");
		targetNames.PushBack("Eye Yaw");
		// Add any extra tracks to targetNames.
		for( FxSize i = 0; i < extraTracks.Length(); ++i )
		{
			targetNames.PushBack(extraTracks[i].GetData());
		}
		// Bake it all out.
		FxFaceGraphBaker baker(pActor);
		baker.SetAnim(groupName.GetData(), animName.GetData());
		baker.Bake(targetNames);

		// Get the phoneme list.
		FxPhonWordList phonWordList;
		FxSessionProxy::GetPhonWordList(&phonWordList);
		
		// Write the LTF file.
		ofstream ltfFile(filePath.GetData());
		if( ltfFile )
		{
			ltfFile << "// LTF file" << endl
					<< "LTF" << endl << endl
					<< "// Version" << endl
					<< "1.1" << endl << endl
					<< "// Phoneme Timing Data" << endl
					<< "// ===================" << endl << endl
					<< "// Num Phonemes" << endl
					<< phonWordList.GetNumPhonemes() << endl << endl;

			for( FxSize i = 0; i < phonWordList.GetNumPhonemes(); ++i )
			{
				ltfFile << phonWordList.GetPhonemeEnum(i) << " "
					    << phonWordList.GetPhonemeStartTime(i) << " " 
						<< phonWordList.GetPhonemeEndTime(i) << endl;
			}

			ltfFile << endl
					<< "// Curve Data" << endl
					<< "// ===================" << endl << endl
					<< "// Num Curves" << endl
					<< baker.GetNumResultAnimCurves() << endl << endl;

			for( FxSize i = 0; i < baker.GetNumResultAnimCurves(); ++i )
			{
				const FxAnimCurve& animCurve = baker.GetResultAnimCurve(i);
				ltfFile << "// Curve " << i << endl
						<< "{" << endl
						<< "\t// Name" << endl
						<< "\t" << animCurve.GetNameAsCstr() << endl << endl
						<< "\t// Num Keys" << endl
						<< "\t" << animCurve.GetNumKeys() << endl << endl
						<< "\t// Keys\n\t";

				for( FxSize k = 0; k < animCurve.GetNumKeys(); ++k )
				{
					/*if( !(k%25) )*/ if( !((k+1)%26) ) ltfFile << endl << "\t";
					ltfFile << animCurve.GetKey(k).GetTime() << " "
						<< animCurve.GetKey(k).GetValue() << " ";
				}
				ltfFile << endl << "}" << endl << endl;
			}

			return FxTrue;
		}
	}
#endif
	return FxFalse;
}

FxBool FxExportLTFCommand::
ExportAll( const FxArray<FxString>& extraTracks, const FxString& outputDirectory )
{
	FxActor* pActor = NULL;
	FxSessionProxy::GetActor(&pActor);
	if( pActor )
	{
		FxSize numAnimGroups = pActor->GetNumAnimGroups();
		for( FxSize i = 0; i < numAnimGroups; ++i )
		{
			const FxAnimGroup& animGroup = pActor->GetAnimGroup(i);
			FxString animGroupName = animGroup.GetNameAsString();
			FxSessionProxy::SetSelectedAnimGroup(animGroupName);
			FxSize numAnims = animGroup.GetNumAnims();
			for( FxSize j = 0; j < numAnims; ++j )
			{
				const FxAnim& anim = animGroup.GetAnim(j);
				FxString animName = anim.GetNameAsString();
				FxSessionProxy::SetSelectedAnim(animName);
				FxString filePath = outputDirectory;
				if( FxPlatformPathSeparator != filePath[filePath.Length()-1] )
				{
					filePath += FxPlatformPathSeparator;
				}
				filePath += pActor->GetNameAsString();
				filePath += "_";
				filePath += animGroupName;
				filePath += "_";
				filePath += animName;
				filePath += ".ltf";
				if( !ExportLTF(animGroupName, animName, extraTracks, filePath) )
				{
					return FxFalse;
				}
			}
		}
		return FxTrue;
	}
	return FxFalse;
}

} // namespace Face

} // namespace OC3Ent
