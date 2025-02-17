using System;
using System.Collections.Generic;
using System.Text;
using System.Drawing;

namespace UnrealControls
{
	/// <summary>
	/// Represents a segment of colored text within a document.
	/// </summary>
	public struct ColoredDocumentLineSegment
	{
		Color? mColor;
		string mText;

		/// <summary>
		/// Gets the color of the text.
		/// </summary>
		public Color? Color
		{
			get { return mColor; }
		}

		/// <summary>
		/// Gets the text associated with the segment.
		/// </summary>
		public string Text
		{
			get { return mText; }
		}

		/// <summary>
		/// Constructor.
		/// </summary>
		/// <param name="Color">The color of the segment.</param>
		/// <param name="Text">The text associated with the segment.</param>
		public ColoredDocumentLineSegment(Color? Color, string Text)
		{
			if(Text == null)
			{
				throw new ArgumentNullException("Text");
			}

			mColor = Color;
			mText = Text;
		}
	}
}
