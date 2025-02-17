/*=============================================================================
	DlgNewPhysXDestructible.h: Destructible Vertical Component.
	Copyright 2007-2008 AGEIA Technologies.
=============================================================================*/

#ifndef __DLGNEWPHYSXDESTRUCTIBLE_H__
#define __DLGNEWPHYSXDESTRUCTIBLE_H__

/*-----------------------------------------------------------------------------
WxDlgNewPhysXDestructible.
-----------------------------------------------------------------------------*/
class WxDlgNewPhysXDestructible : public wxDialog
{
public:
	WxDlgNewPhysXDestructible();
	~WxDlgNewPhysXDestructible();

	UFracturedStaticMesh		*FracturedStaticMesh;
	UPackage					*Package;
	FString						FracturedMeshName;
	WxPropertyWindow			*PropertyWindow;
	class UPhysXFractureOptions	*PhysXFractureOptions;
	wxBoxSizer					*PropertyWindowSizer;

	UBOOL Show( UFracturedStaticMesh* InFracturedStaticMesh, UPackage* InPackage, FString InFracturedMeshName);
	void OnOK( wxCommandEvent& In );
	void OnCancel( wxCommandEvent& In );
	void OnClose( wxCloseEvent& In );

	DECLARE_EVENT_TABLE()
};

#endif // __DLGNEWPHYSXDESTRUCTIBLE_H__
