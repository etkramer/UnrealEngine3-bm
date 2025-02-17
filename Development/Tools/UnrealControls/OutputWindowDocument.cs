using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.IO;
using System.Drawing;

namespace UnrealControls
{
	/// <summary>
	/// A document that can be viewed by multiple <see cref="OutputWindowView"/>'s.
	/// </summary>
	public class OutputWindowDocument
	{
		/// <summary>
		/// Represents a line of colored text within a document.
		/// </summary>
		class DocumentLine
		{
			/// <summary>
			/// Represents a range of colored characters within a line of a document.
			/// </summary>
			class ColoredTextRange
			{
				public Color? mColor;
				public int mStartIndex;
				public int mLength;

				/// <summary>
				/// Constructor.
				/// </summary>
				/// <param name="TxtColor">The color of the range.</param>
				/// <param name="StartIndex">The start index within the range within a line of text.</param>
				/// <param name="Length">The number of characters the range encompasses.</param>
				public ColoredTextRange(Color? TxtColor, int StartIndex, int Length)
				{
					if(Length < 0)
					{
						throw new ArgumentOutOfRangeException("Length");
					}

					if(StartIndex < 0)
					{
						throw new ArgumentOutOfRangeException("StartIndex");
					}

					mColor = TxtColor;
					mStartIndex = StartIndex;
					mLength = Length;
				}
			}

			StringBuilder mText = new StringBuilder();
			List<ColoredTextRange> mColorRanges = new List<ColoredTextRange>();

			/// <summary>
			/// Gets the length of the line in characters.
			/// </summary>
			public int Length
			{
				get { return mText.Length; }
			}

			/// <summary>
			/// Gets the character at the specified index.
			/// </summary>
			/// <param name="Index">The index of the character to get.</param>
			/// <returns>The character at the specified index.</returns>
			public char this[int Index]
			{
				get { return mText[Index]; }
			}

			/// <summary>
			/// Appends a character to the line of text.
			/// </summary>
			/// <param name="TxtColor">The color of the character.</param>
			/// <param name="CharToAppend">The character to append.</param>
			public void Append(Color? TxtColor, char CharToAppend)
			{
				if(mColorRanges.Count == 0)
				{
					mColorRanges.Add(new ColoredTextRange(TxtColor, 0, 1));
				}
				else
				{
					int CurColorIndex = mColorRanges.Count - 1;

					if(mColorRanges[CurColorIndex].mColor != TxtColor)
					{
						mColorRanges.Add(new ColoredTextRange(TxtColor, mText.Length, 1));
					}
					else
					{
						++mColorRanges[CurColorIndex].mLength;
					}
				}

				mText.Append(CharToAppend);
			}

			/// <summary>
			/// Appends text to the end of the line.
			/// </summary>
			/// <param name="TxtColor">The color of the text.</param>
			/// <param name="TxtToAppend">The text to append.</param>
			public void Append(Color? TxtColor, string TxtToAppend)
			{
				if(mColorRanges.Count == 0)
				{
					mColorRanges.Add(new ColoredTextRange(TxtColor, 0, TxtToAppend.Length));
				}
				else
				{
					int CurColorIndex = mColorRanges.Count - 1;

					if(mColorRanges[CurColorIndex].mColor != TxtColor)
					{
						mColorRanges.Add(new ColoredTextRange(TxtColor, mText.Length, TxtToAppend.Length));
					}
					else
					{
						mColorRanges[CurColorIndex].mLength += TxtToAppend.Length;
					}
				}

				mText.Append(TxtToAppend);
			}

			/// <summary>
			/// Gets an array of colored line segments that can be used for drawing.
			/// </summary>
			/// <returns>An array of colored line segments.</returns>
			public ColoredDocumentLineSegment[] GetLineSegments()
			{
				ColoredDocumentLineSegment[] Segments = new ColoredDocumentLineSegment[mColorRanges.Count];
				int CurSegment = 0;

				foreach(ColoredTextRange CurRange in mColorRanges)
				{
					Segments[CurSegment] = new ColoredDocumentLineSegment(CurRange.mColor, mText.ToString(CurRange.mStartIndex, CurRange.mLength));
				}

				return Segments;
			}

			/// <summary>
			/// Gets an array of colored line segments that can be used for drawing.
			/// </summary>
			/// <param name="StartIndex">The starting character at which to begin generating segments.</param>
			/// <param name="Length">The number of characters to include in segment generation.</param>
			/// <returns>An array of colored line segments.</returns>
			public ColoredDocumentLineSegment[] GetLineSegments(int StartIndex, int Length)
			{
				List<ColoredDocumentLineSegment> Segments = new List<ColoredDocumentLineSegment>(mColorRanges.Count);
				int CharsToCopy = 0;
				bool bFoundStartSegment = false;

				foreach(ColoredTextRange CurRange in mColorRanges)
				{
					if(bFoundStartSegment)
					{
						CharsToCopy = Math.Min(Length, CurRange.mLength);

						Segments.Add(new ColoredDocumentLineSegment(CurRange.mColor, mText.ToString(CurRange.mStartIndex, CharsToCopy)));

						Length -= CharsToCopy;

						if(Length <= 0)
						{
							break;
						}
					}
					else
					{
						if(CurRange.mStartIndex <= StartIndex)
						{
							bFoundStartSegment = true;
							CharsToCopy = Math.Min(Length, CurRange.mLength);

							Segments.Add(new ColoredDocumentLineSegment(CurRange.mColor, mText.ToString(Math.Max(StartIndex, CurRange.mStartIndex), CharsToCopy)));

							Length -= CharsToCopy;

							if(Length <= 0)
							{
								break;
							}
						}
					}
				}

				return Segments.ToArray();
			}

			/// <summary>
			/// Returns the text associated with the line.
			/// </summary>
			/// <returns>The text associated with the line.</returns>
			public override string ToString()
			{
				return mText.ToString();
			}

			/// <summary>
			/// Returns the text associated with the line.
			/// </summary>
			/// <param name="StartIndex">The character index that the returned text will start at.</param>
			/// <param name="Length">The number of characters to return.</param>
			/// <returns>A string containing the text within the specified range of the line.</returns>
			public string ToString(int StartIndex, int Length)
			{
				return mText.ToString(StartIndex, Length);
			}
		}

		List<DocumentLine> mLines = new List<DocumentLine>();
		LinkedList<OutputWindowView> mViews = new LinkedList<OutputWindowView>();
		int mLongestLineLength;

		/// <summary>
		/// Gets/Sets the text in the document.
		/// </summary>
		public string Text
		{
			set
			{
				if(value == null)
				{
					throw new ArgumentNullException("value");
				}

				Clear();

				AppendText(null, value);
			}
			get
			{
				StringBuilder Bldr = new StringBuilder();

				foreach(DocumentLine CurLine in mLines)
				{
					Bldr.Append(CurLine.ToString());
				}

				return Bldr.ToString();
			}
		}

		/// <summary>
		/// Gets the lines associated with the document.
		/// </summary>
		public string[] Lines
		{
			get
			{
				string[] Ret = new string[mLines.Count];

				for(int i = 0; i < mLines.Count; ++i)
				{
					Ret[i] = mLines[i].ToString();
				}

				return Ret;
			}
		}

		/// <summary>
		/// Gets the number of lines in the document.
		/// </summary>
		public int LineCount
		{
			get { return mLines.Count; }
		}

		/// <summary>
		/// Gets the length in characters of the longest line in the document.
		/// </summary>
		public int LongestLineLength
		{
			get { return mLongestLineLength; }
		}

		/// <summary>
		/// Gets the <see cref="TextLocation"/> of the beginning of the document.
		/// </summary>
		public TextLocation BeginningOfDocument
		{
			get { return new TextLocation(0, 0); }
		}

		/// <summary>
		/// Gets the <see cref="TextLocation"/> of the end of the document.
		/// </summary>
		public TextLocation EndOfDocument
		{
			get { return new TextLocation(mLines.Count - 1, GetLineLength(mLines.Count - 1)); }
		}

		/// <summary>
		/// Appends text of the specified color to the document.
		/// </summary>
		/// <param name="TxtColor">The color of the text to be appended</param>
		/// <param name="Txt">The text to be appended</param>
		public void AppendText(Color? TxtColor, string Txt)
		{
			DocumentLine CurLine;
			int CurLineLength = 0;

			if(mLines.Count == 0)
			{
				CurLine = new DocumentLine();
				mLines.Add(CurLine);
			}
			else
			{
				CurLine = mLines[mLines.Count - 1];
			}

			char LastChar = (char)0;

			foreach(char CurChar in Txt)
			{
				if(CurChar == '\n' && LastChar != '\r')
				{
					CurLine.Append(TxtColor, '\r');
				}

				// replace tabs with 4 spaces
				if(CurChar == '\t')
				{
					CurLine.Append(TxtColor, "    ");
				}
				else
				{
					CurLine.Append(TxtColor, CurChar);
				}

				if(CurChar == '\n')
				{
					// only count displayable characters (cut \r\n)
					CurLineLength = CurLine.Length - 2;

					if(CurLineLength > mLongestLineLength)
					{
						mLongestLineLength = CurLineLength;
					}

					CurLine = new DocumentLine();
					mLines.Add(CurLine);
				}

				LastChar = CurChar;
			}

			// only count displayable characters (cut \r\n)
			CurLineLength = CurLine.Length;

			if(CurLineLength > mLongestLineLength)
			{
				mLongestLineLength = CurLineLength;
			}

			UpdateViews();
		}

		/// <summary>
		/// Appends a line of colored text to the document.
		/// </summary>
		/// <param name="TxtColor">The color of the text to be appended.</param>
		/// <param name="Txt">The text to append.</param>
		public void AppendLine(Color? TxtColor, string Txt)
		{
			AppendText(TxtColor, Txt + Environment.NewLine);
		}

		/// <summary>
		/// Retrieves a line of text from the document.
		/// </summary>
		/// <param name="Index">The index of the line to retrieve.</param>
		/// <param name="bIncludeEOL">True if the line includes the EOL characters.</param>
		/// <returns>A string containing the text of the specified line.</returns>
		public string GetLine(int Index, bool bIncludeEOL)
		{
			DocumentLine Line = mLines[Index];

			if(!bIncludeEOL && Line.Length >= 2 && Line[Line.Length - 1] == '\n' && Line[Line.Length - 2] == '\r')
			{
				return Line.ToString(0, Line.Length - 2);
			}

			return Line.ToString();
		}

		/// <summary>
		/// Retrieves a line of text from the document.
		/// </summary>
		/// <param name="LineIndex">The index of the line to retrieve.</param>
		/// <param name="CharOffset">The character to start copying.</param>
		/// <param name="Length">The number of characters to copy.</param>
		/// <returns>A string containing the specified range of text text in the specified line.</returns>
		public string GetLine(int LineIndex, int CharOffset, int Length)
		{
			return mLines[LineIndex].ToString(CharOffset, Length);
		}

		/// <summary>
		/// Gets the length of the line in characters at the specified index.
		/// </summary>
		/// <param name="Index">The index of the line to retrieve length information for.</param>
		/// <returns>The length of the line in characters at the specified index.</returns>
		public int GetLineLength(int Index)
		{
			if(Index >= 0 && Index < mLines.Count)
			{
				DocumentLine Line = mLines[Index];

				if(Line.Length >= 2 && Line[Line.Length - 1] == '\n' && Line[Line.Length - 2] == '\r')
				{
					return Line.Length - 2;
				}
				else
				{
					return Line.Length;
				}
			}

			return 0;
		}

		/// <summary>
		/// Clears the document.
		/// </summary>
		public void Clear()
		{
			mLines.Clear();
			mLongestLineLength = 0;

			UpdateViews();
		}

		/// <summary>
		/// Tells all views that are registered with the document to refresh themselves.
		/// </summary>
		private void UpdateViews()
		{
			foreach(OutputWindowView View in mViews)
			{
				View.TriggerDocumentModified();
			}
		}

		/// <summary>
		/// Registers a view with the document.
		/// </summary>
		/// <param name="View">The view to be registered.</param>
		internal void RegisterView(OutputWindowView View)
		{
			if(!mViews.Contains(View))
			{
				mViews.AddLast(View);
			}
		}

		/// <summary>
		/// Unregisters a view with the document.
		/// </summary>
		/// <param name="View">The view to be unregistered.</param>
		internal void UnregisterView(OutputWindowView View)
		{
			mViews.Remove(View);
		}

		/// <summary>
		/// Finds the first occurence of the specified string within the document if it exists.
		/// </summary>
		/// <param name="Txt">The string to find.</param>
		/// <param name="StartLoc">The position within the document to begin searching.</param>
		/// <param name="EndLoc">The position within the document to end searching.</param>
		/// <param name="Flags">Flags telling the document how to conduct its search.</param>
		/// <param name="Result">The location within the document of the supplied text.</param>
		/// <returns>True if the text was found.</returns>
		public bool Find(string Txt, TextLocation StartLoc, TextLocation EndLoc, RichTextBoxFinds Flags, out FindResult Result)
		{
			if((Flags & RichTextBoxFinds.Reverse) == RichTextBoxFinds.Reverse)
			{
				return FindReverse(Txt, ref StartLoc, ref EndLoc, Flags, out Result);
			}
			else
			{
				return FindForward(Txt, ref StartLoc, ref EndLoc, Flags, out Result);
			}
		}

		/// <summary>
		/// Looks forward in the document from the specified location for the supplied text.
		/// </summary>
		/// <param name="Txt">The text to find.</param>
		/// <param name="StartLoc">The location to start searching from.</param>
		/// <param name="EndLoc">The location to stop searching at.</param>
		/// <param name="Flags">Flags that control how the search is performed.</param>
		/// <param name="Result">Receives the result.</param>
		/// <returns>True if a match was found.</returns>
		private bool FindForward(string Txt, ref TextLocation StartLoc, ref TextLocation EndLoc, RichTextBoxFinds Flags, out FindResult Result)
		{
			bool bMatchWord;
			bool bFound = false;
			bool bIsWord;
			StringComparison ComparisonFlags;

			SetupFindState(Txt, ref StartLoc, ref EndLoc, Flags, out Result, out bIsWord, out ComparisonFlags, out bMatchWord);

			for(int CurLineIndex = StartLoc.Line; CurLineIndex <= EndLoc.Line && !bFound; ++CurLineIndex)
			{
				if(GetLineLength(CurLineIndex) == 0)
				{
					continue;
				}

				DocumentLine CurLineBldr = mLines[CurLineIndex];
				string LineTxt;
				int ColumnIndex = 0;

				if(CurLineIndex == StartLoc.Line && StartLoc.Column > 0)
				{
					LineTxt = CurLineBldr.ToString(StartLoc.Column, CurLineBldr.Length - StartLoc.Column);
					ColumnIndex = StartLoc.Column;
				}
				else if(CurLineIndex == EndLoc.Line && EndLoc.Column < GetLineLength(CurLineIndex))
				{
					LineTxt = CurLineBldr.ToString(0, EndLoc.Column + 1);
				}
				else
				{
					LineTxt = CurLineBldr.ToString();
				}

				int Index = LineTxt.IndexOf(Txt, ComparisonFlags);

				if(Index != -1)
				{
					ColumnIndex += Index;
					CheckForWholeWord(Txt, ref Result, bMatchWord, ref bFound, bIsWord, CurLineIndex, CurLineBldr, ColumnIndex, Index);
				}
			}

			return bFound;
		}

		/// <summary>
		/// Checks to see if a text location is a matching word.
		/// </summary>
		/// <param name="Txt">The text being searched for.</param>
		/// <param name="Result">Receives the result if the text location is a matching word.</param>
		/// <param name="bMatchWord">True if an entire word is to be matched.</param>
		/// <param name="bFound">Set to true if a matching word is found.</param>
		/// <param name="bIsWord">True if <paramref name="Txt"/> is a valid word.</param>
		/// <param name="CurLineIndex">The index of the current line.</param>
		/// <param name="CurLineBldr">The text of the current line.</param>
		/// <param name="ColumnIndex">The character index within the line of text where the matching will begin.</param>
		/// <param name="Index">The index of a match within the range of searchable characters for the current line. The true line index is <paramref name="ColumnIndex"/> + <paramref name="Index"/>.</param>
		private static void CheckForWholeWord(string Txt, ref FindResult Result, bool bMatchWord, ref bool bFound, bool bIsWord, int CurLineIndex, DocumentLine CurLine, int ColumnIndex, int Index)
		{
			int FinalCharIndex = ColumnIndex + Txt.Length;

			if(bMatchWord && bIsWord)
			{
				if((FinalCharIndex >= CurLine.Length || !IsWordCharacter(CurLine[FinalCharIndex]))
					&& (ColumnIndex == 0 || !IsWordCharacter(CurLine[ColumnIndex - 1])))
				{
					bFound = true;
				}
			}
			else
			{
				bFound = true;
			}

			if(bFound)
			{
				Result.Line = CurLineIndex;
				Result.Column = ColumnIndex;
				Result.Length = Txt.Length;
			}
		}

		/// <summary>
		/// Checks to see if a character is valid within a word.
		/// </summary>
		/// <param name="CharToCheck">The character to check.</param>
		/// <returns>True if the character is valid within a word.</returns>
		private static bool IsWordCharacter(char CharToCheck)
		{
			return char.IsLetterOrDigit(CharToCheck) || CharToCheck == '_';
		}

		/// <summary>
		/// Performs general housekeeping for setting up a search.
		/// </summary>
		/// <param name="Txt">The text to search for.</param>
		/// <param name="StartLoc">The location to begin searching from.</param>
		/// <param name="EndLoc">The location to stop searching at.</param>
		/// <param name="Flags">Flags controlling how the search is performed.</param>
		/// <param name="Result">Receives the resulting location if a match is found.</param>
		/// <param name="bIsWord">Set to true if <paramref name="Txt"/> is a valid word.</param>
		/// <param name="ComparisonFlags">Receives flags controlling how strings are compared.</param>
		/// <param name="bMatchWord">Is set to true if only full words are to be matched.</param>
		private void SetupFindState(string Txt, ref TextLocation StartLoc, ref TextLocation EndLoc, RichTextBoxFinds Flags, out FindResult Result, out bool bIsWord, out StringComparison ComparisonFlags, out bool bMatchWord)
		{
			Result = FindResult.Empty;

			if(!IsValidTextLocation(StartLoc))
			{
				throw new ArgumentException("StartLoc is an invalid text location!");
			}

			if(!IsValidTextLocation(EndLoc))
			{
				throw new ArgumentException("EndLoc is an invalid text location!");
			}

			if((Flags & RichTextBoxFinds.Reverse) == RichTextBoxFinds.Reverse)
			{
				if(StartLoc < EndLoc)
				{
					throw new ArgumentException("StartLoc must be greater than EndLoc when doing a reverse search!");
				}
			}
			else
			{
				if(StartLoc > EndLoc)
				{
					throw new ArgumentException("StartLoc must be less than EndLoc when doing a forward search!");
				}
			}

			bMatchWord = (Flags & RichTextBoxFinds.WholeWord) == RichTextBoxFinds.WholeWord;
			bIsWord = IsWord(0, Txt);
			ComparisonFlags = StringComparison.OrdinalIgnoreCase;

			if((Flags & RichTextBoxFinds.MatchCase) == RichTextBoxFinds.MatchCase)
			{
				ComparisonFlags = StringComparison.Ordinal;
			}
		}

		/// <summary>
		/// Checks a string to see if it contains a valid word.
		/// </summary>
		/// <param name="Index">The index to start validating at.</param>
		/// <param name="Txt">The text to be validated.</param>
		/// <returns>True if all text including and after <paramref name="Index"/> are part of a valid word.</returns>
		private static bool IsWord(int Index, string Txt)
		{
			for(; Index < Txt.Length; ++Index)
			{
				char CurChar = Txt[Index];

				if(!char.IsLetterOrDigit(CurChar) && CurChar != '_')
				{
					return false;
				}
			}

			return true;
		}

		/// <summary>
		/// Searches for a string in the reverse direction of <see cref="FindForward"/>.
		/// </summary>
		/// <param name="Txt">The text to search for.</param>
		/// <param name="StartLoc">The starting location of the search.</param>
		/// <param name="EndLoc">The ending location of the search.</param>
		/// <param name="Flags">Flags controlling how the searching is conducted.</param>
		/// <param name="Result">Receives the results of the search.</param>
		/// <returns>True if a match was found.</returns>
		private bool FindReverse(string Txt, ref TextLocation StartLoc, ref TextLocation EndLoc, RichTextBoxFinds Flags, out FindResult Result)
		{
			bool bFound = false;
			bool bMatchWord;
			bool bIsWord;
			StringComparison ComparisonFlags;

			SetupFindState(Txt, ref StartLoc, ref EndLoc, Flags, out Result, out bIsWord, out ComparisonFlags, out bMatchWord);

			for(int CurLineIndex = StartLoc.Line; CurLineIndex >= EndLoc.Line && !bFound; --CurLineIndex)
			{
				if(GetLineLength(CurLineIndex) == 0)
				{
					continue;
				}

				DocumentLine CurLineBldr = mLines[CurLineIndex];
				string LineTxt;
				int ColumnIndex = 0;

				if(CurLineIndex == StartLoc.Line && StartLoc.Column < GetLineLength(CurLineIndex))
				{
					LineTxt = CurLineBldr.ToString(0, StartLoc.Column);
				}
				else if(CurLineIndex == EndLoc.Line && EndLoc.Column > 0)
				{
					LineTxt = CurLineBldr.ToString(EndLoc.Column, CurLineBldr.Length - EndLoc.Column);
					ColumnIndex = EndLoc.Column;
				}
				else
				{
					LineTxt = CurLineBldr.ToString();
				}

				int Index = LineTxt.LastIndexOf(Txt, ComparisonFlags);

				if(Index != -1)
				{
					ColumnIndex += Index;
					CheckForWholeWord(Txt, ref Result, bMatchWord, ref bFound, bIsWord, CurLineIndex, CurLineBldr, ColumnIndex, Index);
				}
			}

			return bFound;
		}

		/// <summary>
		/// Saves the document to disk.
		/// </summary>
		/// <param name="Name">The path to a file that the document will be written to.</param>
		public void SaveToFile(string Name)
		{
			using(StreamWriter Writer = new StreamWriter(File.Open(Name, FileMode.Create, FileAccess.Write, FileShare.Read)))
			{
				foreach(DocumentLine CurLine in mLines)
				{
					Writer.Write(CurLine.ToString());
				}
			}
		}

		/// <summary>
		/// Determines whether a location within the document is valid.
		/// </summary>
		/// <param name="Loc">The location to validate.</param>
		/// <returns>True if the supplied location is within the bounds of the document.</returns>
		public bool IsValidTextLocation(TextLocation Loc)
		{
			bool bResult = false;

			if(Loc.Line >= 0 && Loc.Line < mLines.Count && Loc.Column >= 0 && Loc.Column <= GetLineLength(Loc.Line))
			{
				bResult = true;
			}

			return bResult;
		}

		/// <summary>
		/// Gets an array of line segments for the line at the specified index.
		/// </summary>
		/// <param name="LineIndex">The index of the line to retrieve the segments for.</param>
		/// <returns>An array of line segments.</returns>
		public ColoredDocumentLineSegment[] GetLineSegments(int LineIndex)
		{
			return mLines[LineIndex].GetLineSegments();
		}

		/// <summary>
		/// Gets an array of line segments for the line at the specified index.
		/// </summary>
		/// <param name="LineIndex">The index of the line to retrieve the segments for.</param>
		/// <param name="StartIndex">The starting character at which to begin generating segments.</param>
		/// <param name="Length">The number of characters to include in segment generation.</param>
		/// <returns>An array of line segments.</returns>
		public ColoredDocumentLineSegment[] GetLineSegments(int LineIndex, int StartIndex, int Length)
		{
			return mLines[LineIndex].GetLineSegments(StartIndex, Length);
		}
	}
}