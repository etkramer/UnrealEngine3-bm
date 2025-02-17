#include "Stdafx.h"
#include "SharedSettings.h"
#include "TagSet.h"
#include "Tag.h"
#include "FileGroup.h"

namespace ConsoleInterface
{
	SharedSettings::SharedSettings() 
		: TagSets(gcnew array<TagSet^>(0)), KnownLanguages(gcnew array<Tag^>(0)), KnownPlatforms(gcnew array<Tag^>(0)),	FileGroups(gcnew array<FileGroup^>(0))
	{
	}
}