#include "Stdafx.h"
#include "FileGroup.h"
#include "FileSet.h"

namespace ConsoleInterface
{
	FileGroup::FileGroup() 
		: bIsForSync(true), bIsForTOC(true), Tag(String::Empty), Platform(nullptr), Files(gcnew array<FileSet^>(0))
	{
	}
}