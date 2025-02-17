/**
 * This class is used for testing and regressing changes to Unrealscript MetaData parsing.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class Test0023_MetaData extends Actor
	placeable;

const ExampleConst = 1.f;

enum EMetadataTestEnum <displayname=Test Enum>
{
	METADATA_FirstValue<displayname=First Value>,
	METADATA_SecondValue<displayname=Second Value>,
	METADATA_ThirdValue<displayname=Third Value>,
};

struct ExampleStruct
{
	var()		string		ExampleStructString<tooltip=The example string from inside the struct|displayname=Struct String>;
};


var()		string				MemberString<tooltip=This is the member string variable|displayname=Display String>;
var()		ExampleStruct		MemberStruct<tooltip=Aggragates multiple properties for testing meta data|displayname=Test Struct>;
var()		EMetadataTestEnum	MemberEnum<tooltip=Another property for testing enum values|displayname=Enum Testing>;

DefaultProperties
{

}
