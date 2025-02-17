#include "Stdafx.h"
#include "GameSettings.h"
#include "FileGroup.h"

namespace ConsoleInterface
{
	GameSettings::GameSettings()
		: FileGroups(gcnew array<FileGroup^>(0)), XGDFileRelativePath(String::Empty), PS3TitleID(String::Empty)
	{
	}
}