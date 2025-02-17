/*=============================================================================
	DiagnosticTable.h: Diagnostic table implementation
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __DIAGNOSTICTABLE_H__
#define __DIAGNOSTICTABLE_H__

#if ALLOW_DEBUG_FILES

/**
 * Encapsulates writing a table of data to a CSV file that is output by the engine for diagnostic purposes.
 */
class FDiagnosticTableWriterCSV
{
public:

	/** Initialization constructor. */
	FDiagnosticTableWriterCSV(FArchive* InOutputStream)
	:	OutputStream(InOutputStream)
	{}

	/** Destructor; closes the output stream. */
	~FDiagnosticTableWriterCSV()
	{
		// Ensure the user flushed the last row to the output.
		check(!CurrentRow.Len());

		// Close the output stream.
		Close();
	}

	/** Adds a column to the current row. */
	void AddColumn(const TCHAR* Format,...)
	{
		check(OutputStream);

		// Format the input string.
		TCHAR ColumnText[4096];
		GET_VARARGS(ColumnText,ARRAY_COUNT(ColumnText),ARRAY_COUNT(ColumnText)-1,Format,Format);

		// If this isn't the first column in the row, add a comma to separate the columns.
		if(CurrentRow.Len())
		{
			CurrentRow += TEXT(',');
		}

		// Replace quotes in the column with doubles quotes.
		FString EscapedText = ColumnText;
		EscapedText = EscapedText.Replace(TEXT("\""),TEXT("\"\""));

		// Append the enquoted column to the current row.
		CurrentRow += TEXT('\"');
		CurrentRow += EscapedText;
		CurrentRow += TEXT('\"');
	}

	/** Advances to the next row. */
	void CycleRow()
	{
		check(OutputStream);

		// Write the current row to the output stream.
		OutputStream->Logf(TEXT("%s"),*CurrentRow);

		// Reset the current row.
		CurrentRow.Empty();
	}

	/** Closes the output stream. */
	void Close()
	{
		if(OutputStream)
		{
			// Close the output stream.
			OutputStream->Close();
			OutputStream = NULL;
		}
	}

private:

	/** The contents of the current row. */
	FString CurrentRow;

	/** The stream the table is being written to. */
	FArchive* OutputStream;
};

/** Encapsulates a diagnostic table that is written to a temporary file and opened in a viewer. */
class FDiagnosticTableViewer : public FDiagnosticTableWriterCSV
{
public:

	/** Creates the viewer with a uniquely named temporary file. */
	static FString GetUniqueTemporaryFilePath(const TCHAR* BaseName)
	{
		return FString::Printf(TEXT("%sLogs%s%s-%s.csv"),*appGameDir(),PATH_SEPARATOR,BaseName,*appSystemTimeString());
	}

	/** Initialization constructor. */
	FDiagnosticTableViewer(const TCHAR* InTemporaryFilePath)
	:	FDiagnosticTableWriterCSV(GFileManager->CreateDebugFileWriter(InTemporaryFilePath))
	,	bHasOpenedViewer(FALSE)
	,	TemporaryFilePath(InTemporaryFilePath)
	{}

	/** Destructor. */
	~FDiagnosticTableViewer()
	{
		OpenViewer();
	}

	/** Forces the writer to close and opens the written table in the viewer. */
	void OpenViewer()
	{
		if(!bHasOpenedViewer)
		{
			bHasOpenedViewer = TRUE;

			// Close the writer.
			Close();

			// Open the viewer.
			appLaunchURL(*FString::Printf(TEXT("%s"),*GFileManager->ConvertToAbsolutePath(*TemporaryFilePath)));
		}
	}

private:
	
	/** True if the file has been opened in the viewer. */
	UBOOL bHasOpenedViewer;

	/** The filename used for the temporary file. */
	FString TemporaryFilePath;
};

#endif

#endif
