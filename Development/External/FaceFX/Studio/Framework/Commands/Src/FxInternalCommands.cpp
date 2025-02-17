//------------------------------------------------------------------------------
// All built-in FaceFX commands should be included here.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxInternalCommands.h"
#include "FxDeveloperCommand.h"
#include "FxUndoCommand.h"
#include "FxRedoCommand.h"
#include "FxFlushUndoCommand.h"
#include "FxExitCommand.h"
#include "FxPrintCommands.h"
#include "FxNewActorCommand.h"
#include "FxLoadActorCommand.h"
#include "FxCloseActorCommand.h"
#include "FxSaveActorCommand.h"
#include "FxRenameCommand.h"
#include "FxRenameActorCommand.h"
#include "FxSelectCommand.h"
#include "FxCurrentTimeCommand.h"
#include "FxExecCommand.h"
#include "FxNodeGroupCommand.h"
#include "FxPhonemeListCommand.h"
#include "FxGraphCommand.h"
#include "FxMapCommand.h"
#include "FxBatchCommand.h"
#include "FxKeyCommand.h"
#include "FxAnimGroupCommand.h"
#include "FxAnimCommand.h"
#include "FxCurveCommand.h"
#include "FxAnalysisCommand.h"
#include "FxGetCommand.h"
#include "FxSetCommand.h"
#include "FxPlaybackCommands.h"
#include "FxExportLTFCommand.h"
#include "FxTemplateCommand.h"
#include "FxAnimSetCommand.h"
#include "FxCookCommand.h"
#include "FxCoordSysCommand.h"
#include "FxSetNameCommand.h"
#include "FxCollapseFaceGraphCommand.h"

#ifdef __UNREAL__
	// Include the needed Unreal headers.
	#include "FxUE3Command.h"
	// Include the needed Unreal implementations.
	#include "FxUE3Command.cpp"
#endif // __UNREAL__

#include "FxVM.h"

namespace OC3Ent
{

namespace Face
{

void FxInternalCommands::RegisterAll( void )
{
	// Register the classes.
	FxDeveloperCommand::StaticClass();
	FxEatMemCommand::StaticClass();
	FxUndoCommand::StaticClass();
	FxRedoCommand::StaticClass();
	FxFlushUndoCommand::StaticClass();
	FxExitCommand::StaticClass();
	FxPrintCommand::StaticClass();
	FxWarnCommand::StaticClass();
	FxErrorCommand::StaticClass();
	FxNewActorCommand::StaticClass();
	FxLoadActorCommand::StaticClass();
	FxCloseActorCommand::StaticClass();
	FxSaveActorCommand::StaticClass();
	FxRenameCommand::StaticClass();
	FxRenameActorCommand::StaticClass();
	FxSelectCommand::StaticClass();
	FxCurrentTimeCommand::StaticClass();
	FxExecCommand::StaticClass();
	FxNodeGroupCommand::StaticClass();
	FxPhonemeListCommand::StaticClass();
	FxGraphCommand::StaticClass();
	FxMapCommand::StaticClass();
	FxBeginBatchCommand::StaticClass();
	FxExecBatchCommand::StaticClass();
	FxBatchCommand::StaticClass();
	FxKeyCommand::StaticClass();
	FxAnimGroupCommand::StaticClass();
	FxAnimCommand::StaticClass();
	FxCurveCommand::StaticClass();
	FxAnalysisCommand::StaticClass();
	FxGetCommand::StaticClass();
	FxSetCommand::StaticClass();
	FxPlayCommand::StaticClass();
	FxStopCommand::StaticClass();
	FxExportLTFCommand::StaticClass();
	FxTemplateCommand::StaticClass();
	FxAnimSetCommand::StaticClass();
	FxCookCommand::StaticClass();
	FxCoordSysCommand::StaticClass();
	FxSetNameCommand::StaticClass();
	FxCollapseFaceGraphCommand::StaticClass();
#ifdef __UNREAL__
	FxUE3Command::StaticClass();
#endif // __UNREAL__

	// Register the commands.
	FxVM::RegisterCommand(FxString("developer"),   FxDeveloperCommand::CreateSyntax(),			&FxDeveloperCommand::ConstructObject);
	FxVM::RegisterCommand(FxString("eatMem"),      FxEatMemCommand::CreateSyntax(),				&FxEatMemCommand::ConstructObject);
	FxVM::RegisterCommand(FxString("undo"),        FxUndoCommand::CreateSyntax(),				&FxUndoCommand::ConstructObject);
	FxVM::RegisterCommand(FxString("redo"),        FxRedoCommand::CreateSyntax(),				&FxRedoCommand::ConstructObject);
	FxVM::RegisterCommand(FxString("flushUndo"),   FxFlushUndoCommand::CreateSyntax(),			&FxFlushUndoCommand::ConstructObject);
	FxVM::RegisterCommand(FxString("exit"),        FxExitCommand::CreateSyntax(),				&FxExitCommand::ConstructObject);
	FxVM::RegisterCommand(FxString("quit"),        FxExitCommand::CreateSyntax(),				&FxExitCommand::ConstructObject);
	FxVM::RegisterCommand(FxString("print"),       FxPrintCommand::CreateSyntax(),				&FxPrintCommand::ConstructObject);
	FxVM::RegisterCommand(FxString("warn"),        FxWarnCommand::CreateSyntax(),				&FxWarnCommand::ConstructObject);
	FxVM::RegisterCommand(FxString("error"),       FxErrorCommand::CreateSyntax(),				&FxErrorCommand::ConstructObject);
	FxVM::RegisterCommand(FxString("newActor"),    FxNewActorCommand::CreateSyntax(),			&FxNewActorCommand::ConstructObject);
	FxVM::RegisterCommand(FxString("loadActor"),   FxLoadActorCommand::CreateSyntax(),			&FxLoadActorCommand::ConstructObject);
	FxVM::RegisterCommand(FxString("closeActor"),  FxCloseActorCommand::CreateSyntax(),			&FxCloseActorCommand::ConstructObject);
	FxVM::RegisterCommand(FxString("saveActor"),   FxSaveActorCommand::CreateSyntax(),			&FxSaveActorCommand::ConstructObject);
	FxVM::RegisterCommand(FxString("rename"),      FxRenameCommand::CreateSyntax(),				&FxRenameCommand::ConstructObject);
	FxVM::RegisterCommand(FxString("renameActor"), FxRenameActorCommand::CreateSyntax(),		&FxRenameActorCommand::ConstructObject);
	FxVM::RegisterCommand(FxString("select"),      FxSelectCommand::CreateSyntax(),				&FxSelectCommand::ConstructObject);
	FxVM::RegisterCommand(FxString("currentTime"), FxCurrentTimeCommand::CreateSyntax(),		&FxCurrentTimeCommand::ConstructObject);
	FxVM::RegisterCommand(FxString("exec"),        FxExecCommand::CreateSyntax(),				&FxExecCommand::ConstructObject);
    FxVM::RegisterCommand(FxString("nodeGroup"),   FxNodeGroupCommand::CreateSyntax(),			&FxNodeGroupCommand::ConstructObject);
	FxVM::RegisterCommand(FxString("phonList"),    FxPhonemeListCommand::CreateSyntax(),		&FxPhonemeListCommand::ConstructObject);
	FxVM::RegisterCommand(FxString("graph"),       FxGraphCommand::CreateSyntax(),				&FxGraphCommand::ConstructObject);
	FxVM::RegisterCommand(FxString("map"),         FxMapCommand::CreateSyntax(),				&FxMapCommand::ConstructObject);
	FxVM::RegisterCommand(FxString("batch"),       FxBeginBatchCommand::CreateSyntax(),			&FxBeginBatchCommand::ConstructObject);
	FxVM::RegisterCommand(FxString("execBatch"),   FxExecBatchCommand::CreateSyntax(),			&FxExecBatchCommand::ConstructObject);
	FxVM::RegisterCommand(FxString("key"),         FxKeyCommand::CreateSyntax(),				&FxKeyCommand::ConstructObject);
	FxVM::RegisterCommand(FxString("animGroup"),   FxAnimGroupCommand::CreateSyntax(),			&FxAnimGroupCommand::ConstructObject);
	FxVM::RegisterCommand(FxString("anim"),        FxAnimCommand::CreateSyntax(),				&FxAnimCommand::ConstructObject);
	FxVM::RegisterCommand(FxString("curve"),       FxCurveCommand::CreateSyntax(),				&FxCurveCommand::ConstructObject);
	FxVM::RegisterCommand(FxString("analyze"),     FxAnalysisCommand::CreateSyntax(),			&FxAnalysisCommand::ConstructObject);
	FxVM::RegisterCommand(FxString("get"),         FxGetCommand::CreateSyntax(),				&FxGetCommand::ConstructObject);
	FxVM::RegisterCommand(FxString("set"),         FxSetCommand::CreateSyntax(),				&FxSetCommand::ConstructObject);
	FxVM::RegisterCommand(FxString("play"),        FxPlayCommand::CreateSyntax(),				&FxPlayCommand::ConstructObject);
	FxVM::RegisterCommand(FxString("stop"),        FxStopCommand::CreateSyntax(),				&FxStopCommand::ConstructObject);
	FxVM::RegisterCommand(FxString("exportLTF"),   FxExportLTFCommand::CreateSyntax(),			&FxExportLTFCommand::ConstructObject);
	FxVM::RegisterCommand(FxString("template"),    FxTemplateCommand::CreateSyntax(),			&FxTemplateCommand::ConstructObject);
	FxVM::RegisterCommand(FxString("animSet"),     FxAnimSetCommand::CreateSyntax(),			&FxAnimSetCommand::ConstructObject);
	FxVM::RegisterCommand(FxString("cook"),        FxCookCommand::CreateSyntax(),				&FxCookCommand::ConstructObject);
	FxVM::RegisterCommand(FxString("coordsys"),    FxCoordSysCommand::CreateSyntax(),			&FxCoordSysCommand::ConstructObject);
	FxVM::RegisterCommand(FxString("setName"),     FxSetNameCommand::CreateSyntax(),			&FxSetNameCommand::ConstructObject);
	FxVM::RegisterCommand(FxString("fgcollapse"),  FxCollapseFaceGraphCommand::CreateSyntax(),	&FxCollapseFaceGraphCommand::ConstructObject);
#ifdef __UNREAL__
	FxVM::RegisterCommand(FxString("ue3"),         FxUE3Command::CreateSyntax(),				&FxUE3Command::ConstructObject);
#endif // __UNREAL__
}

void FxInternalCommands::UnregisterAll( void )
{
	// Unregister the commands.
	FxVM::UnregisterCommand(FxString("developer"));
	FxVM::UnregisterCommand(FxString("eatMem"));
	FxVM::UnregisterCommand(FxString("undo"));
	FxVM::UnregisterCommand(FxString("redo"));
	FxVM::UnregisterCommand(FxString("flushUndo"));
	FxVM::UnregisterCommand(FxString("exit"));
	FxVM::UnregisterCommand(FxString("quit"));
	FxVM::UnregisterCommand(FxString("print"));
	FxVM::UnregisterCommand(FxString("warn"));
	FxVM::UnregisterCommand(FxString("error"));
	FxVM::UnregisterCommand(FxString("newActor"));
	FxVM::UnregisterCommand(FxString("loadActor"));
	FxVM::UnregisterCommand(FxString("closeActor"));
	FxVM::UnregisterCommand(FxString("saveActor"));
	FxVM::UnregisterCommand(FxString("rename"));
	FxVM::UnregisterCommand(FxString("renameActor"));
	FxVM::UnregisterCommand(FxString("select"));
	FxVM::UnregisterCommand(FxString("currentTime"));
	FxVM::UnregisterCommand(FxString("exec"));
    FxVM::UnregisterCommand(FxString("nodeGroup"));
	FxVM::UnregisterCommand(FxString("phonList"));
	FxVM::UnregisterCommand(FxString("graph"));
	FxVM::UnregisterCommand(FxString("map"));
	FxVM::UnregisterCommand(FxString("batch"));
	FxVM::UnregisterCommand(FxString("execBatch"));
	FxVM::UnregisterCommand(FxString("key"));
	FxVM::UnregisterCommand(FxString("animGroup"));
	FxVM::UnregisterCommand(FxString("anim"));
	FxVM::UnregisterCommand(FxString("curve"));
	FxVM::UnregisterCommand(FxString("analyze"));
	FxVM::UnregisterCommand(FxString("get"));
	FxVM::UnregisterCommand(FxString("set"));
	FxVM::UnregisterCommand(FxString("play"));
	FxVM::UnregisterCommand(FxString("stop"));
	FxVM::UnregisterCommand(FxString("exportLTF"));
	FxVM::UnregisterCommand(FxString("template"));
	FxVM::UnregisterCommand(FxString("animSet"));
	FxVM::UnregisterCommand(FxString("cook"));
	FxVM::UnregisterCommand(FxString("coordsys"));
	FxVM::UnregisterCommand(FxString("setName"));
	FxVM::UnregisterCommand(FxString("fgcollapse"));
#ifdef __UNREAL__
	FxVM::UnregisterCommand(FxString("ue3"));
#endif // __UNREAL__
}

} // namespace Face

} // namespace OC3Ent
