#ifndef TestFxString_H__
#define TestFxString_H__

#include "FxString.h"
#include "FxArchive.h"
#include "FxArchiveStoreNull.h"
#include "FxArchiveStoreMemory.h"

using OC3Ent::Face::FxString;

UNITTEST( FxString, DefaultConstructor )
{
	FxString test;
	CHECK( test.Length() == 0 );
	CHECK( test.Allocated() == 0 );
}

UNITTEST( FxString, ConstructFromNull )
{
	FxChar* nullString = NULL;
	FxString test( nullString );
	CHECK( test.Length() == 0 );
	CHECK( test.Allocated() == 0 );
}

UNITTEST( FxString, ConstructFromPointer )
{
	FxString test( "testing one two three" );
	CHECK( test.Length() == 21 );
}

UNITTEST( FxString, EqualityOperator )
{
	FxString test1( "hello world" );
	FxString test2( "hello world" );
	FxString test3( "break me!" );

	CHECK( test1 == test2 );
	CHECK( !(test1 == test3) );
}

UNITTEST( FxString, InequalityOperator )
{
	FxString test1( "hello world" );
	FxString test2( "not hello world" );
	FxString test3( "not hello world" );

	CHECK( test1 != test2 );
	CHECK( !(test2 != test3) );
}

UNITTEST( FxString, LessThanOperator )
{
	FxString test1( "hello world" );
	FxString test2( "allo world" );
	FxString test3( "zallo world" );

	CHECK( test2 < test1 );
	CHECK( !(test3 < test1) );
}

UNITTEST( FxString, AssignmentOperator )
{
	FxString original( "hello world" );
	FxString other;
	other = original;

	CHECK( other.Length() == 11 );
	CHECK( other == "hello world" );
}

UNITTEST( FxString, Swap )
{
	FxString hello( "hello" );
	FxString test( "test" );
	hello.Swap(test);

	CHECK( hello == "test" && test == "hello" );
	CHECK( hello.Allocated() == 5 && test.Allocated() == 6 );
}

UNITTEST( FxString, CharacterAccess )
{
	FxString hello( "hello" );
	CHECK( hello[0] == 'h' && hello[1] == 'e' && hello[2] == 'l' && hello[3] == 'l' && hello[4] == 'o' );
	hello[3] = 'b';
	CHECK( hello == "helbo" );
}

UNITTEST( FxString, ConstCharacterAccess )
{
	const FxString hello( "hello" );
	CHECK( hello[0] == 'h' && hello[1] == 'e' && hello[2] == 'l' && hello[3] == 'l' && hello[4] == 'o' );
}

UNITTEST( FxString, GetCstr )
{
	FxString hello( "hello" );
	CHECK( strcmp( hello.GetCstr(), "hello" ) == 0 );
}

UNITTEST( FxString, Length )
{
	FxChar* nullString = NULL;
	FxString test( nullString );
	CHECK( test.Length() == 0 );

	test = "hello";
	CHECK( test.Length() == 5 );

	test.Clear();
	CHECK( test.Length() == 0 );
}

UNITTEST( FxString, Conversions )
{
	FxString test("this is a test");
	FxWString wtest("this is a test");
	CHECK(strcmp(test.GetCstr(), "this is a test") == 0);
	CHECK(strcmp(test.GetCstr(), "this is a test") == 0);
	CHECK(wcscmp(test.GetWCstr(), L"this is a test") == 0);
	CHECK(wcscmp(test.GetWCstr(), L"this is a test") == 0);

	FxString testc(L"conv from unicode");
	FxWString wtestc("conv from ansi");
	CHECK(strcmp(testc.GetData(), "conv from unicode") == 0);
	CHECK(wcscmp(wtestc.GetData(), L"conv from ansi") == 0);

	FxString constrFromWide(wtestc.GetData());
	FxString constrFromConvertedAnsi(wtestc.GetCstr());
	FxString constrFromConvertedWide(wtestc.GetWCstr());
	CHECK(strcmp(constrFromWide.GetData(), "conv from ansi")==0);
	CHECK(strcmp(constrFromConvertedWide.GetData(), "conv from ansi")==0);
	CHECK(strcmp(constrFromConvertedWide.GetData(), "conv from ansi")==0);

	FxWString wconstrFromAnsi(testc.GetData());
	FxWString wconstrFromConvertedAnsi(testc.GetCstr());
	FxWString wconstrFromConvertedWide(testc.GetWCstr());
	CHECK(wcscmp(wconstrFromAnsi.GetData(), L"conv from unicode")==0);
	CHECK(wcscmp(wconstrFromConvertedAnsi.GetData(), L"conv from unicode")==0);
	CHECK(wcscmp(wconstrFromConvertedWide.GetData(), L"conv from unicode")==0);
}

UNITTEST( FxString, NewFunctions )
{
	FxString dummy;
	CHECK(dummy.IsEmpty());
	dummy += "hello";
	CHECK(!dummy.IsEmpty());
	CHECK(dummy == "hello");
	//       0123456789012345678901234567890123456789012
	dummy = "the quick brown fox jumps over the lazy dog";
	CHECK(dummy.FindFirst('q') == 4);
	CHECK(dummy.BeforeFirst('q') == "the ");
	CHECK(dummy.AfterFirst('q') == "uick brown fox jumps over the lazy dog");
	CHECK(dummy.FindLast('z') == 37);
	CHECK(dummy.BeforeLast('z') == "the quick brown fox jumps over the la");
	CHECK(dummy.AfterLast('z') == "y dog");
}

UNITTEST( FxString, Serialization )
{
	FxString toSave("this is a big test string.");
	OC3Ent::Face::FxArchive directoryCreater(OC3Ent::Face::FxArchiveStoreNull::Create(), OC3Ent::Face::FxArchive::AM_CreateDirectory);
	directoryCreater.Open();
	directoryCreater << toSave;
	OC3Ent::Face::FxArchiveStoreMemory* savingMemoryStore = 
		OC3Ent::Face::FxArchiveStoreMemory::Create( NULL, 0 );
	OC3Ent::Face::FxArchive savingArchive( savingMemoryStore, OC3Ent::Face::FxArchive::AM_Save );
	savingArchive.Open();
	savingArchive.SetInternalDataState(directoryCreater.GetInternalData());
	savingArchive << toSave;
	
	OC3Ent::Face::FxArchiveStoreMemory* loadingMemoryStore = 
		OC3Ent::Face::FxArchiveStoreMemory::Create( savingMemoryStore->GetMemory(), savingMemoryStore->GetSize() );
	OC3Ent::Face::FxArchive loadingArchive( loadingMemoryStore, OC3Ent::Face::FxArchive::AM_Load );
	loadingArchive.Open();

#ifdef NO_SAVE_VERSION
	CHECK(loadingArchive.IsValid() == FxFalse)
#else
	FxString loaded;
	loadingArchive << loaded;

	CHECK( toSave == loaded );
#endif
}

UNITTEST( FxString, Streaming )
{
	FxString test("hello");
	test << " world";
	test << ": " << 5 << 2.0f << "\n";
	CHECK(test == "hello world: 52.000000\n");
}

UNITTEST( FxString, Substring )
{
	FxString test("hello world");
	FxString sub = test.Substring(6);
	CHECK(sub == "world");
	sub = test.Substring(0, 5);
	CHECK(sub == "hello");
}

UNITTEST( FxString, Find )
{	
	FxString test("hello world");
	CHECK(test.Find("hello") == 0);
	CHECK(test.Find("world") == 6);
	CHECK(test.Find("hello world") == 0);

	FxString blank;
	CHECK(blank.Find("Not here") == FxInvalidIndex);
}

UNITTEST( FxString, Replace )
{
	FxString test("hello world");
	test.Replace("lo w", "big tom boonen");
	CHECK(test == "helbig tom boonenorld");

	test = "hello world";
	test.Replace("world", "tom boonen");
	CHECK(test == "hello tom boonen");

	test.Replace("hello", "floyd landis");
	CHECK(test == "floyd landis tom boonen");
	test.Replace("tom ", "");
	CHECK(test == "floyd landis boonen");
}

UNITTEST( FxString, FindNotOf )
{
	FxString test("  hello world ");
	CHECK(test.FindFirstNotOf(' ') == 2);
	CHECK(test.FindLastNotOf(' ') == 12);
}

UNITTEST( FxString, RemoveWhitespace )
{
	FxString test("  hello world ");
	FxString res = test.StripWhitespace();
	CHECK(res == "hello world");

	FxWString wtest("\t hello world \r\n \t");
	FxWString wres = wtest.StripWhitespace();
	CHECK(wres == L"hello world");
}

UNITTEST( FxString, ToUpper )
{
	FxString test("Hello World");
	test = test.ToUpper();
	CHECK(test == "HELLO WORLD");

	FxWString wtest(L"Hello World");
	wtest = wtest.ToUpper();
	CHECK(wtest == L"HELLO WORLD");
}

UNITTEST( FxString, ToLower )
{
	FxString test("Hello World");
	test = test.ToLower();
	CHECK(test == "hello world");

	FxWString wtest(L"Hello World");
	wtest = wtest.ToLower();
	CHECK(wtest == L"hello world");
}

#endif
