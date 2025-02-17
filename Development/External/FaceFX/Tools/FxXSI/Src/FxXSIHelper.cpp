#include "FxXSIHelper.h"
#include <xsi_application.h>
#include <xsi_project.h>
#include <xsi_parameter.h>
#include <xsi_property.h>
#include <xsi_ref.h>

using namespace XSI;


void XSISetCurrentFrame(double in_dFrame)
{
	Application app;
	Project prj = app.GetActiveProject();

	// The PlayControl property set is stored with scene data under the project
	CRefArray proplist = prj.GetProperties();
	Property playctrl( proplist.GetItem(L"Play Control") );

	playctrl.PutParameterValue(L"Current",in_dFrame);
}

double XSIGetCurrentFrame()
{
	Application app;
	Project prj = app.GetActiveProject();

	// The PlayControl property set is stored with scene data under the project
	CRefArray proplist = prj.GetProperties();
	Property playctrl( proplist.GetItem(L"Play Control") );

	return playctrl.GetParameterValue(L"Current");
}

void XSISetSceneStartFrame(double in_dFrame)
{
	Application app;
	Project prj = app.GetActiveProject();

	// The PlayControl property set is stored with scene data under the project
	CRefArray proplist = prj.GetProperties();
	Property playctrl( proplist.GetItem(L"Play Control") );

	playctrl.PutParameterValue(L"GlobalIn", in_dFrame);
}

void XSISetSceneEndFrame(double in_dFrame)
{
	Application app;
	Project prj = app.GetActiveProject();

	// The PlayControl property set is stored with scene data under the project
	CRefArray proplist = prj.GetProperties();
	Property playctrl( proplist.GetItem(L"Play Control") );

	playctrl.PutParameterValue(L"GlobalOut", in_dFrame);
}
