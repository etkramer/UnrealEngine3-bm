#include "Stdafx.h"
#include "FileSet.h"
#include "FileFilter.h"

namespace ConsoleInterface
{
	FileSet::FileSet() 
		: Path(nullptr), bIsRecursive(false), FileFilters(gcnew array<FileFilter^>(0))
	{

	}
}