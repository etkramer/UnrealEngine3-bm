#pragma once

using namespace System;
using namespace System::Xml::Serialization;

namespace ConsoleInterface
{
	/// <summary>
	/// Holder struct for a generic tag
	/// </summary>
	[Serializable]
	public ref class Tag
	{
	public:
		/// <summary>
		/// The name of the tag
		/// </summary>
		[XmlAttribute]
		String ^Name;

		/// <summary>
		/// Optional alternative for this tag
		/// </summary>
		[XmlAttribute]
		String ^Alt;

	public:
		Tag();
	};
}