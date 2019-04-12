//
// MainPage.xaml.h
// Deklaration der MainPage-Klasse
//

#pragma once

#include "MainPage.g.h"

namespace HdrToolkit
{
	/// <summary>
	/// Eine leere Seite, die eigenständig verwendet werden kann oder auf die innerhalb eines Rahmens navigiert werden kann.
	/// </summary>
	public ref class MainPage sealed
	{
	public:
		MainPage();

	protected:
		virtual void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;
	};
}
