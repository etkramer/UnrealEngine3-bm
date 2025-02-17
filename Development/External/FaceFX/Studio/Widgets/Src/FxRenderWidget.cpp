//------------------------------------------------------------------------------
// A render window widget.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"
#include "FxRenderWidget.h"
#include "FxStudioApp.h"
#include "FxUtilityFunctions.h"
#include "FxArchive.h"
#include "FxArchiveStoreFile.h"
#include "FxProgressDialog.h"

namespace OC3Ent
{

namespace Face
{

WX_IMPLEMENT_ABSTRACT_CLASS(FxRenderWidget, wxWindow)

FxRenderWidget::FxRenderWidget( wxWindow* parent, FxWidgetMediator* mediator )
	: Super(parent, -1, wxDefaultPosition, wxDefaultSize, wxCLIP_CHILDREN)
	, FxWidget(mediator)
	, _actor(NULL)
{
}

FxRenderWidget::~FxRenderWidget()
{
}

void FxRenderWidget::OnActorChanged( FxWidget* FxUnused(sender), FxActor* actor )
{
	_actor = actor;
}

void FxRenderWidget::
OnActorInternalDataChanged( FxWidget* FxUnused(sender), FxActorDataChangedFlag FxUnused(flags) )
{
	// Do nothing.
}

} // namespace Face

} // namespace OC3Ent
