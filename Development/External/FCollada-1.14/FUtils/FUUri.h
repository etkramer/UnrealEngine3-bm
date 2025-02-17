/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/
/*
	Based on the FS Import classes:
	Copyright (C) 2005-2006 Feeling Software Inc
	Copyright (C) 2005-2006 Autodesk Media Entertainment
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#ifndef _FU_URI_H_
#define _FU_URI_H_

class FUUri
{
public:
	fstring prefix;
	string suffix;

	FUUri();
	FUUri(const string& id);
};

typedef vector<FUUri> FUUriList;

#endif // _FU_URI_H_

