//------------------------------------------------------------------------------
// A few utility functions available throughout Studio.
// 
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"
#include "FxUtilityFunctions.h"
#include "FxName.h"
#include "FxFile.h"
#include "FxTearoffWindow.h"
#ifdef IS_FACEFX_STUDIO
	#include "FxStudioApp.h"
	#include "FxVM.h"
#endif

#ifdef __UNREAL__
	#include "UnrealEd.h"
	#include "EngineSoundClasses.h"
#else
	#include "wx/tokenzr.h"
	#include "wx/filename.h"
	#ifdef WIN32
		// For _access() on Win32.
		#include <io.h>
	#endif
#endif

namespace OC3Ent
{

namespace Face
{

#ifdef __UNREAL__
void FindSoundNodeWaveHelper( const USoundNode* SoundNode, const FxString& SoundNodeWaveName, USoundNodeWave*& SoundNodeWave )
{
	if( SoundNode )
	{
		if( FxString(TCHAR_TO_ANSI(*SoundNode->GetName())) == SoundNodeWaveName )
		{
			USoundNodeWave* TestSoundNodeWave = CastChecked<USoundNodeWave>(SoundNode);
			if( TestSoundNodeWave )
			{
				SoundNodeWave = TestSoundNodeWave;
			}
		}
		else
		{
			FxInt32 NumChildren = SoundNode->ChildNodes.Num();
			for( FxInt32 i = 0; i < NumChildren; ++i )
			{
				const USoundNode* Child = SoundNode->ChildNodes(i);
				if( Child )
				{
					FindSoundNodeWaveHelper(Child, SoundNodeWaveName, SoundNodeWave);
				}
			}
		}
	}
}

USoundNodeWave* FindSoundNodeWave( const USoundCue* SoundCue, const FxString& SoundNodeWaveName )
{
	USoundNodeWave* SoundNodeWave = NULL;
	if( SoundCue && SoundCue->FirstNode )
	{
		FindSoundNodeWaveHelper(SoundCue->FirstNode, SoundNodeWaveName, SoundNodeWave);
	}
	return SoundNodeWave;
}

void FindAllSoundNodeWavesHelper( const USoundNode* SoundNode, FxArray<wxString>& SoundNodeWaves )
{
	if( SoundNode )
	{
		if( SoundNode->IsA(USoundNodeWave::StaticClass()) )
		{
			SoundNodeWaves.PushBack(wxString(*SoundNode->GetName()));
		}
		FxInt32 NumChildren = SoundNode->ChildNodes.Num();
		for( FxInt32 i = 0; i < NumChildren; ++i )
		{
			const USoundNode* Child = SoundNode->ChildNodes(i);
			if( Child )
			{
				FindAllSoundNodeWavesHelper(Child, SoundNodeWaves);
			}
		}
	}
}

FxArray<wxString> FindAllSoundNodeWaves( const USoundCue* SoundCue )
{
	FxArray<wxString> SoundNodeWaves;
	if( SoundCue && SoundCue->FirstNode )
	{
		FindAllSoundNodeWavesHelper(SoundCue->FirstNode, SoundNodeWaves);
	}
	return SoundNodeWaves;
}
#endif

int FxMessageBox( const wxString& message, const wxString& caption, int style, 
				  wxWindow* parent, int x, int y )
{
	int retVal = wxID_OK;
#ifdef IS_FACEFX_STUDIO
	FxTearoffWindow::CacheAndSetAllTearoffWindowsToNotAlwaysOnTop();
	FxConsoleVariable* g_unattended(FxVM::FindConsoleVariable("g_unattended"));
	FxBool isUnattendedCVarSet = FxFalse;
	if( g_unattended )
	{
		isUnattendedCVarSet = static_cast<FxBool>(*g_unattended);
	}

	if( (FxStudioApp::IsCommandLineMode() || FxStudioApp::IsUnattended() || isUnattendedCVarSet) && 
		(~style & wxYES_NO) )
	{
		FxVM::DisplayWarning(FxString(message.mb_str(wxConvLibc)));
	}
	else
	{
#endif
		retVal = wxMessageBox(message, caption, style, parent, x, y);
#ifdef IS_FACEFX_STUDIO
	}
	FxTearoffWindow::RestoreAllTearoffWindowsAlwaysOnTop();
#endif
	return retVal;
}

FxString GetInterpName( FxInterpolationType interp )
{
	switch( interp )
	{
	case IT_Hermite:
		return "Hermite";
	case IT_Linear:
		return "Linear";
	default:
		return "Unknown interpolation algorithm";
	};
}

FxInterpolationType GetInterpolator( const FxString& name )
{
	if( name == "Hermite" )
	{
		return IT_Hermite;
	}
	else if( name == "Linear" )
	{
		return IT_Linear;
	}
	return IT_Hermite; // Assume Hermite.
}

wxColour FxStringToColour( const wxString& string )
{
	long red = 0, green = 0, blue = 0;
	wxStringTokenizer tokenizer(string, wxT(" "));
	if( tokenizer.CountTokens() == 3 )
	{
		tokenizer.GetNextToken().ToLong(&red);
		tokenizer.GetNextToken().ToLong(&green);
		tokenizer.GetNextToken().ToLong(&blue);
	}
	return wxColour(red, green, blue);
}

wxString FxColourToString( const wxColour& colour )
{
	wxString retval;
	retval << colour.Red() << wxT(" ") << colour.Green() << wxT(" ") << colour.Blue();
	return retval;
}

FxColor FxStringToColor( const wxString& string )
{
	wxColour colour = FxStringToColour(string);
	return FxColor(colour.Red(), colour.Green(), colour.Blue());
}

wxString FxColorToString( const FxColor& color )
{
	wxColour colour(color.GetRedByte(), color.GetGreenByte(), color.GetBlueByte());
	return FxColourToString(colour);
}

int wxCALLBACK CompareNames( long item1, long item2, long sortData )
{
	FxName* name1 = reinterpret_cast<FxName*>(item1);
	FxName* name2 = reinterpret_cast<FxName*>(item2);
	if( name1 && name2 )
	{
		if( sortData == 1 )
		{
			return FxStrcmp(name1->GetAsCstr(), name2->GetAsCstr());
		}
		else
		{
			return FxStrcmp(name2->GetAsCstr(), name1->GetAsCstr());
		}
	}
	return 0;
}

int wxCALLBACK CompareStrings( long item1, long item2, long sortData )
{
	FxString* name1 = reinterpret_cast<FxString*>(item1);
	FxString* name2 = reinterpret_cast<FxString*>(item2);
	if( name1 && name2 )
	{
		if( sortData == 1 )
		{
			return FxStrcmp(name1->GetData(), name2->GetData());
		}
		else
		{
			return FxStrcmp(name2->GetData(), name1->GetData());
		}
	}
	return 0;
}

void FxTriggerScriptEvent( FxScriptTriggerEventType trigger, void* param )
{
#ifdef IS_FACEFX_STUDIO
	FxString scriptFile;
	FxString appOverrideScriptFile;

	switch( trigger )
	{
	case STET_PostLoadApp:
		{
			wxFileName autoexecPath = FxStudioApp::GetAppDirectory();
			autoexecPath.SetName(wxT("autoexec.fxl"));
			scriptFile = autoexecPath.GetFullPath().mb_str(wxConvLibc);
        }
		break;
	case STET_PreLoadActor:
	case STET_PostLoadActor:
	case STET_PreSaveActor:
	case STET_PostSaveActor:
		{
			wxFileName appOverrideScriptFilePath = FxStudioApp::GetAppDirectory();

			FxAssert(param);
			if( param )
			{
				FxString* pParamString = reinterpret_cast<FxString*>(param);
				FxAssert(pParamString);

#ifdef __UNREAL__
				wxFileName scriptPath(FxStudioApp::GetAppDirectory());
				wxString filename(pParamString->GetData(), wxConvLibc);
				filename += wxT(".fxl");
				scriptPath.SetName(filename);
				FxString scriptFileStem(scriptPath.GetFullPath().mb_str(wxConvLibc));
				scriptFileStem = scriptFileStem.BeforeLast('.');
#else
				FxString scriptFileStem = pParamString->BeforeLast('.');
#endif // __UNREAL__
				scriptFile = scriptFileStem;
				switch( trigger )
				{
				case STET_PreLoadActor:
					appOverrideScriptFilePath.SetName(wxT("preloadactor.fxl"));	
					scriptFile += "_PreLoad.fxl";
					break;
				case STET_PostLoadActor:
					appOverrideScriptFilePath.SetName(wxT("postloadactor.fxl"));
					scriptFile += "_PostLoad.fxl";
					break;
				case STET_PreSaveActor:
					appOverrideScriptFilePath.SetName(wxT("presaveactor.fxl"));
					scriptFile += "_PreSave.fxl";
					break;
				case STET_PostSaveActor:
					appOverrideScriptFilePath.SetName(wxT("postsaveactor.fxl"));
					scriptFile += "_PostSave.fxl";
					break;
				default:
					FxAssert(!"Unknown script trigger event type!");
					break;
				}

				appOverrideScriptFile = appOverrideScriptFilePath.GetFullPath().mb_str(wxConvLibc);

				// If the script file does not exist, fall back to the old style
				// ActorName.fxl, but *only* in the STET_PostLoadActor case.
				if( !FxFileExists(scriptFile) && STET_PostLoadActor == trigger )
				{
					scriptFile = scriptFileStem;
					scriptFile += ".fxl";
				}
			}
		}
		break;
	default:
		FxAssert(!"Unknown script trigger event type!");
		break;
	}

	if( appOverrideScriptFile.Length() > 0 )
	{
		if( FxFileExists(appOverrideScriptFile) )
		{
			FxString execCommand = "exec -file \"";
			execCommand += appOverrideScriptFile;
			execCommand += "\"";
			FxVM::ExecuteCommand(execCommand);
		}
	}

	if( scriptFile.Length() > 0 )
	{
		if( FxFileExists(scriptFile) )
		{
			FxString execCommand = "exec -file \"";
			execCommand += scriptFile;
			execCommand += "\"";
			FxVM::ExecuteCommand(execCommand);
		}
	}
#else
	trigger; param; // prevent warnings.
#endif
}

FxBool FxFileExists( const FxString& filename )
{
	return FxFile::Exists(filename.GetData());
}

FxBool FxFileExists( const wxString& filename )
{
	return FxFileExists(FxString(filename.mb_str(wxConvLibc)));
}

wxString FxLoadUnicodeTextFile( const FxString& filename )
{
	wxString textFileContents;
	if( FxFileExists(filename) )
	{
		FxFile file(filename.GetData()); // Opens in binary read by default.
		FxByte* fileContents = NULL;
		FxSize fileLength = file.Length();
		if( file.IsValid() && fileLength > 0 )
		{
			fileContents = new FxByte[fileLength+2];
			file.Read(reinterpret_cast<FxByte*>(fileContents), fileLength);
			fileContents[fileLength] = NULL;
			fileContents[fileLength+1] = NULL;
		}
		file.Close();

		wxString testString;
		if( fileContents )
		{
			if( fileLength >= 2 && fileContents[0] == 0xFF && fileContents[1] == 0xFE )
			{
#if !defined(FX_LITTLE_ENDIAN)
				// Flip the byte order
				for( FxSize i = 2; i < fileLength; i += 2 )
				{
					FxByteSwap(reinterpret_cast<FxByte*>(fileContents + i), 2);
				}
#endif
				// We're probably looking at a little-endian unicode file.
				FxWString tempString(reinterpret_cast<FxWChar*>(fileContents + 2));
				testString = wxString(tempString.GetData());
			}
			else if( fileLength >= 2 && fileContents[0] == 0xFE && fileContents[1] == 0xFF )
			{
#if defined(FX_LITTLE_ENDIAN)
				// Flip the byte order
				for( FxSize i = 2; i < fileLength; i += 2 )
				{
					FxByteSwap(reinterpret_cast<FxByte*>(fileContents + i), 2);
				}
#endif
				// We're probably looking at a big-endian unicode file.
				FxWString tempString(reinterpret_cast<FxWChar*>(fileContents + 2));
				testString = wxString(tempString.GetData());
			}
			else if( fileLength >= 3 && fileContents[0] == 0xEF && fileContents[1] == 0xBB && fileContents[2] == 0xBF )
			{
				// We're probably looking at a UTF-8 encoded file.
				testString = wxString(reinterpret_cast<FxChar*>(fileContents) + 3, wxConvUTF8);
			}
			else
			{
				// We're looking at either a strange unicode encoding or ascii.
				testString = wxString(reinterpret_cast<FxChar*>(fileContents), wxConvLibc);
			}

			delete [] fileContents;
		}

		// Strip newlines, carriage returns and tabs.
		wxString correctedText;
		for( FxSize i = 0; i < testString.Length(); ++i )
		{
			if( testString[i] == wxT('\r') || testString[i] == wxT('\n') || testString[i] == wxT('\t') )
			{
				correctedText.Append(wxT(' '));
			}
			else
			{
				correctedText.Append(testString[i]);
			}
		}
		textFileContents = correctedText;
	}
	return textFileContents;
}

FxBool FxFileIsReadOnly( const FxString& filename )
{
#ifdef __UNREAL__
		return FxFalse;
#else
	#ifdef WIN32
		if( FxFileExists(filename) )
		{
			if( -1 == (_access(filename.GetData(), 2)) )
			{
				return FxTrue;
			}
		}
		return FxFalse;
	#else
		return FxFalse;
	#endif
#endif
}

FxBool FxFileIsReadOnly( const wxString& filename )
{
	return FxFileIsReadOnly(FxString(filename.mb_str(wxConvLibc)));
}

} // namespace Face

} // namespace OC3Ent
