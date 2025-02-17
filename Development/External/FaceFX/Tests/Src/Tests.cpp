// Tests.cpp : Defines the entry point for the test suite.
//

#include "stdafx.h"

#include <iostream>

#include <cmath>

#ifdef _MSC_VER
	#if _MSC_VER > 1200
		using namespace std;
	#endif
#else
	using namespace std;
#endif

#define SHOW_CLASS_INFO

#ifdef SHOW_CLASS_INFO
	#include <iomanip>
#endif

#include "FxSDK.h"
#include "FxVersionInfo.h"

#include "TestFramework.h"

#define FLOAT_EQUALITY( x, y ) fabs( (x) - (y) ) < FLT_EPSILON

#include "TestFxPlatform.h"
#include "TestFxString.h"
#include "TestFxArray.h"
#include "TestFxList.h"
#include "TestFxClass.h"
#include "TestFxObject.h"
#include "TestFxNamedObject.h"
#include "TestFxArchive.h"
#include "TestFxRefObject.h"
#include "TestFxRefString.h"
#include "TestFxName.h"
#include "TestFxKey.h"
#include "TestFxCurve.h"
#include "TestFxQuat.h"
#include "TestFxVec3.h"
#include "TestFxVec2.h"
#include "TestFxAnim.h"
#include "TestFxBone.h"
#include "TestFxMasterBoneList.h"
#include "TestFxActor.h"
#include "TestFxActorInstance.h"
#include "TestFxFaceGraph.h"
#include "TestFxFaceGraphNode.h"
#include "TestFxBonePoseNode.h"
#include "TestFxLinkFn.h"
#include "TestFxFaceGraphNodeLink.h"
#include "TestFxAnimPlayer.h"

#ifndef __UNREAL__
	#pragma message("Building with sample code.")
	#include "SampleCode.h"
#endif // __UNREAL__

static FxInt32 numBytesAllocated = 0;

void* FX_CALL TestAllocate(FxSize numBytes)
{
	numBytesAllocated += numBytes;
	return ::operator new(numBytes);
}

void* FX_CALL TestAllocateDebug(FxSize numBytes, const FxChar* system)
{
	numBytesAllocated += numBytes;
	return ::operator new(numBytes);
}

void FX_CALL TestFree(void* ptr, FxSize numBytes)
{
	numBytesAllocated -= numBytes;
	::operator delete(ptr);
}

int main( int argc, char* argv[] )
{
	FxBool testFailed = FxFalse;
	{
	// Start up the FaceFX SDK with its default allocation policy.
	FxMemoryAllocationPolicy allocationPolicy;
	allocationPolicy.allocationType = MAT_Custom;
	allocationPolicy.bUseMemoryPools = FxFalse;
	allocationPolicy.userAllocate = TestAllocate;
	allocationPolicy.userAllocateDebug = TestAllocateDebug;
	allocationPolicy.userFree = TestFree;
	FxSDKStartup(allocationPolicy);

#ifdef SHOW_CLASS_INFO
	FxSize numClasses = FxClass::GetNumClasses();
	std::cout << "FaceFx Class System [" << numClasses 
		      << " classes]:" << std::endl << std::endl;
	std::cout << "Class\t\tVersion\t\tSize\t\tNum. Children\t\tParent" << 
		std::endl;
	std::cout << "-------------------------------------------" << 
		std::endl;
	FxSize largestClassName = 0;
	for( FxSize i = 0; i < numClasses; ++i )
	{
		const FxClass* pClass = FxClass::GetClass(i);
		if( pClass )
		{
			if( pClass->GetName().GetAsString().Length() > largestClassName )
			{
				largestClassName = pClass->GetName().GetAsString().Length();
			}
		}
	}
	for( FxSize i = 0; i < numClasses; ++i )
	{
		const FxClass* pClass = FxClass::GetClass(i);
		if( pClass )
		{
			std::cout << std::setw(2) << pClass->GetName().GetAsString().GetData() << 
				std::setw(largestClassName - pClass->GetName().GetAsString().Length() + 12 )
					  << pClass->GetCurrentVersion() << std::setw(4)
					  << pClass->GetSize() << std::setw(4)
					  << pClass->GetNumChildren() << std::setw(largestClassName)
					  << (pClass->GetBaseClassDesc() ? 
					  	 pClass->GetBaseClassDesc()->GetName().GetAsString().GetData() :
					     "None")
					  << std::endl;
		}
	}
	std::cout << std::resetiosflags(std::ios_base::adjustfield);
	std::cout << std::endl;
	
	std::cout << "Name Table [" << FxName::GetNameTableSize() << " bytes]:" << std::endl;
	for( FxSize i = 0; i < FxName::GetNumNames(); ++i )
	{
		if( FxName::GetNameAsString(i) )
		{
			std::cout << FxName::GetNameAsString(i)->GetCstr() << std::endl;
		}
	}

	std::cout << "Link Function Table [" << FxLinkFn::GetNumLinkFunctions() 
		      << " link functions]:" << std::endl;
	for( FxSize i = 0; i < FxLinkFn::GetNumLinkFunctions(); ++i )
	{
		const FxLinkFn* linkFn = FxLinkFn::GetLinkFunction(i);
		if( linkFn )
		{
			std::cout << linkFn->GetName().GetAsString().GetCstr() << ":" << std::endl << std::endl;
			std::cout << "\tDesc: " << linkFn->GetFnDesc().GetCstr() << std::endl << std::endl;
			std::cout << "\tParams [" << linkFn->GetNumParams() << " params]: ";
			for( FxSize j = 0; j < linkFn->GetNumParams(); ++j )
			{
				if( j > 0 )
				{
					std::cout << ", ";
				}
				std::cout << linkFn->GetParamName(j).GetCstr();
			}
			std::cout << std::endl << std::endl;
		}
	}
	std::cout << std::endl << std::endl;
#endif

	std::cout << "----[Begin Tests]----" << std::endl;

	TestResult testResults;
	TestFramework::RunAllTests( testResults );

	std::cout << "----[End Tests]----" << std::endl;
	testFailed = testResults.AnyTestFailed();

	// Shut down the FaceFX SDK.
	FxSDKShutdown();
	}
	std::cout << "Number of bytes allocated: " << numBytesAllocated;

	return testFailed; 
}
