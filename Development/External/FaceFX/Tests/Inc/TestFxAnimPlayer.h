
#ifndef TestFxAnimPlayer_H__
#define TestFxAnimPlayer_H__

//@todo compiled This test is no longer valid because there is no FxAnimPlayer
//      anymore.  Should we rip this out or somehow re-write it?
/*
#include "FxAnimPlayer.h"
using namespace OC3Ent::Face;

UNITTEST( FxAnimPlayer, Playing )
{
	FxActor actor;
	actor.SetName("testActor");

	FxBone refBone;
	refBone.SetName("testRefBone");
	refBone.SetPos(FxVec3(0.0f, 0.0f, 0.0f));
	actor.GetMasterBoneList().AddRefBone(refBone);

	FxBone refBone2;
	refBone2.SetName("testRefBone2");
	refBone2.SetPos(FxVec3(0.0f, 0.0f, 0.0f));
	actor.GetMasterBoneList().AddRefBone(refBone2);

	FxBone targetBone;
	targetBone.SetName("testRefBone");
	targetBone.SetPos(FxVec3(1.0f, 1.0f, 1.0f));

	FxBone targetBone2;
	targetBone2.SetName("testRefBone2");
	targetBone2.SetPos(FxVec3(1.0f, 1.0f, 1.0f));

	FxBonePoseNode* pBonePoseNode = new FxBonePoseNode();
	pBonePoseNode->SetName("test");
	pBonePoseNode->AddBone(targetBone);
	pBonePoseNode->AddBone(targetBone2);
	actor.GetFaceGraph().AddNode(pBonePoseNode);
	CHECK( 1 == actor.GetFaceGraph().GetNumNodes() );
	
	actor.GetMasterBoneList().Link(actor.GetFaceGraph());

	actor.GetMasterBoneList().SetRefBoneWeight(refBone.GetName(), 0.0f);
	actor.GetMasterBoneList().SetRefBoneWeight(refBone2.GetName(), 0.75f);

	FxAnim anim;
	anim.SetName("testAnim");
	FxAnimBoneWeight animBoneWeight;
	animBoneWeight.boneName = targetBone.GetName();
	animBoneWeight.boneWeight = 0.5f;
	CHECK( FxTrue == anim.AddBoneWeight(animBoneWeight) );
	CHECK( FxFalse == anim.AddBoneWeight(animBoneWeight) );
	CHECK( 1 == anim.GetNumBoneWeights() );
	FxAnimCurve curve;
	curve.SetName("test");
	FxAnimKey key1,key2,key3;
	key1.SetTime(0.0f);
	key1.SetValue(0.0f);
	key2.SetTime(2.0f);
	key2.SetValue(1.0f);
	key3.SetTime(4.0f);
	key3.SetValue(0.0f);
	curve.InsertKey(key1);
	curve.InsertKey(key2);
	curve.InsertKey(key3);
	CHECK( 3 == curve.GetNumKeys() );
	anim.AddAnimCurve(curve);
	CHECK( 1 == anim.GetNumAnimCurves() );

	actor.AddAnimGroup("testGroup");
	CHECK( 2 == actor.GetNumAnimGroups() );
	actor.AddAnim("testGroup", anim);
	CHECK( 0 == actor.GetAnimGroup(0).GetNumAnims() );
	CHECK( 1 == actor.GetAnimGroup(1).GetNumAnims() );

	FxVec3 bonePos, boneScale;
	FxQuat boneRot;
	FxReal boneWeight;
	
	FxAnimPlayer player(&actor);
	CHECK( player.Play("testGroup", "noAnim") == FxFalse );
	CHECK( player.IsPlaying() == FxFalse );
	CHECK( player.GetState() == APS_Stopped );
	CHECK( player.Play("noGroup", "noAnim") == FxFalse );
	CHECK( player.IsPlaying() == FxFalse );
	CHECK( player.GetState() == APS_Stopped );
	CHECK( player.Play("testAnim", "testGroup") == FxTrue );
	CHECK( player.GetState() == APS_Playing );
	CHECK( player.IsPlaying() == FxTrue );
	CHECK( player.Tick(0.0, -1.0f) == APS_StartAudio );
	CHECK( player.Tick(0.0, -1.0f) == APS_Playing );
	actor.GetMasterBoneList().UpdateBonePoses(actor.GetFaceGraph());
	CHECK( FLOAT_EQUALITY(pBonePoseNode->GetValue(), 0.0f) );
	actor.GetMasterBoneList().GetBlendedBone(0, bonePos, boneRot, boneScale, boneWeight);
	CHECK( bonePos   == refBone.GetPos() );
	CHECK( boneRot   == refBone.GetRot() );
	CHECK( boneScale == refBone.GetScale() );
	CHECK( boneWeight == 0.5f );
	actor.GetMasterBoneList().GetBlendedBone(1, bonePos, boneRot, boneScale, boneWeight);
	CHECK( bonePos   == refBone2.GetPos() );
	CHECK( boneRot   == refBone2.GetRot() );
	CHECK( boneScale == refBone2.GetScale() );
	CHECK( boneWeight == 0.75f );
	CHECK( player.Tick(2.0, -1.0f) == APS_Playing );
	actor.GetMasterBoneList().UpdateBonePoses(actor.GetFaceGraph());
	CHECK( FLOAT_EQUALITY(pBonePoseNode->GetValue(), 1.0f) );
	actor.GetMasterBoneList().GetBlendedBone(0, bonePos, boneRot, boneScale, boneWeight);
	CHECK( bonePos   == targetBone.GetPos() );
	CHECK( boneRot   == targetBone.GetRot() );
	CHECK( boneScale == targetBone.GetScale() );
	CHECK( boneWeight == 0.5f );
	actor.GetMasterBoneList().GetBlendedBone(1, bonePos, boneRot, boneScale, boneWeight);
	CHECK( bonePos   == targetBone2.GetPos() );
	CHECK( boneRot   == targetBone2.GetRot() );
	CHECK( boneScale == targetBone2.GetScale() );
	CHECK( boneWeight == 0.75f );
	CHECK( player.Tick(4.0, -1.0f) == APS_Playing );
	actor.GetMasterBoneList().UpdateBonePoses(actor.GetFaceGraph());
	CHECK( FLOAT_EQUALITY(pBonePoseNode->GetValue(), 0.0f) );
	actor.GetMasterBoneList().GetBlendedBone(0, bonePos, boneRot, boneScale, boneWeight);
	CHECK( bonePos   == refBone.GetPos() );
	CHECK( boneRot   == refBone.GetRot() );
	CHECK( boneScale == refBone.GetScale() );
	CHECK( boneWeight == 0.5f );
	actor.GetMasterBoneList().GetBlendedBone(1, bonePos, boneRot, boneScale, boneWeight);
	CHECK( bonePos   == refBone2.GetPos() );
	CHECK( boneRot   == refBone2.GetRot() );
	CHECK( boneScale == refBone2.GetScale() );
	CHECK( boneWeight == 0.75f );
	CHECK( player.Tick(6.0, -1.0f) == APS_Stopped );
	CHECK( player.GetState() == APS_Stopped );
	CHECK( player.IsPlaying() == FxFalse );
	
	CHECK( FxTrue == anim.RemoveBoneWeight(targetBone.GetName()) );
	CHECK( FxFalse == anim.RemoveBoneWeight(targetBone.GetName()) );
	CHECK( 0 == anim.GetNumBoneWeights() );
}

UNITTEST(FxAnimPlayer, SkipNegativeTime)
{
	FxActor actor;
	actor.SetName("testActor");

	FxCombinerNode* pCombinerNode = new FxCombinerNode();
	pCombinerNode->SetName("test");
	actor.GetFaceGraph().AddNode(pCombinerNode);
	CHECK( 1 == actor.GetFaceGraph().GetNumNodes() );

	FxAnim anim;
	anim.SetName("testAnim");
	FxAnimCurve curve;
	curve.SetName("test");
	FxAnimKey key1,key2,key3;
	key1.SetTime(-1.0f);
	key1.SetValue(0.5f);
	key2.SetTime(0.0f);
	key2.SetValue(1.0f);
	key3.SetTime(1.0f);
	key3.SetValue(0.0f);
	curve.InsertKey(key1);
	curve.InsertKey(key2);
	curve.InsertKey(key3);
	anim.AddAnimCurve(curve);
	anim.FindBounds();

	actor.AddAnimGroup("testGroup");
	CHECK( 2 == actor.GetNumAnimGroups() );
	actor.AddAnim("testGroup", anim);
	CHECK( 0 == actor.GetAnimGroup(0).GetNumAnims() );
	CHECK( 1 == actor.GetAnimGroup(1).GetNumAnims() );

	actor.Link();

	FxAnimPlayer player(&actor);
	CHECK( player.Play("testAnim", "testGroup") == FxTrue );
	CHECK( player.GetState() == APS_Playing );
	CHECK( player.IsPlaying() == FxTrue );
	CHECK( player.Tick(0.0, -1.0) == APS_Playing );
	CHECK( actor.GetFaceGraph().FindNode("test")->GetTrackValue() == 0.5f );
	CHECK( player.Tick(1.0, -1.0) == APS_StartAudio);
	CHECK( actor.GetFaceGraph().FindNode("test")->GetTrackValue() == 1.0f );
	player.Stop();

	CHECK( player.Play("testAnim", "testGroup", FxTrue) == FxTrue );
	CHECK( player.GetState() == APS_Playing );
	CHECK( player.IsPlaying() == FxTrue );
	CHECK( player.Tick(0.0, -1.0) == APS_StartAudio );
	CHECK( actor.GetFaceGraph().FindNode("test")->GetTrackValue() == 1.0f );
	CHECK( player.Tick(1.0, -1.0) == APS_Playing);
	CHECK( actor.GetFaceGraph().FindNode("test")->GetTrackValue() == 0.0f );

}
*/
#endif
