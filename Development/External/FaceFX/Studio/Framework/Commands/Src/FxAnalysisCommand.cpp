//------------------------------------------------------------------------------
// The analysis command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxAnalysisCommand.h"
#include "FxVM.h"
#include "FxSessionProxy.h"
#include "FxStudioApp.h"

#ifdef __UNREAL__
	#include "UnrealEd.h"
	#include "EngineSoundClasses.h"
#endif

namespace OC3Ent
{

namespace Face
{

FX_IMPLEMENT_CLASS(FxAnalysisCommand, 0, FxCommand);

FxAnalysisCommand::FxAnalysisCommand()
{
	_shouldFlushUndo = FxTrue;
}

FxAnalysisCommand::~FxAnalysisCommand()
{
}

FxCommandSyntax FxAnalysisCommand::CreateSyntax( void )
{
	FxCommandSyntax newSyntax;
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-g", "-group", CAT_String));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-an", "-anim", CAT_String));
#ifdef __UNREAL__
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-sc", "-soundcue", CAT_String));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-snw", "-soundnodewave", CAT_String));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-pkg", "-package", CAT_String));
#else
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-au", "-audio", CAT_String));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-dir", "-directory", CAT_String));
#endif
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-l", "-language", CAT_String));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-cc", "-coarticulationconfig", CAT_String));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-gc", "-gestureconfig", CAT_String));
#ifndef __UNREAL__
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-r", "-recurse", CAT_Flag));
#endif
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-o", "-overwrite", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-nt", "-notext", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-ng", "-nogestures", CAT_Flag));
	return newSyntax;
}

#ifdef __UNREAL__
FxCommandError FxAnalysisCommand::Execute( const FxCommandArgumentList& argList )
{
#ifdef FX_NO_ANALYZE_COMMAND
	#if defined(EVALUATION_VERSION)
		FxVM::DisplayError("The analyze command (and thus batch processing) is disabled in the evaluation version.");
	#elif defined(MOD_DEVELOPER_VERSION)
		FxVM::DisplayError("The analyze command (and thus batch processing) is disabled in the mod developer version.");
	#else
		FxVM::DisplayError("The analyze command is disabled in this version.");
	#endif
	// Return success so that scripts with analyze commands will continue to 
	// run properly.
	return CE_Success;
#else
	// Get the language.
	FxString language;
	FxArray<FxLanguageInfo> languages;
	FxSessionProxy::QueryLanguageInfo(languages);
	for( FxSize i = 0; i < languages.Length(); ++i )
	{
		if( languages[i].isDefault )
		{
			language = languages[i].languageName;
			break;
		}
	}
	argList.GetArgument("-language", language);

	// Get the coarticulation configuration.
	FxString coarticulationConfig = "";
	argList.GetArgument("-coarticulationconfig", coarticulationConfig);
	// Get the gesture configuration.
	FxString gestureConfig = "";
	argList.GetArgument("-gestureconfig", gestureConfig);
	// Get the overwrite flag.
	FxBool overwrite = argList.GetArgument("-overwrite");
	FxBool notext = argList.GetArgument("-notext");
	FxBool noGestures = argList.GetArgument("-nogestures");
	// Check for batch mode.
	FxString package;
	if( argList.GetArgument("-package", package) )
	{
		FxStudioApp::SetIsUnattended(FxTrue);
		// Find the optional sound cue group name to analyze.  If the cue group 
		// name was specified it will follow the package name by '.'.
		FxString CueGroupName = package.AfterFirst('.');
		if( !CueGroupName.IsEmpty() ) 
		{
			// Remove the cue group name from the package name.
			package = package.BeforeFirst('.');
		}

		// The group name is optional in batch mode.  If it is not present,
		// the group name will come from the package in which the soundcue
		// exists.  If it is present, all animations will go into the
		// specified group, regardless of the package in which they exist.
		FxString groupName;
		if( !argList.GetArgument("-group", groupName) )
		{
			groupName = package;
		}
		// Analyze all SoundCues in the specified package and cue group.
		// Note that the package must be loaded first.
		FxString infoStr("Analyzing all USoundCue objects in package: ");
		infoStr += package;
		if( !CueGroupName.IsEmpty() ) 
		{
			infoStr += " and group: ";
			infoStr += CueGroupName;
		}
		infoStr += "...";
		FxVM::DisplayInfo(infoStr);
		for( TObjectIterator<USoundCue> It; It; ++It )
		{
			USoundCue* SoundCue = *It;
			if( SoundCue && SoundCue->GetOutermost() )
			{
				if( FxString(TCHAR_TO_ANSI(*SoundCue->GetOutermost()->GetName())) == package )
				{
					// Check if either the cue group wasn't specified or the specified one matches the current cue's group.
					if( CueGroupName.IsEmpty() || FxString(TCHAR_TO_ANSI(*SoundCue->GetOuter()->GetName())) == CueGroupName )
					{
						FxString soundCueStr("USoundCue: ");
						soundCueStr += TCHAR_TO_ANSI(*SoundCue->GetName());
						FxVM::DisplayInfo(soundCueStr);
						FxArray<wxString> SoundNodeWaves = FindAllSoundNodeWaves(SoundCue);
						//for( FxSize i = 0; i < SoundNodeWaves.Length(); ++i )
						if( SoundNodeWaves.Length() >= 1 )
						{
							FxString animName;
							//FxString SoundNodeWaveName(SoundNodeWaves[i].mb_str(wxConvLibc));
							FxString SoundNodeWaveName(SoundNodeWaves[0].mb_str(wxConvLibc));
							FxWString optionalText = L"";
							FxAudio* pAudio = NULL;
							USoundNodeWave* SoundNodeWave = FindSoundNodeWave(SoundCue, SoundNodeWaveName);
							if( SoundNodeWave )
							{
								pAudio = FxAudio::CreateFromSoundNodeWave(SoundNodeWave);
								if( SoundNodeWave->SpokenText.Len() > 0 )
								{
									optionalText = *SoundNodeWave->SpokenText;
								}
							}
							if( notext )
							{
								optionalText = L"";
							}
							animName = TCHAR_TO_ANSI(*SoundNodeWave->GetName());
							FxString soundNodeWaveStr("Analyzing USoundNodeWave: ");
							soundNodeWaveStr += animName;
							soundNodeWaveStr += "...";
							FxVM::DisplayInfo(soundNodeWaveStr);
							if( FxSessionProxy::AnalyzeRawAudio(pAudio, optionalText, groupName, animName, language, coarticulationConfig, gestureConfig, !noGestures, overwrite) )
							{
								// If the analysis was successful, set the SoundCue path and the
								// SoundNodeWave name.
								FxAnim* anim = NULL;
								if( FxSessionProxy::GetAnim(groupName, animName, &anim) )
								{
									if( anim )
									{
										anim->SetSoundCuePath(FxString(TCHAR_TO_ANSI(*SoundCue->GetPathName())));
										anim->SetSoundNodeWave(animName);
										anim->SetSoundCuePointer(SoundCue);
									}
								}
							}
						}
					}
				}
			}
			FxStudioApp::SetIsUnattended(FxFalse);
		}
		return CE_Success;
	}
	else
	{
		// In single file mode, the soundcue and soundnodewave are mandatory.
		FxString soundCue;
		FxString soundNodeWave;
		if( argList.GetArgument("-soundcue", soundCue) && 
			argList.GetArgument("-soundnodewave", soundNodeWave) )
		{
			// The animation name is optional.  If it is not given, the name
			// will come from the soundnodewave name.
			FxString animName;
			argList.GetArgument("-anim", animName);
			// The group name is optional.  If it is not present, the animation
			// will go into the "Default" group.
			FxString groupName;
			if( !argList.GetArgument("-group", groupName) )
			{
				groupName = FxAnimGroup::Default.GetAsString();
			}
			FxWString optionalText = L"";
			FxAudio* pAudio = NULL;
			USoundCue* SoundCue = LoadObject<USoundCue>(NULL, ANSI_TO_TCHAR(soundCue.GetData()), NULL, LOAD_NoWarn, NULL);
			USoundNodeWave* SoundNodeWave = FindSoundNodeWave(SoundCue, soundNodeWave);
			if( SoundNodeWave )
			{
				if( 0 == animName.Length() )
				{
					animName = TCHAR_TO_ANSI(*SoundNodeWave->GetName());
				}
				pAudio = FxAudio::CreateFromSoundNodeWave(SoundNodeWave);
				if( SoundNodeWave->SpokenText.Len() > 0 )
				{
					optionalText = *SoundNodeWave->SpokenText;
				}
			}
			if( notext )
			{
				optionalText = L"";
			}
			FxString soundNodeWaveStr("Analyzing USoundNodeWave: ");
			soundNodeWaveStr += animName;
			soundNodeWaveStr += "...";
			FxVM::DisplayInfo(soundNodeWaveStr);
			if( FxSessionProxy::AnalyzeRawAudio(pAudio, optionalText, groupName, animName, language, coarticulationConfig, gestureConfig, !noGestures, overwrite) )
			{
				// If the analysis was successful, set the SoundCue path and the
				// SoundNodeWave name.
				FxAnim* anim = NULL;
				if( FxSessionProxy::GetAnim(groupName, animName, &anim) )
				{
					if( anim )
					{
						anim->SetSoundCuePath(soundCue);
						anim->SetSoundNodeWave(soundNodeWave);
						anim->SetSoundCuePointer(SoundCue);
					}
				}
				return CE_Success;
			}
		}
		else
		{
			FxVM::DisplayError("-package, -soundcue, or -soundnodewave was not specified!");
		}
	}
	return CE_Failure;
#endif
}
#else
FxCommandError FxAnalysisCommand::Execute( const FxCommandArgumentList& argList )
{
#ifdef FX_NO_ANALYZE_COMMAND
	#if defined(EVALUATION_VERSION)
		FxVM::DisplayError("The analyze command (and thus batch processing) is disabled in the evaluation version.");
	#elif defined(MOD_DEVELOPER_VERSION)
		FxVM::DisplayError("The analyze command (and thus batch processing) is disabled in the mod developer version.");
	#else
		FxVM::DisplayError("The analyze command is disabled in this version.");
	#endif
	// Return success so that scripts with analyze commands will continue to 
	// run properly.
	return CE_Success;
#else
	// Get the language.
	FxString language;
	FxArray<FxLanguageInfo> languages;
	FxSessionProxy::QueryLanguageInfo(languages);
	for( FxSize i = 0; i < languages.Length(); ++i )
	{
		if( languages[i].isDefault )
		{
			language = languages[i].languageName;
			break;
		}
	}
	argList.GetArgument("-language", language);

	// Get the coarticulation configuration.
	FxString coarticulationConfig = "";
	argList.GetArgument("-coarticulationconfig", coarticulationConfig);
	// Get the gesture configuration.
	FxString gestureConfig = "";
	argList.GetArgument("-gestureconfig", gestureConfig);
	// Get the overwrite flag.
	FxBool overwrite = argList.GetArgument("-overwrite");
	// Get the no text flag.
	FxBool notext = argList.GetArgument("-notext");
	// Get the no speech gestures flag.
	FxBool noGestures = argList.GetArgument("-nogestures");

	// Check for batch mode.
	FxString directory;
	if( argList.GetArgument("-directory", directory) )
	{
		FxStudioApp::SetIsUnattended(FxTrue);
		// Get the recurse flag.
		FxBool recurse = argList.GetArgument("-recurse");
		// The group name is optional in batch mode.  If it is not present,
		// the group name will come from the directory in which the audio
		// file exists.  If it is present, all animations will go into the
		// specified group, regardless of the directory in which they exist.
		FxString groupName;
		argList.GetArgument("-group", groupName);
		if( FxSessionProxy::AnalyzeDirectory(directory, groupName, language, 
			coarticulationConfig, gestureConfig, !noGestures, overwrite, recurse, notext) )
		{
			FxStudioApp::SetIsUnattended(FxFalse);
			return CE_Success;
		}
	}
	else
	{
		// In single file mode, the audio file is mandatory.
		FxString audioFile;
		if( argList.GetArgument("-audio", audioFile) )
		{
			// The animation name is optional.  If it is not given, the name
			// will come from the audio file name.
			FxString animName;
			argList.GetArgument("-anim", animName);
			// The group name is optional.  If it is not present, the animation
			// will go into the "Default" group.
			FxString groupName;
			if( !argList.GetArgument("-group", groupName) )
			{
				groupName = FxAnimGroup::Default.GetAsString();
			}
			FxWString optionalText = L"__USE_TXT_FILE__";
			if( notext )
			{
				optionalText = L"";
			}
			if( FxSessionProxy::AnalyzeFile(audioFile, optionalText, groupName, 
				animName, language, coarticulationConfig, gestureConfig, 
				!noGestures, overwrite) )
			{
				return CE_Success;
			}
		}
		else
		{
			FxVM::DisplayError("-directory or -audio was not specified!");
		}
	}
	return CE_Failure;
#endif
}
#endif

} // namespace Face

} // namespace OC3Ent
