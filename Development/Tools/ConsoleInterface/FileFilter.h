#pragma once

using namespace System;
using namespace System::Xml::Serialization;

namespace ConsoleInterface
{
	[Serializable]
	public ref class FileFilter
	{
	public:
		[XmlAttribute]
		String ^Name;

	public:
		FileFilter();
	};
}