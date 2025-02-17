#pragma once

using namespace System;
using namespace System::Xml::Serialization;

namespace ConsoleInterface
{
	//forward declarations
	ref class FileFilter;

	/// <summary>
	/// Information about a file or set of files in a file group
	/// </summary>
	[Serializable]
	public ref class FileSet
	{
	public:
		/// <summary>
		/// Wildcard that specifies some files to sync
		/// </summary>
		[XmlAttribute]
		String ^Path;

		/// <summary>
		/// Should this set of files to a recursive sync
		/// </summary>
		[XmlAttribute]
		bool bIsRecursive;

		/// <summary>
		/// Optional filter to apply to files (array of filenames) to not copy them
		/// </summary>
		[XmlArray]
		array<FileFilter^> ^FileFilters;

	public:
		FileSet();
	};
}