#include "Stdafx.h"
#include "OutputEventArgs.h"

namespace ConsoleInterface
{
	String^ OutputEventArgs::Message::get()
	{
		return mMessage;
	}

	Color OutputEventArgs::TextColor::get()
	{
		return mTxtColor;
	}

	OutputEventArgs::OutputEventArgs(Color TxtColor, String ^Msg)
	{
		if(Msg == nullptr)
		{
			throw gcnew ArgumentNullException(L"Msg");
		}

		mMessage = Msg;
		mTxtColor = TxtColor;
	}
}