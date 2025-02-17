#ifndef TestFxCombinerNode_H__
#define TestFxCombinerNode_H__

#include "FxFaceGraphNode.h"
#include "FxFaceGraphNodeLink.h"
#include "FxCombinerNode.h"
#include "FxGenericTargetNode.h"
#include "FxArchiveStoreNull.h"
#include "FxArchiveStoreMemory.h"
using namespace OC3Ent::Face;

UNITTEST( FxCombinerNode, Construction )
{
	FxCombinerNode test;
	CHECK( test.GetMin() == 0.0f );
	CHECK( test.GetMax() == 1.0f );
	CHECK( test.GetNodeOperation() == IO_Sum );
	CHECK( test.GetNumInputLinks() == 0 );
}

UNITTEST( FxCombinerNode, Evaluation )
{
	//@todo compiled
    /*FxCombinerNode node;
	FxCombinerNode inputNode1;
	inputNode1.SetName("inputNode1");
	FxCombinerNode inputNode2;
	inputNode2.SetName("inputNode2");
	FxLinkFnParameters params;
	params.parameters.PushBack(1);
	FxFaceGraphNodeLink linkTo1("inputNode1", &inputNode1, 
		                        "linear", 
							    FxLinkFn::FindLinkFunctionType("linear"), 
							    params);
	FxFaceGraphNodeLink linkTo2("inputNode2", &inputNode2, 
							    "linear", 
							    FxLinkFn::FindLinkFunctionType("linear"), 
							    params);

	node.AddInputLink(linkTo1);
	node.AddInputLink(linkTo2);

	CHECK( 2 == node.GetNumInputLinks() );

	inputNode1.SetTrackValue(0.2f);
	inputNode2.SetTrackValue(0.3f);

	CHECK( FLOAT_EQUALITY(node.GetValue(), 0.5f) );

	node.ClearValues();
	inputNode1.ClearValues();
	inputNode2.ClearValues();

	node.SetNodeOperation(IO_Multiply);

	inputNode1.SetTrackValue(0.2f);
	inputNode2.SetTrackValue(0.3f);

	CHECK( FLOAT_EQUALITY(node.GetValue(), 0.06f) );

	node.ClearValues();
	inputNode1.ClearValues();
	inputNode2.ClearValues();

	node.SetNodeOperation(IO_Sum);

	node.SetTrackValue(0.1f);
	inputNode1.SetTrackValue(0.2f);
	inputNode2.SetTrackValue(0.3f);
	
	CHECK( FLOAT_EQUALITY(node.GetValue(), 0.6f) );

	node.ClearValues();
	inputNode1.ClearValues();
	inputNode2.ClearValues();

	node.SetNodeOperation(IO_Multiply);

	node.SetTrackValue(0.1f);
	inputNode1.SetTrackValue(0.2f);
	inputNode2.SetTrackValue(0.3f);

	CHECK( FLOAT_EQUALITY(node.GetValue(), 0.16f) );

	node.ClearValues();
	inputNode1.ClearValues();
	inputNode2.ClearValues();

	node.SetNodeOperation(IO_Sum);

	inputNode1.SetTrackValue(0.1f);
	inputNode1.SetUserValue(0.1f, VO_Add);
	inputNode2.SetTrackValue(0.5f);
	inputNode2.SetUserValue(0.1f, VO_Replace);

	node.SetUserValue(0.5f, VO_Add);

	CHECK( FLOAT_EQUALITY(node.GetValue(), 0.8f) );
*/
}

UNITTEST( FxCombinerNode, InputLinkManipulation )
{
	FxCombinerNode node;
	FxCombinerNode inputNode1;
	inputNode1.SetName("inputNode1");
	FxCombinerNode inputNode2;
	inputNode2.SetName("inputNode2");
	FxLinkFnParameters params;
	params.parameters.PushBack(1);
	FxFaceGraphNodeLink linkTo1("inputNode1", &inputNode1, 
							    "linear", 
							    FxLinkFn::FindLinkFunctionType("linear"), 
							    params);
	FxFaceGraphNodeLink linkTo2("inputNode2", &inputNode2, 
							    "linear", 
							    FxLinkFn::FindLinkFunctionType("linear"), 
							    params);

	node.AddInputLink(linkTo1);
	node.AddInputLink(linkTo2);
	CHECK( 2 == node.GetNumInputLinks() );
	
	node.RemoveInputLink(0);
	CHECK( 1 == node.GetNumInputLinks() );

	//@todo compiled
	/*inputNode2.SetTrackValue(1.0f);
	CHECK( FLOAT_EQUALITY(node.GetValue(), 1.0f ));

	node.ClearValues();
	inputNode2.ClearValues();

	node.ModifyInputLink(0, linkTo1);
	inputNode1.SetTrackValue(0.5f);
	CHECK( FLOAT_EQUALITY(node.GetValue(), 0.5f) );*/
}

class FxMaterialParameterNode : public FxFaceGraphNode
{
	// Declare the class.
	FX_DECLARE_CLASS(FxMaterialParameterNode, FxFaceGraphNode)
	// Disable copy construction and assignment.
	FX_NO_COPY_OR_ASSIGNMENT(FxMaterialParameterNode)
public:
	// Constructor.
	FxMaterialParameterNode()
	{
		FxFaceGraphNodeUserProperty materialSlotProperty(UPT_Integer);		materialSlotProperty.SetName("Material Slot Id");		materialSlotProperty.SetIntegerProperty(0);		AddUserProperty(materialSlotProperty);		FxFaceGraphNodeUserProperty materialParameterNameProperty(UPT_String);		materialParameterNameProperty.SetName("Parameter Name");		materialParameterNameProperty.SetStringProperty(FxString("Param"));		AddUserProperty(materialParameterNameProperty);
	}

	// Destructor.
	virtual ~FxMaterialParameterNode()
	{
	}

	// Clones the material parameter node.
	virtual FxFaceGraphNode* Clone( void )
	{
		FxMaterialParameterNode* pNode = FxCast<FxMaterialParameterNode>(FxMaterialParameterNode::ConstructObject());
		CopyData(pNode);
		return pNode;
	}

	// Copies the data from this object into the other object.
	virtual void CopyData( FxFaceGraphNode* pOther )
	{
		Super::CopyData(pOther);
	}

	// Serializes an FxUnMaterialParameterNode to an archive.
	virtual void Serialize( FxArchive& arc )
	{
		Super::Serialize(arc);

		arc.SerializeClassVersion("FxMaterialParameterNode", FxTrue, GetCurrentVersion());
	}

};

#define kCurrentFxMaterialParameterNodeVersion 0
FX_IMPLEMENT_CLASS(FxMaterialParameterNode, kCurrentFxMaterialParameterNodeVersion, FxFaceGraphNode)

UNITTEST(FxFaceGraphNode, UserPropertySerialization)
{
	FxMaterialParameterNode testNode;
#ifdef FX_DISALLOW_SPACES_IN_NAMES
	FxString testNodeName("Test_Node");
#else
	FxString testNodeName("Test Node");
#endif
	testNode.SetName(testNodeName.GetData());
	testNode.SetMax(5.0f);

	FxFaceGraphNodeUserProperty materialSlotProperty(UPT_Integer);	materialSlotProperty.SetName("Material Slot Id");	materialSlotProperty.SetIntegerProperty(1);	FxFaceGraphNodeUserProperty materialParameterNameProperty(UPT_String);	materialParameterNameProperty.SetName("Parameter Name");	materialParameterNameProperty.SetStringProperty(FxString("The Good Value"));	testNode.ReplaceUserProperty(materialSlotProperty);
	testNode.ReplaceUserProperty(materialParameterNameProperty);

	OC3Ent::Face::FxArchive directoryCreater(OC3Ent::Face::FxArchiveStoreNull::Create(), OC3Ent::Face::FxArchive::AM_CreateDirectory);
	directoryCreater.Open();
	directoryCreater << testNode;
	OC3Ent::Face::FxArchiveStoreMemory* savingMemoryStore = OC3Ent::Face::FxArchiveStoreMemory::Create( NULL, 0 );
	OC3Ent::Face::FxArchive savingArchive( savingMemoryStore, OC3Ent::Face::FxArchive::AM_Save );
	savingArchive.Open();
	savingArchive.SetInternalDataState(directoryCreater.GetInternalData());
	savingArchive << testNode;
	
	OC3Ent::Face::FxArchiveStoreMemory* loadingMemoryStore = OC3Ent::Face::FxArchiveStoreMemory::Create( savingMemoryStore->GetMemory(), savingMemoryStore->GetSize() );
	OC3Ent::Face::FxArchive loadingArchive( loadingMemoryStore, OC3Ent::Face::FxArchive::AM_Load );
	loadingArchive.Open();
#ifdef NO_SAVE_VERSION
	CHECK(loadingArchive.IsValid() == FxFalse);
#else
	FxMaterialParameterNode loadNode;
	loadingArchive << loadNode;

#ifdef FX_DISALLOW_SPACES_IN_NAMES
	FxString materialSlotId("Material_Slot_Id");
	FxString parameterName("Parameter_Name");
#else
	FxString materialSlotId("Material Slot Id");
	FxString parameterName("Parameter Name");
#endif

	CHECK(testNodeName == loadNode.GetNameAsCstr());
	CHECK(FxRealEquality(loadNode.GetMax(), 5.0f));
	CHECK(materialSlotId == loadNode.GetUserProperty(0).GetNameAsCstr());
	CHECK(loadNode.GetUserProperty(0).GetIntegerProperty() == 1);
	CHECK(parameterName == loadNode.GetUserProperty(1).GetNameAsCstr());
	CHECK(FxString("The Good Value") == loadNode.GetUserProperty(1).GetStringProperty());
#endif
}

#endif
