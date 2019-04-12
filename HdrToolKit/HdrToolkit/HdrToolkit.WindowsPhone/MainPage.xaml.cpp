//
// MainPage.xaml.cpp
// Implementierung der MainPage-Klasse
//

#include "pch.h"
#include "MainPage.xaml.h"

using namespace HdrToolkit;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

// Die Elementvorlage "Leere Seite" ist unter http://go.microsoft.com/fwlink/?LinkId=234238 dokumentiert.

MainPage::MainPage()
{
	InitializeComponent();
}

/// <summary>
/// Wird aufgerufen, wenn diese Seite in einem Rahmen angezeigt werden soll.
/// </summary>
/// <param name="e">Ereignisdaten, die beschreiben, wie diese Seite erreicht wurde. Die
/// Parametereigenschaft wird normalerweise zum Konfigurieren der Seite verwendet.</param>
void MainPage::OnNavigatedTo(NavigationEventArgs^ e)
{
	(void) e;	// Nicht verwendeter Parameter

	// TODO: Seite vorbereiten, um sie hier anzuzeigen.

	// TODO: Wenn Ihre Anwendung mehrere Seiten enthält, stellen Sie sicher, dass
	// die Hardware-Zurück-Taste behandelt wird, indem Sie das
	// Windows::Phone::UI::Input::HardwareButtons.BackPressed-Ereignis.
	// Wenn Sie den NavigationHelper verwenden, der bei einigen Vorlagen zur Verfügung steht,
	// wird dieses Ereignis für Sie behandelt.
}
