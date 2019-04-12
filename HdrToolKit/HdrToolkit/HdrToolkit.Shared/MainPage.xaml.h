//
// MainPage.xaml.h
// Deklaration der MainPage-Klasse
//

#pragma once

#include "MainPage.g.h"

#include "HdrPanel.h"
#include "ColorConverter.h"

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
		virtual void OnNavigatedFrom(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;
	private:
		Windows::UI::Core::CoreDispatcher^ m_dispatcher;
		Windows::ApplicationModel::Resources::ResourceLoader^ m_loader;
		bool m_isListenPointer; // shall we listen pointer

		void SetDefaults();

		void PickOpen_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void PickSave_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

		void Slider_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
		void Wavelength_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
		void LensRadiusSlider_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);


		void Selection_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
		void Maps_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);

		void ImageDetails_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

		// Extra features for animations
		void PlayAnimation_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void PlayAnimationFlicker_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void GlareToggle_Toggled(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void PlayFlickerToggle_Toggled(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

		void MainPanel_PointerMoved(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e);
		void MainPanel_PointerPressed(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e);
		void MainPanel_KeyDown(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e);
		void MainPanel_KeyUp(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e);

		// Switch glare layer
		void SwitchGlareImage(bool showGlare);
		void CreateInfoPop(Windows::UI::Input::PointerPoint^ ptrPtr);

		void Button_Toggled(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

		void Update(bool forceUpdate = false);
		void EnableAllControls(bool enable);
		// Zhoul's controls
		void EnableNewControls(bool enable);
		
		void DisableUpdate_Toggled(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void Update_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void ShowLog_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void InvertColormapButton_Toggled(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e);
	};
}
