#pragma once

using namespace System;
using namespace System::Xml::Serialization;

namespace ConsoleInterface
{
	// forward declarations
	ref class Tag;

	/// <summary>
	/// Groups of tags so that a sync can be specified with one name, and do mutiple 
	/// FileGroup tags
	/// </summary>
	[Serializable]
	public ref class TagSet
	{
	public:
		/// <summary>
		/// Name of the TagSet
		/// </summary>
		[XmlAttribute]
		String ^Name;

		/// <summary>
		/// List of Tags
		/// </summary>
		[XmlArray]
		array<Tag^> ^Tags;

	public:
		TagSet();
	};
}