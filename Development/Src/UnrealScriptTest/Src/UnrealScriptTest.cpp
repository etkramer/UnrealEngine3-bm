/*=============================================================================
	ScriptTest.cpp
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

// Includes.
#include "UnrealScriptTest.h"

/*-----------------------------------------------------------------------------
	The following must be done once per package.
-----------------------------------------------------------------------------*/

#define STATIC_LINKING_MOJO 1

// Register things.
#define NAMES_ONLY
#define AUTOGENERATE_NAME(name) FName UNREALSCRIPTTEST_##name;
#define AUTOGENERATE_FUNCTION(cls,idx,name) IMPLEMENT_FUNCTION(cls,idx,name)
#include "UnrealScriptTestClasses.h"
#undef AUTOGENERATE_FUNCTION
#undef AUTOGENERATE_NAME
#undef NAMES_ONLY

// Register natives.
#define NATIVES_ONLY
#define NAMES_ONLY
#define AUTOGENERATE_NAME(name)
#define AUTOGENERATE_FUNCTION(cls,idx,name)
#include "UnrealScriptTestClasses.h"
#undef AUTOGENERATE_FUNCTION
#undef AUTOGENERATE_NAME
#undef NATIVES_ONLY
#undef NAMES_ONLY

/**
 * Initialize registrants, basically calling StaticClass() to create the class and also 
 * populating the lookup table.
 *
 * @param	Lookup	current index into lookup table
 */
void AutoInitializeRegistrantsUnrealScriptTest( INT& Lookup )
{
	AUTO_INITIALIZE_REGISTRANTS_UNREALSCRIPTTEST
}

/**
 * Auto generates names.
 */
void AutoGenerateNamesUnrealScriptTest()
{
	#define NAMES_ONLY
	#define AUTOGENERATE_FUNCTION(cls,idx,name)
	#define AUTOGENERATE_NAME(name) UNREALSCRIPTTEST_##name = FName(TEXT(#name));
	#include "UnrealScriptTestClasses.h"
	#undef AUTOGENERATE_FUNCTION
	#undef AUTOGENERATE_NAME
	#undef NAMES_ONLY
}



IMPLEMENT_CLASS(UTest0010_NativeObject);
IMPLEMENT_CLASS(UTest0011_NativeObjectBoolOrder);

IMPLEMENT_CLASS(UTest0002_InterfaceNative);

void UTest0010_NativeObject::TestNativeFunction(UBOOL bParam)
{
	InterfaceMember = this;
	eventTestInterfaceEvent(this);

	eventTest03_CallEventWithNativeInterface(this);
}
void UTest0010_NativeObject::Test02_PassNativeInterfaceToNativeFunction( const TScriptInterface<ITest0002_InterfaceNative>& InterfaceParm )
{
	debugf(TEXT("UTest0010_NativeObject::Test02_PassNativeInterfaceToNativeFunction - InterfaceParmInterface '%s'"), InterfaceParm ? *InterfaceParm.GetObject()->GetFullName() : TEXT("NULL"));
}

void UTest0010_NativeObject::VerifyConversionFromInterfaceToObjectAsNativeParm( UObject* InObject, INT DummyInt )
{
	debugf(TEXT("Results for passing a native interface variable as the value for an object parameter in a native function:"));
	debugf(TEXT("\tObj: %s"), *InObject->GetFullName());
	debugf(TEXT("\tDummyInt: %i"), DummyInt);
}

void UTest0010_NativeObject::PerformNativeTest( INT TestNumber )
{
	switch( TestNumber )
	{
	case 1:
		{
			FConstructorStructString StringFoo;
			FConstructorStructArray ArrayFoo;
			FConstructorStructCombo ComboFoo;
			FNoCtorProps NoCtor;
			appMemzero(&NoCtor, sizeof(NoCtor));
			debugf(TEXT("\r\nCalling event with stack params"));
			eventTest01_CallEventWithStruct(NoCtor,StringFoo,ArrayFoo,ComboFoo,TRUE);

			debugf(TEXT("\r\n*****\r\nCalling event with default params"));
			eventTest01_CallEventWithStruct(NoCtor,DefaultStringStruct,DefaultArrayStruct,DefaultComboStruct,TRUE);
		}
		break;

	case 2:
		TestDoubleLinkedList();
		break;

	case 3:
		{
			UClass* InterfaceClass = ITest0002_InterfaceNative::UClassType::StaticClass();
			ITest0002_InterfaceNative* InterfaceThis = static_cast<ITest0002_InterfaceNative*>(GetInterfaceAddress(InterfaceClass));
			if ( InterfaceThis != NULL )
			{
				InterfaceThis->TestNativeFunction(TRUE);
			}
			break;
		}
	case 4:
		{
			TestNativeFunction(TRUE);

			TScriptInterface<ITest0002_InterfaceNative> InterfaceVar, InterfaceVar2, InterfaceVar3(this);
			InterfaceVar2 = this;
			InterfaceVar = this;

			InterfaceVar->TestNativeFunction(TRUE);


			InterfaceVar = InterfaceVar2;
			if ( InterfaceVar == this )
			{
				debugf(TEXT("hello"));
			}
		}
		break;

	case 5:
		{
			FMyStruct* TempStruct = new(MyArray) FMyStruct(EC_EventParm);
			TempStruct->MyFirstFloat = 1.f;
			TempStruct->MyFirstInt = 3;
			TempStruct->MyFirstString = TEXT("wooyay");
			TempStruct->MyInt = 4;
			TempStruct->MyFloat = 0.2f;
			TempStruct->MyStrings[0] = TEXT("test0");
			TempStruct->MyStrings[1] = TEXT("test1");
			TempStruct->MyStrings[2] = TEXT("test2");

			eventTest05_StructInheritance();
			break;
		}

	case 6:
		{
			debugf(TEXT("****  TESTING ASSIGNMENT FROM INTERFACE TO OBJECT VAR  ****"));
			eventTest06_InterfaceToObjectConversions();
		}
		break;

	case 7:
		{
			FUniqueNetId Source_Qword_Id;
			Source_Qword_Id.Uid = DECLARE_UINT64(0xFEFEFEFE4F4F4F4F);

			debugf(TEXT("Source_Qword_Id: %016I64X"), (QWORD&)Source_Qword_Id.Uid);

			FString Dest_Qword_Id = UOnlineSubsystem::UniqueNetIdToString(Source_Qword_Id);
			debugf(TEXT("Source_Qword_Id => string: %s"), *Dest_Qword_Id);

			FUniqueNetId Final_Qword_Id;
			UOnlineSubsystem::StringToUniqueNetId(Dest_Qword_Id, Final_Qword_Id);
			debugf(TEXT("RoundTrip for QWORD source: %016I64X"), (QWORD&)Final_Qword_Id.Uid);

			debugf(TEXT(""));

			FString Source_String_Id = TEXT("FEFEFEFE4F4F4F4F");
			FUniqueNetId Dest_String_Id;

			debugf(TEXT("Source_String_Id: %s"), *Source_String_Id);
			UOnlineSubsystem::StringToUniqueNetId(Source_String_Id, Dest_String_Id);
			debugf(TEXT("Dest_String_Id => %016I64X"), (QWORD&)Dest_String_Id.Uid);
			debugf(TEXT("RoundTrip for STRING source: %s"), *UOnlineSubsystem::UniqueNetIdToString(Dest_String_Id));
			break;
		}

	case 8:
		{
			debugf(TEXT("Calling event with value for first optional parm"));
			eventTestOptionalEventStringParm(TEXT("This is the first value"),TEXT("This is the second value"));
			debugf(TEXT("Calling event without value for first optional parm"));
			eventTestOptionalEventStringParm(TEXT(""),TEXT("This is the second value"));

			debugf(TEXT("Calling event with value for second optional parm"));
			eventTestOptionalEventStringParm(TEXT("This is the first value"), TEXT("This is the second value"));
			debugf(TEXT("Calling event without value for second optional parm"));
			eventTestOptionalEventStringParm(TEXT("This is the first value"));
			break;
		}

	case 9:
		{
			eventTestMultipleOptionalParms();
			break;
		}

	case 10:
		{
			FLOAT LerpedValue = 0.f;
			FLOAT Value = 0.f;
			const FLOAT Destination = 1.f;
			FLOAT RemainingTime = 1.f;
			const FLOAT InterpAlpha = 0.2;

			FLOAT TimeElapsed = 0.f;
			debugf(TEXT("Start + (Perc * (End - Start))"));
			INT Ticks = 0;
			do 
			{
				const FLOAT DeltaTime = Max(0.3f * appFrand(), 0.05f);
				const FLOAT Start = Value;
				const FLOAT End = Destination;
				const FLOAT Percentage = DeltaTime / Max(0.00001f, RemainingTime);
				const FLOAT Difference = End - Start;

				LerpedValue = Lerp(LerpedValue, End, Percentage);
				TimeElapsed += DeltaTime;
				debugf(TEXT("Start:%f   End:%f   Percentage:%f   Difference:%f    Duration:%f   DeltaTime:%f"), Start, End, Percentage, Difference, RemainingTime, DeltaTime);
				debugf(TEXT("%i:  %f + (%f * (%f - %f)) = %f     SHOULD BE: %f     (LerpedValue:%f)"),
					Ticks, Start, Percentage, End, Start, (Start + (Percentage * (End - Start))), TimeElapsed * Destination, LerpedValue);

				Value = (Start + (Percentage * (End - Start)));

				RemainingTime -= DeltaTime;
				Ticks++;

				debugf(TEXT(""));
			} while( Destination - Value > DELTA && RemainingTime > DELTA );
			break;
		}
	}
}

void UTest0010_NativeObject::TestDoubleLinkedList()
{
	TDoubleLinkedList<FString> StringList;

	StringList.AddHead(TEXT("Base"));

	INT i = 0;
	for ( i = 0; i < 10; i++ )
	{
		StringList.AddTail(appItoa(i));
	}

	for ( i = -1; i > -10; i-- )
	{
		StringList.AddHead(appItoa(i));
	}

	TDoubleLinkedList<FString>::TDoubleLinkedListNode* BaseNode = StringList.FindNode(TEXT("Base"));
	StringList.InsertNode(TEXT("InsertedNode"), BaseNode);

	StringList.RemoveNode(TEXT("5"));

	debugf(TEXT("List has %i elements"), StringList.Num());


	debugf(TEXT("From head to tail:"));
	i = 0;
	for ( TDoubleLinkedList<FString>::TIterator It(StringList.GetHead()); It; ++It )
	{
		FString NodeValue = *It;
		debugf( TEXT("%i) %s"), i++, *NodeValue );
	}

	debugf(TEXT("\r\nFrom tail to head:"));
	i = StringList.Num() - 1;
	for ( TDoubleLinkedList<FString>::TIterator It(StringList.GetTail()); It; --It )
	{
		FString NodeValue = *It;
		debugf( TEXT("%i) %s"), i--, *NodeValue );
	}

}


void UTest0011_NativeObjectBoolOrder::PerformBoolOrderTest()
{
	eventNativeTestBoolOrderEvent(TRUE);
}


UBOOL UTest0011_NativeObjectBoolOrder::NativeTestBoolOrderFunction(UBOOL BoolParm)
{
	debugf(TEXT("Input value for native function: %i"), BoolParm);

	return BoolParm;
}




