#include "Stdafx.h"
#include "TagSet.h"
#include "Tag.h"

namespace ConsoleInterface
{
	TagSet::TagSet() : Name(String::Empty), Tags(gcnew array<Tag^>(0))
	{

	}
}