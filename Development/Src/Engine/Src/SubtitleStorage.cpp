/*=============================================================================
	SubtitleStorage.cpp: Utility for loading subtitle text
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "SubtitleStorage.h"

FSubtitleStorage::FSubtitleStorage()
: ActiveMovie(-1),
  ActiveTip(-1)
{
}

FSubtitleStorage::~FSubtitleStorage()
{
}

void FSubtitleStorage::Load( FString const& Filename )
{
	Movies.Empty();
	Add( Filename );
}

void FSubtitleStorage::Add( FString const& Filename )
{
	FFilename Path = Filename;
	FFilename LocPath = Path;

	FString LangExt(appGetLanguageExt());
	if( LangExt != TEXT("INT") )
	{
		FString LocFilename = LocPath.GetLocalizedFilename(*LangExt);		
		if( GFileManager->FileSize(*LocFilename) != -1 )
		{
			LocPath = LocFilename;
		}
	}

	if (GFileManager->FileSize(*LocPath) == -1)
	{
		return;
	}

	TArray<BYTE> Storage;
	if (!appLoadFileToArray(Storage, *LocPath))
	{
		return;
	}

	// Add the NUL terminator.
	Storage.AddItem(0);

	FSubtitleMovie Movie;

	Movie.MovieName = Path.GetBaseFilename();
	Movie.RandomSelect = FALSE;
	Movie.RandomSelectCycleFrequency = 0;

	UBOOL bFirstLine = TRUE;
	UBOOL bIsDone = FALSE;
	UINT TimeScale = 1000; // By default the values are specified in ms.
	ANSICHAR* Ptr = reinterpret_cast<ANSICHAR*>(Storage.GetData());

	while ( *Ptr )
	{
		// Advance past any new line characters
		while ( *Ptr && (*Ptr=='\r' || *Ptr=='\n') )
		{
			Ptr++;
		}

		// Store the location of the first character of this line
		ANSICHAR* Start = Ptr;

		// Advance the char pointer until we hit a newline character, or EOF
		while ( *Ptr && *Ptr != '\r' && *Ptr != '\n' )
		{
			Ptr++;
		}

		// Terminate the current line, and advance the pointer to the next character in the stream
		if ( *Ptr )
		{
			*Ptr++ = 0;
		}

		if ( bFirstLine )
		{
			// parse the line of text
			INT Matched = appSSCANF(ANSI_TO_TCHAR(Start), TEXT("%u %u"), &TimeScale, &Movie.RandomSelectCycleFrequency);
			if ( Matched )
			{
				bFirstLine = FALSE;
			}
		}
		else
		{
			UINT StartTime;
			UINT StopTime;
			TCHAR Subtitle[1024];
			appMemzero(Subtitle, sizeof(Subtitle));

			if ( strlen(Start) < ARRAY_COUNT(Subtitle) )
			{
				// parse the line of text
#if PS3
				if( appSSCANF(ANSI_TO_TCHAR(Start), TEXT("%u,%u,%ls"), &StartTime, &StopTime, Subtitle) == 3 )
				{
#else
				if( appSSCANF(ANSI_TO_TCHAR(Start), TEXT("%u,%u,%s"), &StartTime, &StopTime, Subtitle) == 3 )
				{
#endif
					if (Subtitle[0])
					{
						FSubtitleKeyFrame KeyFrame;
						KeyFrame.StartTime = StartTime * 1000 / TimeScale;
						KeyFrame.StopTime  = StopTime * 1000 / TimeScale;
						KeyFrame.Subtitle  = FString(Subtitle);

						Movie.KeyFrames.Push(KeyFrame);

						// If start and end times are both zero - enable random selection
						if( StartTime == 0 && StopTime == 0 )
						{
							Movie.RandomSelect = TRUE;
						}
					}
				}
			}
		}
	}

	Movies.Push(Movie);
}

void FSubtitleStorage::ActivateMovie( FString const& MovieName )
{
	FString LookupName = FFilename(MovieName).GetBaseFilename();

	for (INT MovieIndex = 0; MovieIndex < Movies.Num(); ++MovieIndex)
	{
		FSubtitleMovie const& Movie = Movies(MovieIndex);
		if (LookupName == Movie.MovieName)
		{
			ActiveMovie = MovieIndex;
			// Initialize random number generator.
			if( !GIsBenchmarking && !ParseParam(appCmdLine(),TEXT("FIXEDSEED")) )
			{
				appRandInit( appCycles() );
			}
			ActiveTip = ( appRand() * Movie.KeyFrames.Num() ) / RAND_MAX;
			NextRandomSelectCycleTime = Movie.RandomSelectCycleFrequency;
			LastSubtitleTime = 0;
			return;
		}
	}

	ActiveTip = -1;
	ActiveMovie = -1;
}

FString FSubtitleStorage::LookupSubtitle( UINT Time )
{
	if ((ActiveMovie != -1) && (ActiveMovie < Movies.Num()))
	{
		FSubtitleMovie& Movie = Movies(ActiveMovie);

		if( Movie.RandomSelect )
		{
			if( ActiveTip > -1 )
			{
				if ( LastSubtitleTime > Time )
				{
					// adjust for the loop case
					// this isn't perfect, because it loses the time between the LastSubtitleTime
					// and the end of the movie, but it's close enough.
					NextRandomSelectCycleTime = (NextRandomSelectCycleTime - LastSubtitleTime);
				}

				if ( (Movie.RandomSelectCycleFrequency > 0) && (Time > NextRandomSelectCycleTime) )
				{
					INT Delta = ( appRand() * (Movie.KeyFrames.Num() - 1) ) / RAND_MAX;
					ActiveTip = (ActiveTip + Delta) % Movie.KeyFrames.Num();
					NextRandomSelectCycleTime += Movie.RandomSelectCycleFrequency;
				}

				LastSubtitleTime = Time;

				FSubtitleKeyFrame const& KeyFrame = Movie.KeyFrames( ActiveTip );
				return KeyFrame.Subtitle;
			}
		}
		else
		{
			for (INT FrameIndex = 0; FrameIndex < Movie.KeyFrames.Num(); ++FrameIndex)
			{
				FSubtitleKeyFrame const& KeyFrame = Movie.KeyFrames(FrameIndex);
				if (KeyFrame.StartTime > Time)
				{
					break;
				}

				if (KeyFrame.StartTime <= Time && Time <= KeyFrame.StopTime)
				{
					return KeyFrame.Subtitle;
				}
			}
		}
	}

	return FString();
}



