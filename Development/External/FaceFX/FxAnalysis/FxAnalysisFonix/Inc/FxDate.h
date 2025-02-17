//------------------------------------------------------------------------------
// A date class. This is currently used for determining expiration dates in
// FxAnalysisFonix. This code is based heavily on Branislav L. Slantchev's
// zDate code that was donated to the public domain. In keeping with the 
// "license" of zDate, Branislav's original file header and copyright 
// information is pasted intact below this file header.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

/*
* This file is part of PB-Lib C/C++ Library
*
* Copyright (c) 1995, 1996 Branislav L. Slantchev
* A Product of Silicon Creations, Inc.
*
* This class is hereby donated to the SNIPPETS collection (maintained
* by Bob Stout). You are granted the right to use the code contained
* herein free of charge as long as you keep this copyright notice intact.
*
* Contact: 73023.262@compuserve.com
*/

#ifndef FxDate_H__
#define FxDate_H__

#include <time.h>

class FxDate
{
public:

	// The months of the year.
	enum FxMonth 
	{
		FxMonth_January = 1, 
		FxMonth_February, 
		FxMonth_March, 
		FxMonth_April, 
		FxMonth_May, 
		FxMonth_June, 
		FxMonth_July, 
		FxMonth_August, 
		FxMonth_September, 
		FxMonth_October, 
		FxMonth_November, 
		FxMonth_December
	};

	// The days of the week.
	enum FxDay 
	{
		FxDay_Monday = 1, 
		FxDay_Tuesday, 
		FxDay_Wednesday, 
		FxDay_Thursday, 
		FxDay_Friday, 
		FxDay_Saturday, 
		FxDay_Sunday
	};

	FxDate();
	FxDate( FxMonth FxMonth, int day, int year );
	FxDate( int dayOfYear, int year );
	FxDate( unsigned long nDayNumber );
	FxDate( const struct tm* date );
	FxDate( const FxDate& other );
	FxDate& operator=( const FxDate& other );

	static FxDate Today( void );

	FxDate AddMonths( int nMonths ) const;
	FxDate AddWeeks( int nWeeks ) const;
	FxDate AddYears( int nYears ) const;

	inline FxMonth       GetMonth( void ) const { return _month; }
	inline int           GetDay( void ) const { return _day; }
	inline unsigned long GetDayNumber( void ) const { return _dayNumber; }
	inline int           GetYear( void ) const { return _year; }

	FxDay      GetDayOfWeek( void ) const;
	int        GetDayOfYear( void ) const;
	int        GetDaysInMonth( void ) const;
	static int GetDaysInMonth( FxMonth aMonth, int aYear );
	int        GetDaysInYear( void ) const;
	static int GetDaysInYear( int year );
	
	bool        IsLeapYear( void ) const;
	static bool IsLeapYear( int year );
	bool        IsValid( void ) const;
	static bool IsValid( FxMonth aMonth, int aDay, int aYear );
	
	bool    operator!=( const FxDate& aDate ) const;
	bool    operator==( const FxDate& aDate ) const;
	bool    operator<( const FxDate& aDate ) const;
	bool    operator<=( const FxDate& aDate ) const;
	bool    operator>( const FxDate& aDate ) const;
	bool    operator>=( const FxDate& aDate ) const;
	FxDate  operator+( int nDays ) const;
	FxDate  operator+( long nDays ) const;
	long    operator+( const FxDate& aDate ) const;
	FxDate  operator-( int nDays ) const;
	FxDate  operator-( long nDays ) const;
	long    operator-( const FxDate& aDate ) const;
	FxDate& operator+=( int nDays );
	FxDate& operator+=( long nDays );
	FxDate& operator-=( int nDays );
	FxDate& operator-=( long nDays );
	FxDate  operator++( void );
	FxDate  operator++( int nDays );
	FxDate  operator--( void );
	FxDate  operator--( int nDays );
	
	int        GetWeekOfMonth( void ) const;
	int        GetWeekOfYear( void ) const;
	int        GetWeeksInYear( void ) const;
	static int GetWeeksInYear( int year );
	
	// Pope Gregor XIII's reform canceled 10 days: the day after Oct 4 1582 was 
	// Oct 15 1582.
	static const int           ReformYear;
	static const FxMonth       ReformMonth;
	static const unsigned long ReformDayNumber;

protected:
	FxDate        _set( FxMonth aMonth, int aDay, int aYear );
	unsigned long _makeDayNumber( void ) const;
	void          _fromDayNumber( unsigned long nDayNumber );

private:
	FxMonth       _month;
	int           _day;
	int           _year;
	unsigned long _dayNumber;
};

#endif
