#pragma once

using namespace System;
using namespace System::Xml::Serialization;

namespace ConsoleInterface
{
	//forward declarations
	ref class TagSet;
	ref class Tag;
	ref class FileGroup;

	/// <summary>
	/// Summary description for per-platform CookerSettings.
	/// </summary>
	[Serializable]
	public ref class SharedSettings
	{
	public:
		/// <summary>
		/// List of TagSet objects
		/// </summary>
		[XmlArray]
		array<TagSet^> ^TagSets;

		/// <summary>
		/// List of known languages (should match the list in C++ land)
		/// </summary>
		[XmlArray]
		array<Tag^> ^KnownLanguages;

		/// <summary>
		/// List of known platforms
		/// </summary>
		[XmlArray]
		array<Tag^> ^KnownPlatforms;

		/// <summary>
		/// Set of file groups for syncing
		/// </summary>
		[XmlArray]
		array<FileGroup^> ^FileGroups;

	public:
		SharedSettings();
	};
}