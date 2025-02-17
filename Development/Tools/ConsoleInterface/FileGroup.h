#pragma once

using namespace System;
using namespace System::Xml::Serialization;

namespace ConsoleInterface
{
	// forward declarations
	ref class FileSet;

	/// <summary>
	/// Group of files and tags associated with them for filtering syncing
	/// </summary>
	[Serializable]
	public ref class FileGroup
	{
	public:
		/// <summary>
		/// If true, this file will be synced to the target console (for files that need syncing, but don't want to be in the TOC)
		/// </summary>
		[XmlAttribute]
		bool bIsForSync;

		/// <summary>
		/// If true, this file will be put in the TOC
		/// </summary>
		[XmlAttribute]
		bool bIsForTOC;

		/// <summary>
		/// Tag for this group.
		/// </summary>
		[XmlAttribute]
		String ^Tag;

		/// <summary>
		/// Platform for this group. Can also be * for any (same as null), or Console for non-PC platforms
		/// </summary>
		[XmlAttribute]
		String ^Platform;

		/// <summary>
		/// List of file infos for the group
		/// </summary>
		[XmlArray]
		array<FileSet^> ^Files;

	public:
		FileGroup();
	};
}