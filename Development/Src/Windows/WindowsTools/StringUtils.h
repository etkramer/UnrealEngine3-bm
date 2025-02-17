/*=============================================================================
	WindowsSupport/StringUtils.h: String utitilities used by Windows Support.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/ 

#pragma once

/// Converts wide-char string to ansi string
inline string ToString(const wstring& Src)
{
	string Dest;
	Dest.assign(Src.begin(), Src.end());
	return Dest;
}

/// Converts ansi string to wide-char string
inline wstring ToWString(const string& Src)
{
	wstring Dest;
	Dest.assign(Src.begin(), Src.end());
	return Dest;
}
