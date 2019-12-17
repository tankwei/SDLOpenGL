#pragma once

#include "control.h"

#include "util/list.h"

namespace ui
{
	class Button;

	class ButtonObserver
	{
	public:

		util::Link button_link;

		virtual void on_button_click(Button *button) = 0;
	};

	class Button : public Control
	{
	public:

		typedef util::List<ButtonObserver, &ButtonObserver::button_link> ObserverList;

		ObserverList observers;

		//UI_DLL
		Button();
		//UI_DLL
		virtual ~Button();
	};
};
