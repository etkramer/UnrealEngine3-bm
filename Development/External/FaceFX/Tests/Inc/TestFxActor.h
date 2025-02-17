#ifndef TestFxActor_H__
#define TestFxActor_H__

#include "FxActor.h"
#include "FxActorInstance.h"
using namespace OC3Ent::Face;

UNITTEST( FxActor, AnimGroups )
{
	FxActor testActor;
	CHECK( testActor.AddAnimGroup("TestGroup") == FxTrue );
	CHECK( testActor.GetNumAnimGroups() == 2 );

	FxAnim testAnim;
	testAnim.SetName("My First Anim");
	CHECK( testActor.AddAnim("TestGroup", testAnim) == FxTrue );
	testAnim.SetName("My Second Anim");
	CHECK( testActor.AddAnim("TestGroup", testAnim) == FxTrue );

	CHECK( testActor.FindAnimGroup("TestGroup") != FxInvalidIndex );
	CHECK( testActor.RemoveAnimGroup(FxAnimGroup::Default) == FxFalse );
	CHECK( testActor.GetAnimPtr("TestGroup", "My First Anim") != NULL );
	CHECK( testActor.GetAnimPtr("TestGroup", "My Second Anim") != NULL );
	CHECK( testActor.GetAnimPtr("BadGroup", "My First Anim") == NULL );
	CHECK( testActor.GetAnimPtr("TestGroup", "BadAnim") == NULL );
}

UNITTEST( FxActor, AnimSetMounting )
{
	FxAnimGroup testGroup;
	testGroup.SetName("TestGroup");
	FxAnim testAnim;
	testAnim.SetName("My First Anim");
	CHECK( testGroup.AddAnim(testAnim) == FxTrue );
	testAnim.SetName("My Second Anim");
	CHECK( testGroup.AddAnim(testAnim) == FxTrue );
	testAnim.SetName("My Third Anim");
	CHECK( testGroup.AddAnim(testAnim) == FxTrue );
	FxAnimSet testSet;
	testSet.SetAnimGroup(testGroup);
	FxActor testActor;
	CHECK( testActor.MountAnimSet(testSet) == FxTrue );
	CHECK( testActor.MountAnimSet(testSet) == FxFalse );
	CHECK( testActor.GetNumAnimGroups() == 2 );
	CHECK( testActor.FindAnimGroup("TestGroup") != FxInvalidIndex );
	CHECK( testActor.GetAnimGroup(1).GetNumAnims() == 3 );
	testGroup.SetName(FxAnimGroup::Default);
	testSet.SetAnimGroup(testGroup);
	CHECK( testActor.MountAnimSet(testSet) == FxFalse );
	CHECK( testActor.GetNumAnimGroups() == 2 );
}

UNITTEST( FxActor, AnimSetUnmounting )
{
	FxAnimGroup testGroup;
	testGroup.SetName("TestGroup");
	FxAnim testAnim;
	testAnim.SetName("My First Anim");
	CHECK( testGroup.AddAnim(testAnim) == FxTrue );
	testAnim.SetName("My Second Anim");
	CHECK( testGroup.AddAnim(testAnim) == FxTrue );
	testAnim.SetName("My Third Anim");
	CHECK( testGroup.AddAnim(testAnim) == FxTrue );
	FxAnimSet testSet;
	testSet.SetAnimGroup(testGroup);
	FxActor testActor;
	CHECK( testActor.MountAnimSet(testSet) == FxTrue );
	CHECK( testActor.MountAnimSet(testSet) == FxFalse );
	CHECK( testActor.GetNumAnimGroups() == 2 );
	CHECK( testActor.FindAnimGroup("TestGroup") != FxInvalidIndex );
	CHECK( testActor.GetAnimGroup(1).GetNumAnims() == 3 );
	CHECK( testActor.UnmountAnimSet("TestGroup") == FxTrue );
	CHECK( testActor.UnmountAnimSet("TestGroup") == FxFalse );
	CHECK( testActor.GetNumAnimGroups() == 1 );
	CHECK( testActor.FindAnimGroup("TestGroup") == FxInvalidIndex );
	CHECK( testActor.UnmountAnimSet(FxAnimGroup::Default) == FxFalse );
	CHECK( testActor.GetNumAnimGroups() == 1 );
}

UNITTEST( FxActor, AnimSetImporting )
{
	FxAnimGroup testGroup;
	testGroup.SetName("TestGroup");
	FxAnim testAnim;
	testAnim.SetName("My First Anim");
	CHECK( testGroup.AddAnim(testAnim) == FxTrue );
	testAnim.SetName("My Second Anim");
	CHECK( testGroup.AddAnim(testAnim) == FxTrue );
	testAnim.SetName("My Third Anim");
	CHECK( testGroup.AddAnim(testAnim) == FxTrue );
	FxAnimSet testSet;
	testSet.SetAnimGroup(testGroup);
	FxActor testActor;
	CHECK( testActor.ImportAnimSet(testSet) == FxTrue );
	CHECK( testActor.ImportAnimSet(testSet) == FxFalse );
	CHECK( testActor.GetNumAnimGroups() == 2 );
	CHECK( testActor.FindAnimGroup("TestGroup") != FxInvalidIndex );
	CHECK( testActor.GetAnimGroup(1).GetNumAnims() == 3 );
	testGroup.SetName(FxAnimGroup::Default);
	testSet.SetAnimGroup(testGroup);
	CHECK( testActor.ImportAnimSet(testSet) == FxFalse );
	CHECK( testActor.GetNumAnimGroups() == 2 );
}

UNITTEST( FxActor, AnimSetExporting )
{
	FxAnimGroup testGroup;
	testGroup.SetName("TestGroup");
	FxAnim testAnim;
	testAnim.SetName("My First Anim");
	CHECK( testGroup.AddAnim(testAnim) == FxTrue );
	testAnim.SetName("My Second Anim");
	CHECK( testGroup.AddAnim(testAnim) == FxTrue );
	testAnim.SetName("My Third Anim");
	CHECK( testGroup.AddAnim(testAnim) == FxTrue );
	FxAnimSet testSet;
	testSet.SetAnimGroup(testGroup);
	FxActor testActor;
	testActor.SetName("TestActor");
	CHECK( testActor.ImportAnimSet(testSet) == FxTrue );
	CHECK( testActor.GetNumAnimGroups() == 2 );
	CHECK( testActor.FindAnimGroup("TestGroup") != FxInvalidIndex );
	CHECK( testActor.GetAnimGroup(1).GetNumAnims() == 3 );
	FxAnimSet testExportedAnimSet;
	CHECK( testActor.ExportAnimSet("TestGroup", testExportedAnimSet) == FxTrue );
	CHECK( testActor.GetNumAnimGroups() == 1 );
	CHECK( testActor.FindAnimGroup("TestGroup") == FxInvalidIndex );
	CHECK( testExportedAnimSet.GetName() == "TestGroup" );
	CHECK( testExportedAnimSet.GetOwningActorName() == "TestActor" );
	CHECK( testExportedAnimSet.GetAnimGroup().GetName() == "TestGroup" );
	CHECK( testExportedAnimSet.GetAnimGroup().GetNumAnims() == 3 );
	CHECK( testActor.ExportAnimSet(FxAnimGroup::Default, testExportedAnimSet) == FxFalse );
	CHECK( testActor.GetNumAnimGroups() == 1 );
}

UNITTEST( FxActor, ActorManagement )
{
	FxActor* firstActor = new FxActor();
	firstActor->SetName("firstActor");
	CHECK( FxActor::AddActor(firstActor) == FxTrue );
	FxActor* secondActor = new FxActor();
	secondActor->SetName("secondActor");
	CHECK( FxActor::AddActor(secondActor) == FxTrue );
	FxActor* thirdActor = new FxActor();
	thirdActor->SetName("thirdActor");
	CHECK( FxActor::AddActor(thirdActor) == FxTrue );
	CHECK( FxActor::FindActor(firstActor->GetName()) == firstActor );
	CHECK( FxActor::FindActor(thirdActor->GetName()) == thirdActor );
	CHECK( FxActor::FindActor(secondActor->GetName()) == secondActor );

	CHECK( FxActor::RemoveActor(FxActor::FindActor("firstActor")) == FxTrue );
	CHECK( FxActor::FindActor("firstActor") == NULL );
	FxActor::FlushActors();

	CHECK( FxActor::FindActor("firstActor") == NULL );
	CHECK( FxActor::FindActor("secondActor") == NULL );
	CHECK( FxActor::FindActor("thirdActor") == NULL );
}

#endif
