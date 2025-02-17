/*=============================================================================
	UnString.h: Dynamic string definitions.  This needed to be UnString.h to avoid
				conflicting with the Windows platform SDK string.h
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __STRING_H__
#define __STRING_H__

#include "Array.h"

//
// A dynamically sizeable string.
//
class FString : protected TArray<TCHAR>
{
public:
	FString()
	: TArray<TCHAR>()
	{}
	FString( const FString& Other )
	: TArray<TCHAR>( Other.ArrayNum )
	{
		if( ArrayNum )
		{
			appMemcpy( GetData(), Other.GetData(), ArrayNum*sizeof(TCHAR) );
		}
	}
	// Allows slack to be specified
	FString( const FString& Other , INT ExtraSlack)
	{
		TArray<TCHAR>::Empty(Other.ArrayNum + ExtraSlack);
		if( Other.ArrayNum )
		{
			Add(Other.ArrayNum);
			appMemcpy( GetData(), Other.GetData(), ArrayNum*sizeof(TCHAR) );
		}
	}
	FString( const TCHAR* In )
	: TArray<TCHAR>( In && *In ? (appStrlen(In)+1) : 0 )
	{
		if( ArrayNum )
		{
			appMemcpy( GetData(), In, ArrayNum*sizeof(TCHAR) );
		}
	}
	explicit FString( INT InCount, const TCHAR* InSrc )
	:	TArray<TCHAR>( InCount ? InCount+1 : 0 )
	{
		if( ArrayNum )
		{
			appStrncpy( &(*this)(0), InSrc, InCount+1 );
		}
	}
	FString( const ANSICHAR* In )
	: TArray<TCHAR>( In && *In ? (strlen(In)+1) : 0 )
    {
		if( ArrayNum )
		{
			appMemcpy( GetData(), ANSI_TO_TCHAR(In), ArrayNum*sizeof(TCHAR) );
		}
    }
	FString( ENoInit )
	: TArray<TCHAR>( E_NoInit )
	{}
	FString& operator=( const TCHAR* Other )
	{
		if( GetTypedData() != Other )
		{
			ArrayNum = ArrayMax = *Other ? appStrlen(Other)+1 : 0;
			AllocatorInstance.ResizeAllocation(0,ArrayMax,sizeof(TCHAR));
			if( ArrayNum )
			{
				appMemcpy( GetData(), Other, ArrayNum*sizeof(TCHAR) );
			}
		}
		return *this;
	}
	FString& operator=( const FString& Other )
	{
		if( this != &Other )
		{
			ArrayNum = ArrayMax = Other.Num();
			AllocatorInstance.ResizeAllocation(0,ArrayMax,sizeof(TCHAR));
			if( ArrayNum )
			{
				appMemcpy( GetData(), *Other, ArrayNum*sizeof(TCHAR) );
			}
		}
		return *this;
	}
    TCHAR& operator[]( INT i )
	{
		checkSlow(i>=0);
		checkSlow(i<ArrayNum);
		checkSlow(ArrayMax>=ArrayNum);
		return GetTypedData()[i];
	}
	const TCHAR& operator[]( INT i ) const
	{
		checkSlow(i>=0);
		checkSlow(i<ArrayNum);
		checkSlow(ArrayMax>=ArrayNum);
		return GetTypedData()[i];
	}

	void CheckInvariants() const
	{
		checkSlow(ArrayNum>=0);
		checkSlow(!ArrayNum || !(GetTypedData()[ArrayNum-1]));
		checkSlow(ArrayMax>=ArrayNum);
	}
	void Empty( INT Slack=0 )
	{
		TArray<TCHAR>::Empty(Slack > 0 ? Slack + 1 : Slack);
	}
	void Shrink()
	{
		TArray<TCHAR>::Shrink();
	}
	const TCHAR* operator*() const
	{
		return Num() ? &(*this)(0) : TEXT("");
	}
	TArray<TCHAR>& GetCharArray()
	{
		//warning: Operations on the TArray<CHAR> can be unsafe, such as adding
		// non-terminating 0's or removing the terminating zero.
		return (TArray<TCHAR>&)*this;
	}
	FString& operator+=( const TCHAR* Str )
	{
		checkSlow(Str);
		CheckInvariants();

		if( *Str )
		{
			INT Index = ArrayNum;
			INT NumAdd = appStrlen(Str)+1;
			INT NumCopy = NumAdd;
			if (ArrayNum)
			{
				NumAdd--;
				Index--;
			}
			Add( NumAdd );
			appMemcpy( &(*this)(Index), Str, NumCopy * sizeof(TCHAR) );
		}
		return *this;
	}
	FString& operator+=(const TCHAR inChar)
	{
		checkSlow(ArrayMax>=ArrayNum);
		checkSlow(ArrayNum>=0);

		if ( inChar != '\0' )
		{
			// position to insert the character.  
			// At the end of the string if we have existing characters, otherwise at the 0 position
			INT InsertIndex = (ArrayNum > 0) ? ArrayNum-1 : 0;	

			// number of characters to add.  If we don't have any existing characters, 
			// we'll need to append the terminating zero as well.
			INT InsertCount = (ArrayNum > 0) ? 1 : 2;				

			Add(InsertCount);
			(*this)(InsertIndex) = inChar;
			(*this)(InsertIndex+1) = '\0';
		}
		return *this;
	}
	FString& AppendChar(const TCHAR inChar)
	{
		*this += inChar;
		return *this;
	}
	FString& operator+=( const FString& Str )
	{
		CheckInvariants();
		Str.CheckInvariants();
		if( Str.ArrayNum )
		{
			INT Index = ArrayNum;
			INT NumAdd = Str.ArrayNum;
			if (ArrayNum)
			{
				NumAdd--;
				Index--;
			}
			Add( NumAdd );
			appMemcpy( &(*this)(Index), Str.GetData(), Str.ArrayNum * sizeof(TCHAR) );
		}
		return *this;
	}
	FString operator+( const TCHAR inChar ) const
	{
		CheckInvariants();
		return FString(*this, 2) += inChar; // may have an extra character of "slack"
	}
	FString operator+( const TCHAR* Str ) const
	{
		checkSlow(Str);
		CheckInvariants();

		if( *Str )
		{
			INT Index = ArrayNum;
			INT NumAdd = appStrlen(Str)+1;
			INT NumCopy = NumAdd;
			if (ArrayNum)
			{
				NumAdd--;
				Index--;
			}
			FString Ret( *this, NumAdd ) ; 
			Ret.Add( NumAdd );
			appMemcpy( &(Ret)(Index), Str, NumCopy * sizeof(TCHAR) );
			return Ret;
		}
		return *this;
	}
	FString operator+( const FString& Str ) const
	{
		CheckInvariants();
		Str.CheckInvariants();
		if( Str.ArrayNum )
		{
			INT Index = ArrayNum;
			INT NumAdd = Str.ArrayNum;
			if (ArrayNum)
			{
				NumAdd--;
				Index--;
			}
			FString ret( *this, NumAdd ) ; 
			ret.Add( NumAdd );
			appMemcpy( &(ret)(Index), Str.GetData(), Str.ArrayNum * sizeof(TCHAR) );
			return ret;
		}
		return *this;
	}
	FString& operator*=( const TCHAR* Str )
	{
		if( ArrayNum>1 && (*this)(ArrayNum-2)!=PATH_SEPARATOR[0] )
			*this += PATH_SEPARATOR;
		return *this += Str;
	}
	FString& operator*=( const FString& Str )
	{
		return operator*=( *Str );
	}
	FString operator*( const TCHAR* Str ) const
	{
		return FString( *this ) *= Str;
	}
	FString operator*( const FString& Str ) const
	{
		return operator*( *Str );
	}
	UBOOL operator<=( const TCHAR* Other ) const
	{
		return !(appStricmp( **this, Other ) > 0);
	}
	UBOOL operator<( const TCHAR* Other ) const
	{
		return appStricmp( **this, Other ) < 0;
	}
	UBOOL operator<( const FString& Other ) const
	{
		return appStricmp( **this, *Other ) < 0;
	}
	UBOOL operator>=( const TCHAR* Other ) const
	{
		return !(appStricmp( **this, Other ) < 0);
	}
	UBOOL operator>( const TCHAR* Other ) const
	{
		return appStricmp( **this, Other ) > 0;
	}
	UBOOL operator>( const FString& Other ) const
	{
		return appStricmp( **this, *Other ) > 0;
	}
	UBOOL operator==( const TCHAR* Other ) const
	{
		return appStricmp( **this, Other )==0;
	}
	UBOOL operator==( const FString& Other ) const
	{
		return appStricmp( **this, *Other )==0;
	}
	UBOOL operator!=( const TCHAR* Other ) const
	{
		return appStricmp( **this, Other )!=0;
	}
	UBOOL operator!=( const FString& Other ) const
	{
		return appStricmp( **this, *Other )!=0;
	}
	UBOOL operator<=(const FString& Str) const
	{
		return !(appStricmp(**this, *Str) > 0);
	}
	UBOOL operator>=(const FString& Str) const
	{
		return !(appStricmp(**this, *Str) < 0);
	}
	INT Len() const
	{
		return Num() ? Num()-1 : 0;
	}
	FString Left( INT Count ) const
	{
		return FString( Clamp(Count,0,Len()), **this );
	}
	FString LeftChop( INT Count ) const
	{
		return FString( Clamp(Len()-Count,0,Len()), **this );
	}
	FString Right( INT Count ) const
	{
		return FString( **this + Len()-Clamp(Count,0,Len()) );
	}
	FString Mid( INT Start, INT Count=MAXINT ) const
	{
		DWORD End = Start+Count;
		Start    = Clamp( (DWORD)Start, (DWORD)0,     (DWORD)Len() );
		End      = Clamp( (DWORD)End,   (DWORD)Start, (DWORD)Len() );
		return FString( End-Start, **this + Start );
	}

	//@{
	/**
	 * Searches the string for a substring, and returns index into this string
	 * of the first found instance. Can search from beginning or end, and ignore case or not.
	 *
	 * @param SubStr The string to search for
	 * @param bSearchFromEnd If TRUE, the search will start at the end of the string and go backwards
	 * @param bIgnoreCase If TRUE, the search will be case insensitive
	 */
	INT InStr( const TCHAR* SubStr, UBOOL bSearchFromEnd=FALSE, UBOOL bIgnoreCase=FALSE, INT StartPosition=INDEX_NONE ) const
	{
		if ( SubStr == NULL )
		{
			return INDEX_NONE;
		}
		if( !bSearchFromEnd )
		{
			const TCHAR* Start = **this;
			if ( StartPosition != INDEX_NONE )
			{
				Start += Clamp(StartPosition, 0, Len() - 1);
			}
			const TCHAR* Tmp = bIgnoreCase 
				? appStristr(Start, SubStr)
				: appStrstr(Start, SubStr);

			return Tmp ? (Tmp-**this) : -1;
		}
		else
		{
			// if ignoring, do a onetime ToUpper on both strings, to avoid ToUppering multiple
			// times in the loop below
			if (bIgnoreCase)
			{
				return ToUpper().InStr(FString(SubStr).ToUpper(), TRUE, FALSE, StartPosition);
			}
			else
			{
				const INT SearchStringLength=Max(1, appStrlen(SubStr));
				if ( StartPosition == INDEX_NONE )
				{
					StartPosition = Len();
				}
				for( INT i = StartPosition - SearchStringLength; i >= 0; i-- )
				{
					INT j;
					for( j=0; SubStr[j]; j++ )
					{
						if( (*this)(i+j)!=SubStr[j] )
						{
							break;
						}
					}
					if( !SubStr[j] )
					{
						return i;
					}
				}
				return -1;
			}
		}
	}
	INT InStr( const FString& SubStr, UBOOL bSearchFromEnd=FALSE, UBOOL bIgnoreCase=FALSE, INT StartPosition=INDEX_NONE ) const
	{
		return InStr( *SubStr, bSearchFromEnd, bIgnoreCase, StartPosition );
	}	//@}

	UBOOL Split( const FString& InS, FString* LeftS, FString* RightS, UBOOL InRight=0 ) const
	{
		INT InPos = InStr(InS,InRight);
		if( InPos<0 )
			return 0;
		if( LeftS )
			*LeftS = Left(InPos);
		if( RightS )
			*RightS = Mid(InPos+InS.Len());
		return 1;
	}
	FString ToUpper() const
	{
		FString New( **this );
		for( INT i=0; i< New.ArrayNum; i++ )
			New(i) = appToUpper(New(i));
		return New;
	}
	FString ToLower() const
	{
		FString New( **this );
		for( INT i=0; i<New.ArrayNum; i++ )
			New(i) = appToLower(New(i));
		return New;
	}
	FString LeftPad( INT ChCount ) const;
	FString RightPad( INT ChCount ) const;
	
	UBOOL IsNumeric() const;
	
	VARARG_DECL( static FString, static FString, return, Printf, VARARG_NONE, const TCHAR*, VARARG_NONE, VARARG_NONE );

	static FString Chr( TCHAR Ch );
	friend FArchive& operator<<( FArchive& Ar, FString& S );
	friend struct FStringNoInit;


	/**
	 * Returns TRUE if this string begins with the specified text.
	 */
	UBOOL StartsWith(const FString& InPrefix ) const
	{
		return InPrefix.Len() > 0 && !appStrnicmp(**this, *InPrefix, InPrefix.Len());
	}

	/**
	 * Removes whitespace characters from the front of this string.
	 */
	FString Trim()
	{
		INT Pos = 0;
		while(Pos < Len())
		{
			if( appIsWhitespace( (*this)[Pos] ) )
			{
				Pos++;
			}
			else
			{
				break;
			}
		}

		*this = Right( Len()-Pos );

		return *this;
	}

	/**
	 * Removes trailing whitespace characters
	 */
	FString TrimTrailing( void )
	{
		INT Pos = Len() - 1;
		while( Pos >= 0 )
		{
			if( !appIsWhitespace( ( *this )[Pos] ) )
			{
				break;
			}

			Pos--;
		}

		*this = Left( Pos + 1 );

		return( *this );
	}

	/**
	 * Returns a copy of this string with wrapping quotation marks removed.
	 */
	FString TrimQuotes( UBOOL* bQuotesRemoved=NULL ) const
	{
		UBOOL bQuotesWereRemoved=FALSE;
		INT Start = 0, Count = Len();
		if ( Count > 0 )
		{
			if ( (*this)[0] == TCHAR('"') )
			{
				Start++;
				Count--;
				bQuotesWereRemoved=TRUE;
			}

			if ( Len() > 1 && (*this)[Len() - 1] == TCHAR('"') )
			{
				Count--;
				bQuotesWereRemoved=TRUE;
			}
		}

		if ( bQuotesRemoved != NULL )
		{
			*bQuotesRemoved = bQuotesWereRemoved;
		}
		return Mid(Start, Count);
	}

	/**
	 * Breaks up a delimited string into elements of a string array.
	 *
	 * @param	InArray		The array to fill with the string pieces
	 * @param	pchDelim	The string to delimit on
	 * @param	InCullEmpty	If 1, empty strings are not added to the array
	 *
	 * @return	The number of elements in InArray
	 */
	INT ParseIntoArray( TArray<FString>* InArray, const TCHAR* pchDelim, UBOOL InCullEmpty ) const;

	/**
	 * Breaks up a delimited string into elements of a string array, using any whitespace and an 
	 * optional extra delimter, like a ","
	 *
	 * @param	InArray			The array to fill with the string pieces
	 * @param	pchExtraDelim	The string to delimit on
	 *
	 * @return	The number of elements in InArray
	 */
	INT ParseIntoArrayWS( TArray<FString>* InArray, const TCHAR* pchExtraDelim = NULL ) const;

	/**
	 * Takes an array of strings and removes any zero length entries.
	 *
	 * @param	InArray	The array to cull
	 *
	 * @return	The number of elements left in InArray
	 */
	static INT CullArray( TArray<FString>* InArray )
	{
		check(InArray);
		for( INT x=InArray->Num()-1; x>=0; x-- )
		{
			if( (*InArray)(x).Len() == 0 )
			{
				InArray->Remove( x );
			}
		}
		return InArray->Num();
	}
	/**
	 * Returns a copy of this string, with the characters in reverse order
	 */
	FString Reverse() const
	{
		FString New;
		for( int x = Len()-1 ; x > -1 ; x-- )
			New += Mid(x,1);
		return New;
	}

	/**
	 * Reverses the order of characters in this string
	 */
	void ReverseString()
	{
		if ( Len() > 0 )
		{
			TCHAR* StartChar = &(*this)(0);
			TCHAR* EndChar = &(*this)(Len()-1);
			TCHAR TempChar;
			do 
			{
				TempChar = *StartChar;	// store the current value of StartChar
				*StartChar = *EndChar;	// change the value of StartChar to the value of EndChar
				*EndChar = TempChar;	// change the value of EndChar to the character that was previously at StartChar

				StartChar++;
				EndChar--;

			} while( StartChar < EndChar );	// repeat until we've reached the midpoint of the string
		}
	}

	// Replace all occurrences of a substring
	FString Replace(const TCHAR* From, const TCHAR* To, UBOOL bIgnoreCase=FALSE) const;

	/**
	 * Replace all occurrences of SearchText with ReplacementText in this string.
	 *
	 * @param	SearchText	the text that should be removed from this string
	 * @param	ReplacementText		the text to insert in its place
	 *
	 * @return	the number of occurrences of SearchText that were replaced.
	 */
	INT ReplaceInline( const TCHAR* SearchText, const TCHAR* ReplacementText );

	/**
	 * Returns a copy of this string with all quote marks escaped (unless the quote is already escaped)
	 */
	FString ReplaceQuotesWithEscapedQuotes() const;

	/**
	 * Replaces certain characters with the "escaped" version of that character (i.e. replaces "\n" with "\\n").
	 * The characters supported are: { \n, \r, \t, \', \", \\ }.
	 *
	 * @param	Chars	by default, replaces all supported characters; this parameter allows you to limit the replacement to a subset.
	 *
	 * @return	a string with all control characters replaced by the escaped version.
	 */
	FString ReplaceCharWithEscapedChar( const TArray<TCHAR>* Chars=NULL ) const;
	/**
	 * Removes the escape backslash for all supported characters, replacing the escape and character with the non-escaped version.  (i.e.
	 * replaces "\\n" with "\n".  Counterpart to ReplaceCharWithEscapedChar().
	 */
	FString ReplaceEscapedCharWithChar( const TArray<TCHAR>* Chars=NULL ) const;

	// Takes the number passed in and formats the string in comma format ( 12345 becomes "12,345")
	static FString FormatAsNumber( INT InNumber )
	{
		FString Number = appItoa( InNumber ), Result;

		int dec = 0;
		for( int x = Number.Len()-1 ; x > -1 ; --x )
		{
			Result += Number.Mid(x,1);

			dec++;
			if( dec == 3 && x > 0 )
			{
				Result += TEXT(",");
				dec = 0;
			}
		}

		return Result.Reverse();
	}
};
struct FStringNoInit : public FString
{
	FStringNoInit()
	: FString( E_NoInit )
	{}
	FStringNoInit& operator=( const TCHAR* Other )
	{
		if( GetTypedData() != Other )
		{
			ArrayNum = ArrayMax = *Other ? appStrlen(Other)+1 : 0;
			AllocatorInstance.ResizeAllocation(0,ArrayMax,sizeof(TCHAR));
			if( ArrayNum )
			{
				appMemcpy( GetData(), Other, ArrayNum*sizeof(TCHAR) );
			}
		}
		return *this;
	}
	FStringNoInit& operator=( const FString& Other )
	{
		if( this != &Other )
		{
			ArrayNum = ArrayMax = Other.Num();
			AllocatorInstance.ResizeAllocation(0,ArrayMax,sizeof(TCHAR));
			if( ArrayNum )
			{
				appMemcpy( GetData(), *Other, ArrayNum*sizeof(TCHAR) );
			}
		}
		return *this;
	}
};
inline DWORD GetTypeHash( const FString& S )
{
	return appStrihash(*S);
}

/*-----------------------------------------------------------------------------
	FFilename.
-----------------------------------------------------------------------------*/

/**
 * Utility class for quick inquiries against filenames.
 */
class FFilename : public FString
{
public:
	FFilename()
		: FString()
	{}
	FFilename( const FString& Other )
		: FString( Other )
	{}
	FFilename( const TCHAR* In )
		: FString( In )
	{}
	FFilename( const ANSICHAR* In )
		: FString( In )
    {}
	FFilename( ENoInit )
		: FString( E_NoInit )
	{}

	/**
	 * Gets the extension for this filename.
	 *
	 * @param	bIncludeDot		if TRUE, includes the leading dot in the result
	 *
	 * @return	the extension of this filename, or an empty string if the filename doesn't have an extension.
	 */
	FString GetExtension( UBOOL bIncludeDot=FALSE ) const;

	// Returns the filename (with extension), minus any path information.
	FString GetCleanFilename() const;

	// Returns the same thing as GetCleanFilename, but without the extension
	FString GetBaseFilename( UBOOL bRemovePath=TRUE ) const;

	// Returns the path in front of the filename
	FString GetPath() const;

	/**
	 * Returns the localized package name by appending the language suffix before the extension.
	 *
	 * @param	Language	Language to use.
	 * @return	Localized package name
	 */
	FString GetLocalizedFilename( const TCHAR* Language = NULL ) const;
};

//
// String exchanger.
//
inline void ExchangeString( FString& A, FString& B )
{
	appMemswap( &A, &B, sizeof(FString) );
}

/*----------------------------------------------------------------------------
	Special archivers.
----------------------------------------------------------------------------*/

//
// String output device.
//
class FStringOutputDevice : public FString, public FOutputDevice
{
public:
	FStringOutputDevice( const TCHAR* OutputDeviceName=TEXT("") ):
		FString( OutputDeviceName )
	{}
	void Serialize( const TCHAR* InData, EName Event )
	{
		*this += (TCHAR*)InData;
	}
};

#endif
