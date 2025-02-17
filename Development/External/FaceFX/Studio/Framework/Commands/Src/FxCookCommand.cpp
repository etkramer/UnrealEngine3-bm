//------------------------------------------------------------------------------
// The cook command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxCookCommand.h"
//@todo This is included to get access to FxMsg() etc.  We might want to remove 
//      this later on if we actually implement the plug-in architecture.
#include "FxConsole.h"
#include "FxActor.h"
#include "FxAnimSet.h"

namespace OC3Ent
{

namespace Face
{

FX_IMPLEMENT_CLASS(FxCookCommand, 0, FxCommand);

FxCookCommand::FxCookCommand()
{
}

FxCookCommand::~FxCookCommand()
{
}

FxCommandSyntax FxCookCommand::CreateSyntax( void )
{
	FxCommandSyntax newSyntax;
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-p", "-platform", CAT_String));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-f", "-file", CAT_String));
	return newSyntax;
}

FxCommandError FxCookCommand::Execute( const FxCommandArgumentList& argList )
{
	FxString platform, filename;
	if( argList.GetArgument("-platform", platform) && 
		argList.GetArgument("-file", filename) )
	{
		FxString ext = filename.AfterLast('.');

		FxMsg("Loading %s...", filename.GetData());

		FxActor actor;
		FxAnimSet animSet;
		if( ext == "fxe" || ext == "FXE" )
		{
			if( !FxLoadAnimSetFromFile(animSet, filename.GetData(), FxTrue) )
			{
				FxError("<b>ERROR:</b> file does not exist!");
				return CE_Failure;
			}
		}
		else if( ext == "fxa" || ext == "FXA" )
		{
			if( !FxLoadActorFromFile(actor, filename.GetData(), FxTrue) )
			{
				FxError("<b>ERROR:</b> file does not exist!");
				return CE_Failure;
			}
		}
		else
		{
			FxError("<b>ERROR:</b> file does not exist or is not an .fxe or .fxa file!");
			return CE_Failure;
		}

		FxArchive::FxArchiveByteOrder targetPlatformByteOrder = FxArchive::ABO_LittleEndian;
		if( platform == "XBOX360" || platform == "xbox360" )
		{
			FxMsg("Platform: <b>XBox 360</b>");
			targetPlatformByteOrder = FxArchive::ABO_BigEndian;
		}
		else if( platform == "PC" || platform == "pc" )
		{
			FxMsg("Platform: <b>PC</b>");
			targetPlatformByteOrder = FxArchive::ABO_LittleEndian;
		}
		else if( platform == "BIGENDIAN" || platform == "bigendian" )
		{
			FxMsg("Platform: <b>Unknown Big Endian</b>");
			targetPlatformByteOrder = FxArchive::ABO_BigEndian;
		}
		else if( platform == "LITTLEENDIAN" || platform == "littleendian" )
		{
			FxMsg("Platform: <b>Unknown Little Endian</b>");
			targetPlatformByteOrder = FxArchive::ABO_LittleEndian;
		}
		else
		{
			FxError("<b>ERROR:</b> invalid platform!");
			return CE_Failure;
		}

		FxMsg("<i>Cooking...<i>");

		FxString newFilename = filename.BeforeLast('.');
		newFilename += "-Cooked_";
		newFilename += platform;
		newFilename += ".";
		newFilename += ext;

		if( ext == "fxe" || ext == "FXE" )
		{
			if( !FxSaveAnimSetToFile(animSet, newFilename.GetData(), targetPlatformByteOrder) )
			{
				FxError("<b>ERROR:</b> failed to save animset!");
				return CE_Failure;
			}
		}
		else if( ext == "fxa" || ext == "FXA" )
		{
			if( !FxSaveActorToFile(actor, newFilename.GetData(), targetPlatformByteOrder) )
			{
				FxError("<b>ERROR:</b> failed to save actor!");
				return CE_Failure;
			}
		}
	
		FxMsg("Cook complete: <b>%s</b> cooked to <b>%s</b>", filename.GetData(), newFilename.GetData());
		return CE_Success;
	}
	FxError("<b>ERROR:</b> invalid arguments (must specify both -platform and -file)!");
	return CE_Failure;
}

} // namespace Face

} // namespace OC3Ent
