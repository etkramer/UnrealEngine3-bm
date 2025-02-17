//------------------------------------------------------------------------------
// The function publishing interface for the Max plug-in
//
// Owner: Doug Perkowski
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxMaxScriptInterface.h"
#include "FxMaxGUI.h"
#include "FxToolLog.h"

#define FACEFX_INTERFACE_CLASS_ID Class_ID(0x5c0d062f, 0x73283d92)
#define FX_MAXSCRIPT_INTERFACE Interface_ID(0xe46c7f, 0x779a686f)
#define GetFxMaxScriptInterface(obj) ((FxMaxScriptInterface*)obj->GetInterface(FX_MAXSCRIPT_INTERFACE))

// Helper function for returning a Tab<float> structure given an FxArray<FxReal>
Tab<float>* FxArrayToTabFloat(OC3Ent::Face::FxArray<OC3Ent::Face::FxReal> fxArrayReal)
{
	Tab<float>* returnValue = new Tab<float>;
	for( OC3Ent::Face::FxSize i = 0; i < fxArrayReal.Length(); i++ )
	{
		float* myfloat = new float;
		*myfloat = fxArrayReal[i];
		returnValue->Insert(i,1,myfloat);
	}
	return returnValue;
}



static FxMaxScriptInterfaceClassDesc fxMaxScriptInterfaceClassDesc;

extern FPInterfaceDesc FxMaxScript_mixininterface;

// Returns the class description for the FaceFX Max plug-in function publishing.
ClassDesc2* GetFxMaxScriptInterfaceClassDesc( void )
{
	return &fxMaxScriptInterfaceClassDesc;
}

enum {	fn_fxloadactor, fn_fxcreateactor, fn_fxsaveactor, fn_fxgetactorname, fn_fxexportrefpose, 
		fn_fximportrefpose, fn_fxexportbonepose, fn_fxbatchexportboneposes, fn_fximportbonepose, 
		fn_fxbatchimportboneposes, fn_fximportanim, fn_fxclean, fn_fxgetanimgroups, 
		fn_fxgetanims, fn_fxgetcurves, fn_fxgetnodes, fn_fxgetrefbones, fn_fxgetbones, 
		fn_fxgetrawcurvekeyvalues, fn_fxgetrawcurvekeytimes, fn_fxgetrawcurvekeyslopein, 
		fn_fxgetrawcurvekeyslopeout, fn_fxgetbakedcurvekeyvalues, 
		fn_fxgetbakedcurvekeytimes, fn_fxgetbakedcurvekeyslopein, 
		fn_fxgetbakedcurvekeyslopeout, fn_fxgetanimduration, fn_fxdeleteallkeys, fn_fxinsertkey,
		fn_fxsetdisplaywarningdialogs, fn_fxsetnormalizescale, fn_fxisnosave, fn_fxismoddeveloper,}; 

interface IFxMaxScript : public FPMixinInterface
{

    BEGIN_FUNCTION_MAP 

		VFN_1 (fn_fxloadactor, fxloadactor, TYPE_STRING );
		VFN_1 (fn_fxcreateactor, fxcreateactor, TYPE_STRING );
		VFN_1 (fn_fxsaveactor, fxsaveactor, TYPE_STRING );
		FN_0  (fn_fxgetactorname, TYPE_TSTR, fxgetactorname );
		VFN_2 (fn_fxexportrefpose, fxexportrefpose, TYPE_INT, TYPE_STRING_TAB );
		VFN_1 (fn_fximportrefpose, fximportrefpose, TYPE_INT );
		VFN_2 (fn_fxexportbonepose, fxexportbonepose, TYPE_INT, TYPE_STRING );
		VFN_1 (fn_fxbatchexportboneposes, fxbatchexportboneposes, TYPE_STRING );
		VFN_2 (fn_fximportbonepose, fximportbonepose, TYPE_INT, TYPE_STRING );
		VFN_1 (fn_fxbatchimportboneposes, fxbatchimportboneposes, TYPE_STRING );
		VFN_2 (fn_fximportanim, fximportanim, TYPE_STRING, TYPE_STRING );
		VFN_0 (fn_fxclean, fxclean );
		FN_0  (fn_fxgetanimgroups, TYPE_TSTR_TAB_BV, fxgetanimgroups );
		FN_1  (fn_fxgetanims, TYPE_TSTR_TAB_BV, fxgetanims, TYPE_STRING );
		FN_2  (fn_fxgetcurves, TYPE_TSTR_TAB_BV, fxgetcurves, TYPE_STRING, TYPE_STRING );
		FN_1  (fn_fxgetnodes, TYPE_TSTR_TAB_BV, fxgetnodes, TYPE_STRING );
		FN_0  (fn_fxgetrefbones, TYPE_TSTR_TAB_BV, fxgetrefbones );
		FN_1  (fn_fxgetbones, TYPE_TSTR_TAB_BV, fxgetbones, TYPE_STRING );
		FN_3  (fn_fxgetrawcurvekeyvalues, TYPE_FLOAT_TAB, fxgetrawcurvekeyvalues, TYPE_STRING, TYPE_STRING, TYPE_STRING );
		FN_3  (fn_fxgetrawcurvekeytimes, TYPE_FLOAT_TAB, fxgetrawcurvekeytimes, TYPE_STRING, TYPE_STRING, TYPE_STRING );
		FN_3  (fn_fxgetrawcurvekeyslopein, TYPE_FLOAT_TAB, fxgetrawcurvekeyslopein, TYPE_STRING, TYPE_STRING, TYPE_STRING );
		FN_3  (fn_fxgetrawcurvekeyslopeout, TYPE_FLOAT_TAB, fxgetrawcurvekeyslopeout, TYPE_STRING, TYPE_STRING, TYPE_STRING );
		FN_3  (fn_fxgetbakedcurvekeyvalues, TYPE_FLOAT_TAB, fxgetbakedcurvekeyvalues, TYPE_STRING, TYPE_STRING, TYPE_STRING );
		FN_3  (fn_fxgetbakedcurvekeytimes, TYPE_FLOAT_TAB, fxgetbakedcurvekeytimes, TYPE_STRING, TYPE_STRING, TYPE_STRING );
		FN_3  (fn_fxgetbakedcurvekeyslopein, TYPE_FLOAT_TAB, fxgetbakedcurvekeyslopein, TYPE_STRING, TYPE_STRING, TYPE_STRING );
		FN_3  (fn_fxgetbakedcurvekeyslopeout, TYPE_FLOAT_TAB, fxgetbakedcurvekeyslopeout, TYPE_STRING, TYPE_STRING, TYPE_STRING );
		FN_2  (fn_fxgetanimduration, TYPE_FLOAT, fxgetanimduration, TYPE_STRING, TYPE_STRING );
		VFN_3 (fn_fxdeleteallkeys, fxdeleteallkeys, TYPE_STRING, TYPE_STRING, TYPE_STRING );
		FN_7  (fn_fxinsertkey, TYPE_INT, fxinsertkey, TYPE_STRING, TYPE_STRING, TYPE_STRING, TYPE_INT, TYPE_FLOAT, TYPE_FLOAT, TYPE_FLOAT );
		VFN_1  (fn_fxsetdisplaywarningdialogs, fxsetdisplaywarningdialogs, TYPE_STRING );
		VFN_1  (fn_fxsetnormalizescale, fxsetnormalizescale, TYPE_STRING );
		FN_0 (fn_fxisnosave, TYPE_BOOL, fxisnosave);
		FN_0 (fn_fxismoddeveloper, TYPE_BOOL, fxismoddeveloper);

    END_FUNCTION_MAP 

    FPInterfaceDesc* GetDesc() { return &FxMaxScript_mixininterface; }

	virtual void		fxloadactor( TSTR path ) = 0;
	virtual void		fxcreateactor( TSTR name ) = 0;
	virtual void		fxsaveactor( TSTR path ) = 0;
	virtual TSTR*		fxgetactorname( ) = 0;
	virtual void		fxexportrefpose(int frame, Tab<TCHAR*>* bonenames ) = 0;
	virtual void		fximportrefpose( int frame ) = 0;
	virtual void		fxexportbonepose( int frame, TSTR pose ) = 0;
	virtual void		fxbatchexportboneposes( TSTR path ) = 0;
	virtual void		fximportbonepose( int frame, TSTR pose ) = 0;
	virtual void		fxbatchimportboneposes( TSTR path )  = 0;
	virtual void		fximportanim( TSTR group, TSTR anim ) = 0;
	virtual void		fxclean( void ) = 0;
	virtual Tab<TSTR*>  fxgetanimgroups( void ) = 0;
	virtual Tab<TSTR*>  fxgetanims( TSTR group ) = 0;
	virtual Tab<TSTR*>	fxgetcurves( TSTR group, TSTR anim ) = 0;
	virtual Tab<TSTR*>	fxgetnodes( TSTR type ) = 0;
	virtual Tab<TSTR*>  fxgetrefbones( void ) = 0;
	virtual Tab<TSTR*>  fxgetbones( TSTR pose ) = 0;
	virtual Tab<float>*	fxgetrawcurvekeyvalues( TSTR group, TSTR anim, TSTR curve ) = 0;
	virtual Tab<float>* fxgetrawcurvekeytimes( TSTR group, TSTR anim, TSTR curve ) = 0;
	virtual Tab<float>*	fxgetrawcurvekeyslopein( TSTR group, TSTR anim, TSTR curve ) = 0;
	virtual Tab<float>*	fxgetrawcurvekeyslopeout( TSTR group, TSTR anim, TSTR curve ) = 0;
	virtual Tab<float>* fxgetbakedcurvekeyvalues( TSTR group, TSTR anim, TSTR curve ) = 0;
	virtual Tab<float>* fxgetbakedcurvekeytimes( TSTR group, TSTR anim, TSTR curve ) = 0;
	virtual Tab<float>* fxgetbakedcurvekeyslopein( TSTR group, TSTR anim, TSTR curve ) = 0;
	virtual Tab<float>* fxgetbakedcurvekeyslopeout( TSTR group, TSTR anim, TSTR curve ) = 0;
	virtual float		fxgetanimduration( TSTR group, TSTR anim ) = 0;
	virtual void		fxdeleteallkeys( TSTR group, TSTR anim, TSTR curve ) = 0;
	virtual int			fxinsertkey( TSTR group, TSTR anim, TSTR curve, int frame, float value, float inslope, float outslope ) = 0;
	virtual void		fxsetdisplaywarningdialogs( TSTR toggle ) = 0;
	virtual void		fxsetnormalizescale( TSTR toggle ) = 0;
	virtual bool		fxisnosave() = 0;
	virtual bool		fxismoddeveloper() = 0;
};



class FaceFX : public ReferenceTarget, public IFxMaxScript
{
	public:
		FaceFX(){ }
		void DeleteThis(){ delete this; }
		Class_ID ClassID(){ return FACEFX_INTERFACE_CLASS_ID; }
		SClass_ID SuperClassID(){ return REF_TARGET_CLASS_ID; }
		void GetClassName(TSTR& s){ s = "FaceFX"; }
		int IsKeyable(){ return 0;}
		RefResult NotifyRefChanged( Interval i, RefTargetHandle rth, PartID& pi,RefMessage rm )
		{
			return REF_SUCCEED;
		}

		BaseInterface* GetInterface(Interface_ID id) 
		{ 
			if (id == FX_MAXSCRIPT_INTERFACE) 
				return (IFxMaxScript*)this; 
			else 
				return ReferenceTarget::GetInterface(id); 
		}

		void fxloadactor(TSTR path) 
		{ 
			if( !OC3Ent::Face::FxMaxData::maxInterface.LoadActor(path.data()) )
			{
				FxToolLog("fxloadactor maxscript command failed!");
			}
			UpdateMainFaceFxPanel();
		}

		void fxcreateactor(TSTR name) 
		{ 
			if( !OC3Ent::Face::FxMaxData::maxInterface.CreateActor(name.data()) )
			{
				FxToolLog("fxcreateactor maxscript command failed!");
			}
			UpdateMainFaceFxPanel();
		}

		void fxsaveactor(TSTR path) 
		{ 
			if( !OC3Ent::Face::FxMaxData::maxInterface.SaveActor(path.data()) )
			{
				FxToolLog("fxsaveactor maxscript command failed!");
			}
		}
		TSTR* fxgetactorname( void )
		{
			TSTR* returnValue = new TSTR;
			*returnValue = OC3Ent::Face::FxMaxData::maxInterface.GetActorName().GetData();
			return returnValue;
		}

		// This function is called from Maxscript like this:
		//		FaceFXObject.fxexportrefpose 0 #("BN_Head", "BN_Neck");
		void fxexportrefpose(int frame, Tab<TCHAR*>* bonenames )
		{
			OC3Ent::Face::FxArray<OC3Ent::Face::FxString> refBones;
			for( OC3Ent::Face::FxSize i = 0; i < bonenames->Count(); ++i )
			{
				OC3Ent::Face::FxString name = (OC3Ent::Face::FxChar*)(*bonenames)[i];
				refBones.PushBack(name);
			}
			if( !OC3Ent::Face::FxMaxData::maxInterface.ExportRefPose(frame, refBones) )
			{
				FxToolLog("fxexportrefpose maxscript command failed!");
			}
			UpdateMainFaceFxPanel();
		}
		void fximportrefpose( int frame )
		{
			OC3Ent::Face::FxMaxData::maxInterface.ImportRefPose(frame);
		}

		void fxexportbonepose( int frame, TSTR pose )
		{
			OC3Ent::Face::FxMaxData::maxInterface.ExportBonePose(frame, pose.data());
			UpdateMainFaceFxPanel();
		}
		void fxbatchexportboneposes( TSTR path ) 
		{
			OC3Ent::Face::FxMaxData::maxInterface.BatchExportBonePoses(path.data());
			UpdateMainFaceFxPanel();
		}
		void fximportbonepose( int frame, TSTR pose )
		{
			OC3Ent::Face::FxMaxData::maxInterface.ImportBonePose(pose.data(),frame);
			UpdateMainFaceFxPanel();
		}
		void fxbatchimportboneposes( TSTR path ) 
		{
			OC3Ent::Face::FxInt32 startFrame = 0;
			OC3Ent::Face::FxInt32 endFrame   = 0;
			OC3Ent::Face::FxMaxData::maxInterface.BatchImportBonePoses(path.data(),
				startFrame,endFrame);
			UpdateMainFaceFxPanel();
		}
		void fximportanim( TSTR group, TSTR anim )
		{
			OC3Ent::Face::FxMaxData::maxInterface.ImportAnim(group.data(), 
				anim.data(), static_cast<OC3Ent::Face::FxReal>(GetFrameRate()));
			UpdateMainFaceFxPanel();
		}
		void fxclean( void )
		{
			OC3Ent::Face::FxMaxData::maxInterface.CleanBonesMorph();
			UpdateMainFaceFxPanel();
		}
		Tab<TSTR*>  fxgetanimgroups( void )
		{
			Tab<TSTR*> returnValue;
			TSTR str;
			TSTR* t;
			for( OC3Ent::Face::FxSize i = 0; i < OC3Ent::Face::FxMaxData::maxInterface.GetNumAnimGroups(); ++i )
			{
				OC3Ent::Face::FxString fxstr = OC3Ent::Face::FxMaxData::maxInterface.GetAnimGroupName(i);
				t = new TSTR (fxstr.GetData());
				returnValue.Append(1,&t,1);
			}
			return returnValue;
		}        
		Tab<TSTR*>  fxgetanims( TSTR group )
		{
			Tab<TSTR*> returnValue;
			TSTR str;
			TSTR* t;
			for( OC3Ent::Face::FxSize i = 0; i < OC3Ent::Face::FxMaxData::maxInterface.GetNumAnims(group.data()); ++i )
			{
				OC3Ent::Face::FxString fxstr(OC3Ent::Face::FxMaxData::maxInterface.GetAnimName(group.data(), i));
				t = new TSTR (fxstr.GetData());
				returnValue.Append(1,&t,1);
			}
			return returnValue;
		}
		Tab<TSTR*>	fxgetcurves( TSTR group, TSTR anim )
		{
			Tab<TSTR*> returnValue;
			TSTR str;
			TSTR* t;
			for( OC3Ent::Face::FxSize i = 0; i < OC3Ent::Face::FxMaxData::maxInterface.GetNumAnimCurves(group.data(), anim.data()); ++i )
			{
				OC3Ent::Face::FxString fxstr(OC3Ent::Face::FxMaxData::maxInterface.GetAnimCurveName(group.data(), anim.data(), i));
				t = new TSTR (fxstr.GetData());
				returnValue.Append(1,&t,1);
			}
			return returnValue;
		}
		Tab<TSTR*>	fxgetnodes( TSTR type )
		{
			Tab<TSTR*> returnValue;
			TSTR str;
			TSTR* t;
			OC3Ent::Face::FxArray<OC3Ent::Face::FxString> fxstrArray = OC3Ent::Face::FxMaxData::maxInterface.GetNodesOfType(type.data());
			for( OC3Ent::Face::FxSize i = 0; i < fxstrArray.Length(); ++i )
			{
				OC3Ent::Face::FxString fxstr(fxstrArray[i]);
				t = new TSTR (fxstr.GetData());
				returnValue.Append(1,&t,1);
			}
			return returnValue;
		}
		Tab<TSTR*>  fxgetrefbones( void )
		{
			Tab<TSTR*> returnValue;
			TSTR str;
			TSTR* t;
			for( OC3Ent::Face::FxSize i = 0; i < OC3Ent::Face::FxMaxData::maxInterface.GetNumRefBones(); ++i )
			{
				OC3Ent::Face::FxString fxstr(OC3Ent::Face::FxMaxData::maxInterface.GetRefBoneName(i));
				t = new TSTR (fxstr.GetData());
				returnValue.Append(1,&t,1);
			}
			return returnValue;
		}
		Tab<TSTR*> fxgetbones( TSTR pose )
		{
			Tab<TSTR*> returnValue;
			TSTR str;
			TSTR* t;
			for( OC3Ent::Face::FxSize i = 0; i < OC3Ent::Face::FxMaxData::maxInterface.GetNumBones(pose.data()); ++i )
			{
				OC3Ent::Face::FxString fxstr(
					OC3Ent::Face::FxMaxData::maxInterface.GetBoneName(pose.data(), i));
				t = new TSTR (fxstr.GetData());
				returnValue.Append(1,&t,1);
			}
			return returnValue;
		}
		Tab<float>*	fxgetrawcurvekeyvalues( TSTR group, TSTR anim, TSTR curve )
		{
			OC3Ent::Face::FxArray<OC3Ent::Face::FxReal> fxRealArray = 
				OC3Ent::Face::FxMaxData::maxInterface.GetRawCurveKeyValues
				(group.data(), anim.data(), curve.data());
			return FxArrayToTabFloat(fxRealArray);
		}
		Tab<float>*  fxgetrawcurvekeytimes( TSTR group, TSTR anim, TSTR curve )
		{
			OC3Ent::Face::FxArray<OC3Ent::Face::FxReal> fxRealArray = 
				OC3Ent::Face::FxMaxData::maxInterface.GetRawCurveKeyTimes
				(group.data(), anim.data(), curve.data());
			return FxArrayToTabFloat(fxRealArray);
		}
		Tab<float>*	fxgetrawcurvekeyslopein( TSTR group, TSTR anim, TSTR curve )
		{
			OC3Ent::Face::FxArray<OC3Ent::Face::FxReal> fxRealArray = 
				OC3Ent::Face::FxMaxData::maxInterface.GetRawCurveKeySlopeIn
				(group.data(), anim.data(), curve.data());
			return FxArrayToTabFloat(fxRealArray);
		}
		Tab<float>*	fxgetrawcurvekeyslopeout( TSTR group, TSTR anim, TSTR curve )
		{
			OC3Ent::Face::FxArray<OC3Ent::Face::FxReal> fxRealArray = 
				OC3Ent::Face::FxMaxData::maxInterface.GetRawCurveKeySlopeOut
				(group.data(), anim.data(), curve.data());
			return FxArrayToTabFloat(fxRealArray);
		}
		Tab<float>* fxgetbakedcurvekeyvalues( TSTR group, TSTR anim, TSTR curve )
		{
			OC3Ent::Face::FxArray<OC3Ent::Face::FxReal> fxRealArray = 
				OC3Ent::Face::FxMaxData::maxInterface.GetBakedCurveKeyValues
				(group.data(), anim.data(), curve.data());
			return FxArrayToTabFloat(fxRealArray);
		}
		Tab<float>* fxgetbakedcurvekeytimes( TSTR group, TSTR anim, TSTR curve )
		{
			OC3Ent::Face::FxArray<OC3Ent::Face::FxReal> fxRealArray = 
				OC3Ent::Face::FxMaxData::maxInterface.GetBakedCurveKeyTimes
				(group.data(), anim.data(), curve.data());
			return FxArrayToTabFloat(fxRealArray);
		}
		Tab<float>* fxgetbakedcurvekeyslopein( TSTR group, TSTR anim, TSTR curve )
		{
			OC3Ent::Face::FxArray<OC3Ent::Face::FxReal> fxRealArray = 
				OC3Ent::Face::FxMaxData::maxInterface.GetBakedCurveKeySlopeIn
				(group.data(), anim.data(), curve.data());
			return FxArrayToTabFloat(fxRealArray);
		}
		Tab<float>* fxgetbakedcurvekeyslopeout( TSTR group, TSTR anim, TSTR curve )
		{
			OC3Ent::Face::FxArray<OC3Ent::Face::FxReal> fxRealArray = 
				OC3Ent::Face::FxMaxData::maxInterface.GetBakedCurveKeySlopeOut
				(group.data(), anim.data(), curve.data());
			return FxArrayToTabFloat(fxRealArray);
		}
		float fxgetanimduration( TSTR group, TSTR anim )
		{
			float returnValue = OC3Ent::Face::FxMaxData::maxInterface.GetAnimDuration
				(group.data(), anim.data());
			return returnValue;
		}
		void fxdeleteallkeys( TSTR group, TSTR anim, TSTR curve )
		{
			OC3Ent::Face::FxMaxData::maxInterface.DeleteAllKeys(group.data(), anim.data(), curve.data());
		}
		int fxinsertkey( TSTR group, TSTR anim, TSTR curve, int frame, float value, float inslope, float outslope)
		{
		
			OC3Ent::Face::FxReal time = (static_cast<OC3Ent::Face::FxReal>(frame) / static_cast<OC3Ent::Face::FxReal>(GetFrameRate()));

			int returnValue = OC3Ent::Face::FxMaxData::maxInterface.InsertKey(group.data(), anim.data(), curve.data(),
				time, static_cast<OC3Ent::Face::FxReal>(value), static_cast<OC3Ent::Face::FxReal>(inslope), static_cast<OC3Ent::Face::FxReal>(outslope));
			return returnValue;
		}
		void fxsetdisplaywarningdialogs( TSTR toggle )
		{
			OC3Ent::Face::FxBool displaywarningdialogs = FxFalse;
			if ( stricmp(toggle.data(), "on") == 0 )
			{
				displaywarningdialogs = FxTrue;
			}
			OC3Ent::Face::FxMaxData::maxInterface.SetShouldDisplayWarningDialogs(displaywarningdialogs);
		}
		void fxsetnormalizescale( TSTR toggle )
		{
			OC3Ent::Face::FxBool normalizescale = FxFalse;
			if ( stricmp(toggle.data(), "on") == 0 )
			{
				normalizescale = FxTrue;
			}
			OC3Ent::Face::FxMaxData::maxInterface.SetNormalizeScale(normalizescale);
			UpdateMainFaceFxPanel();
		}
		bool fxisnosave( )
		{
			OC3Ent::Face::FxBool answer = OC3Ent::Face::FxMaxData::maxInterface.IsNoSave();
			return answer;
		}
		bool fxismoddeveloper( )
		{
			OC3Ent::Face::FxBool answer = OC3Ent::Face::FxMaxData::maxInterface.IsModDeveloper();
			return answer;
		}
			
};

static FPInterfaceDesc FxMaxScript_mixininterface(

    FX_MAXSCRIPT_INTERFACE, _T("FaceFX_Maxscript"), 0, &fxMaxScriptInterfaceClassDesc, FP_MIXIN, 
	fn_fxloadactor,  _T("fxloadactor"), 0, TYPE_VOID, 0, 1, 
		_T("file"), NULL, TYPE_STRING,	
	fn_fxcreateactor, _T("fxcreateactor"), 0, TYPE_VOID, 0, 1, 
		_T("name"), NULL, TYPE_STRING,
	fn_fxsaveactor, _T("fxsaveactor"), 0, TYPE_VOID, 0, 1, 
		_T("file"), NULL, TYPE_STRING,
	fn_fxgetactorname, _T("fxgetactorname"), 0, TYPE_STRING, 0, 0,
	fn_fxexportrefpose, _T("fxexportrefpose"), 0, TYPE_VOID, 0, 2,
		_T("frame"), NULL, TYPE_INT,
		_T("bones"), NULL, TYPE_STRING_TAB,
	fn_fximportrefpose, _T("fximportrefpose"), 0, TYPE_VOID, 0, 1,
		_T("frame"), NULL, TYPE_INT,
	fn_fxexportbonepose, _T("fxexportbonepose"), 0, TYPE_VOID, 0, 2,
		_T("frame"), NULL, TYPE_INT,
		_T("pose"), NULL, TYPE_STRING,
	fn_fxbatchexportboneposes, _T("fxbatchexportboneposes"), 0, TYPE_VOID, 0, 1, 
		_T("batch file"), NULL, TYPE_STRING,
	fn_fximportbonepose, _T("fximportbonepose"), 0, TYPE_VOID, 0, 2,
		_T("frame"), NULL, TYPE_INT,
		_T("pose"), NULL, TYPE_STRING,
	fn_fxbatchimportboneposes, _T("fxbatchimportboneposes"), 0, TYPE_VOID, 0, 1, 
		_T("batch file"), NULL, TYPE_STRING,
	fn_fximportanim, _T("fximportanim"), 0, TYPE_VOID, 0, 2,
		_T("group"), NULL, TYPE_STRING,
		_T("anim"), NULL, TYPE_STRING,
	fn_fxclean, _T("fxclean"), 0, TYPE_VOID, 0, 0,
	fn_fxgetanimgroups, _T("fxgetanimgroups"), 0, TYPE_TSTR_TAB_BV, 0, 0,
	fn_fxgetanims, _T("fxgetanims"), 0, TYPE_TSTR_TAB_BV, 0, 1, 
		_T("group"), NULL, TYPE_STRING,
	fn_fxgetcurves, _T("fxgetcurves"), 0, TYPE_TSTR_TAB_BV, 0, 2,
		_T("group"), NULL, TYPE_STRING,
		_T("anim"), NULL, TYPE_STRING,
	fn_fxgetnodes, _T("fxgetnodes"), 0, TYPE_TSTR_TAB_BV, 0, 1, 
		_T("type"), NULL, TYPE_STRING,
	fn_fxgetrefbones, _T("fxgetrefbones"), 0, TYPE_TSTR_TAB_BV, 0, 0,
	fn_fxgetbones, _T("fxgetbones"), 0, TYPE_TSTR_TAB_BV, 0, 1, 
		_T("pose"), NULL, TYPE_STRING,
	fn_fxgetrawcurvekeyvalues, _T("fxgetrawcurvekeyvalues"), 0, TYPE_FLOAT_TAB, 0, 3,
		_T("group"), NULL, TYPE_STRING,
		_T("anim"), NULL, TYPE_STRING,
		_T("curve"), NULL, TYPE_STRING,
	fn_fxgetrawcurvekeytimes, _T("fxgetrawcurvekeytimes"), 0, TYPE_FLOAT_TAB, 0, 3,
		_T("group"), NULL, TYPE_STRING,
		_T("anim"), NULL, TYPE_STRING,
		_T("curve"), NULL, TYPE_STRING,
	fn_fxgetrawcurvekeyslopein, _T("fxgetrawcurvekeyslopein"), 0, TYPE_FLOAT_TAB, 0, 3,
		_T("group"), NULL, TYPE_STRING,
		_T("anim"), NULL, TYPE_STRING,
		_T("curve"), NULL, TYPE_STRING,
	fn_fxgetrawcurvekeyslopeout, _T("fxgetrawcurvekeyslopeout"), 0, TYPE_FLOAT_TAB, 0, 3,
		_T("group"), NULL, TYPE_STRING,
		_T("anim"), NULL, TYPE_STRING,
		_T("curve"), NULL, TYPE_STRING,
	fn_fxgetbakedcurvekeyvalues, _T("fxgetbakedcurvekeyvalues"), 0, TYPE_FLOAT_TAB, 0, 3,
		_T("group"), NULL, TYPE_STRING,
		_T("anim"), NULL, TYPE_STRING,
		_T("curve"), NULL, TYPE_STRING,
	fn_fxgetbakedcurvekeytimes, _T("fxgetbakedcurvekeytimes"), 0, TYPE_FLOAT_TAB, 0, 3,
		_T("group"), NULL, TYPE_STRING,
		_T("anim"), NULL, TYPE_STRING,
		_T("curve"), NULL, TYPE_STRING,
	fn_fxgetbakedcurvekeyslopein, _T("fxgetbakedcurvekeyslopein"), 0, TYPE_FLOAT_TAB, 0, 3,
		_T("group"), NULL, TYPE_STRING,
		_T("anim"), NULL, TYPE_STRING,
		_T("curve"), NULL, TYPE_STRING,
	fn_fxgetbakedcurvekeyslopeout, _T("fxgetbakedcurvekeyslopeout"), 0, TYPE_FLOAT_TAB, 0, 3,
		_T("group"), NULL, TYPE_STRING,
		_T("anim"), NULL, TYPE_STRING,
		_T("curve"), NULL, TYPE_STRING,
	fn_fxgetanimduration, _T("fxgetanimduration"), 0, TYPE_FLOAT, 0, 2,
		_T("group"), NULL, TYPE_STRING,
		_T("anim"), NULL, TYPE_STRING,
	fn_fxdeleteallkeys, _T("fxdeleteallkeys"), 0,  TYPE_VOID, 0, 3,
		_T("group"), NULL, TYPE_STRING,
		_T("anim"), NULL, TYPE_STRING,
		_T("curve"), NULL, TYPE_STRING,
	fn_fxinsertkey, _T("fxinsertkey"), 0, TYPE_INT, 0, 7,
		_T("group"), NULL, TYPE_STRING,
		_T("anim"), NULL, TYPE_STRING,
		_T("curve"), NULL, TYPE_STRING,
		_T("frame"), NULL, TYPE_INT,
		_T("value"), NULL, TYPE_FLOAT,
		_T("inslope"), NULL, TYPE_FLOAT,
		_T("outslope"), NULL, TYPE_FLOAT,
	fn_fxsetdisplaywarningdialogs, _T("fxsetdisplaywarningdialogs"), 0, TYPE_VOID, 0, 1, 
		_T("display"), NULL, TYPE_STRING,
	fn_fxsetnormalizescale, _T("fxsetnormalizescale"), 0, TYPE_VOID, 0, 1, 
		_T("normalize"), NULL, TYPE_STRING,
	fn_fxisnosave, _T("fxisnosave"), 0, TYPE_BOOL, 0, 0,
	fn_fxismoddeveloper, _T("fxismoddeveloper"), 0, TYPE_BOOL, 0, 0,
    end 

);


//------------------------------------------------------------------------------
// FxMaxScriptInterfaceClassDesc.
//------------------------------------------------------------------------------

// Controls if the plug-in shows up in lists from the user to choose from.
int FxMaxScriptInterfaceClassDesc::IsPublic( void ) 
{ 
	return TRUE; 
}

// Max calls this method when it needs a pointer to a new instance of the 
// plug-in class.
void* FxMaxScriptInterfaceClassDesc::Create( BOOL loading )
{ 
	return new FaceFX();
}

// Returns the name of the class.
const TCHAR* FxMaxScriptInterfaceClassDesc::ClassName( void ) 
{ 
	return GetString(IDS_CLASS_NAME); 
}

// Returns a system defined constant describing the class this plug-in 
// class was derived from.
SClass_ID FxMaxScriptInterfaceClassDesc::SuperClassID( void ) 
{ 
	return REF_TARGET_CLASS_ID; 
}

// Returns the unique ID for the object.
Class_ID FxMaxScriptInterfaceClassDesc::ClassID( void ) 
{
	return FACEFX_INTERFACE_CLASS_ID; 
}

// Returns a string describing the category the plug-in fits into.
const TCHAR* FxMaxScriptInterfaceClassDesc::Category( void ) 
{ 
	return _T("");
}

// Returns a string which provides a fixed, machine parsable internal name 
// for the plug-in.  This name is used by MAXScript.
const TCHAR* FxMaxScriptInterfaceClassDesc::InternalName( void ) 
{ 
	return _T("FaceFX"); 
}

// Returns the DLL instance handle of the plug-in.
HINSTANCE FxMaxScriptInterfaceClassDesc::HInstance( void ) 
{ 
	return hInstance;
}
