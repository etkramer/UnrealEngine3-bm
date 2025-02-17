/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#include "StdAfx.h"
#include "FUtils/FUStringConversion.h"
#include "FUtils/FUUri.h"

FUUri::FUUri()
{
}

FUUri::FUUri(const string& id)
{
	size_t index = id.find('#');
	if (index == string::npos) suffix = id;
	else
	{
		prefix = FUStringConversion::ToFString(id.substr(0, index));
		suffix = id.substr(index + 1);
	}
}
