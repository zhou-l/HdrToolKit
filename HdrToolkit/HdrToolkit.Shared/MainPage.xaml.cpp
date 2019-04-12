//
// MainPage.xaml.cpp
// Implementierung der MainPage-Klasse
//

#include "pch.h"
#include "MainPage.xaml.h"

#include "ColorMap.h"

#include <numeric>

using namespace HdrToolkit;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::UI::Xaml::Shapes;
using namespace concurrency;

// Die Elementvorlage "Leere Seite" ist unter http://go.microsoft.com/fwlink/?LinkId=234238 dokumentiert.

MainPage::MainPage()
{
	InitializeComponent();

	static Platform::Collections::Vector<Platform::Object^>^ displayOutputs = ref new Platform::Collections::Vector<Platform::Object^>
	{
		ref new DisplayOutputView(DisplayOutput::Output),
		ref new DisplayOutputView(DisplayOutput::Input),
		ref new DisplayOutputView(DisplayOutput::PostProcessed),
		ref new DisplayOutputView(DisplayOutput::TonemappedImage),
		ref new DisplayOutputView(DisplayOutput::BlobMask),
		ref new DisplayOutputView(DisplayOutput::Overlay),
		ref new DisplayOutputView(DisplayOutput::Aperture),
		ref new DisplayOutputView(DisplayOutput::Glare),
		ref new DisplayOutputView(DisplayOutput::GlareOverlay),
	};

	static Platform::Collections::Vector<ColorMap^>^ colorMaps = ref new Platform::Collections::Vector<ColorMap^>
	{
		ColorMap::GetColorMap(NamedColorMap::magma), // by default use magma
		 nullptr, // empty cell -> no color mapping
		ColorMap::GetColorMap(NamedColorMap::blkCustom),
		// Perceptually Uniform Sequential
		ColorMap::GetColorMap(NamedColorMap::viridis),
		ColorMap::GetColorMap(NamedColorMap::plasma),
		ColorMap::GetColorMap(NamedColorMap::inferno),
		//ColorMap::GetColorMap(NamedColorMap::magma),

		// Sequential
		// all bright low values
		ColorMap::GetColorMap(NamedColorMap::Greys),
		ColorMap::GetColorMap(NamedColorMap::Purples),
		ColorMap::GetColorMap(NamedColorMap::Blues),
		ColorMap::GetColorMap(NamedColorMap::Greens),
		ColorMap::GetColorMap(NamedColorMap::Oranges),
		ColorMap::GetColorMap(NamedColorMap::Reds),
		ColorMap::GetColorMap(NamedColorMap::YlOrBr),
		ColorMap::GetColorMap(NamedColorMap::YlOrRd),
		ColorMap::GetColorMap(NamedColorMap::OrRd),
		ColorMap::GetColorMap(NamedColorMap::PuRd),
		ColorMap::GetColorMap(NamedColorMap::RdPu),
		ColorMap::GetColorMap(NamedColorMap::BuPu),
		ColorMap::GetColorMap(NamedColorMap::GnBu),
		ColorMap::GetColorMap(NamedColorMap::PuBu),
		ColorMap::GetColorMap(NamedColorMap::YlGnBu),
		ColorMap::GetColorMap(NamedColorMap::PuBuGn),
		ColorMap::GetColorMap(NamedColorMap::BuGn),
		ColorMap::GetColorMap(NamedColorMap::YlGn),

		// Sequential (2)
		ColorMap::GetColorMap(NamedColorMap::binary), // bright low values
		ColorMap::GetColorMap(NamedColorMap::gist_yarg), // bright low values
		ColorMap::GetColorMap(NamedColorMap::gist_gray),
		ColorMap::GetColorMap(NamedColorMap::gray),
		ColorMap::GetColorMap(NamedColorMap::bone),
		ColorMap::GetColorMap(NamedColorMap::pink),
		ColorMap::GetColorMap(NamedColorMap::spring),
		ColorMap::GetColorMap(NamedColorMap::summer),
		ColorMap::GetColorMap(NamedColorMap::autumn),
		ColorMap::GetColorMap(NamedColorMap::winter),
		ColorMap::GetColorMap(NamedColorMap::cool),
		ColorMap::GetColorMap(NamedColorMap::Wistia),
		ColorMap::GetColorMap(NamedColorMap::hot),
		ColorMap::GetColorMap(NamedColorMap::afmhot),
		ColorMap::GetColorMap(NamedColorMap::gist_heat),
		ColorMap::GetColorMap(NamedColorMap::copper),

		// Diverging
		ColorMap::GetColorMap(NamedColorMap::PiYG),
		ColorMap::GetColorMap(NamedColorMap::PRGn),
		ColorMap::GetColorMap(NamedColorMap::BrBG),
		ColorMap::GetColorMap(NamedColorMap::PuOr),
		ColorMap::GetColorMap(NamedColorMap::RdGy),
		ColorMap::GetColorMap(NamedColorMap::RdBu),
		ColorMap::GetColorMap(NamedColorMap::RdYlBu),
		ColorMap::GetColorMap(NamedColorMap::RdYlGn),
		ColorMap::GetColorMap(NamedColorMap::Spectral),
		ColorMap::GetColorMap(NamedColorMap::coolwarm),
		ColorMap::GetColorMap(NamedColorMap::bwr),
		ColorMap::GetColorMap(NamedColorMap::seismic),

		// Miscellaneous
		ColorMap::GetColorMap(NamedColorMap::ocean),
		ColorMap::GetColorMap(NamedColorMap::gist_earth),
		ColorMap::GetColorMap(NamedColorMap::terrain),
		ColorMap::GetColorMap(NamedColorMap::gist_stern),
		ColorMap::GetColorMap(NamedColorMap::gnuplot),
		ColorMap::GetColorMap(NamedColorMap::gnuplot2),
		ColorMap::GetColorMap(NamedColorMap::CMRmap),
		ColorMap::GetColorMap(NamedColorMap::cubehelix),
		ColorMap::GetColorMap(NamedColorMap::brg),
		ColorMap::GetColorMap(NamedColorMap::hsv),
		ColorMap::GetColorMap(NamedColorMap::gist_rainbow),
		ColorMap::GetColorMap(NamedColorMap::rainbow),
		ColorMap::GetColorMap(NamedColorMap::jet),
		ColorMap::GetColorMap(NamedColorMap::nipy_spectral),
		ColorMap::GetColorMap(NamedColorMap::gist_ncar),
	};

	DisplayOutput->ItemsSource = displayOutputs;
	DisplayOutput->SelectedIndex = 0;

	Maps->ItemsSource = colorMaps;
	Maps->SelectedIndex = 0;

	m_isListenPointer = false;
}

void MainPage::OnNavigatedTo(NavigationEventArgs^ e)
{
	m_dispatcher = CoreWindow::GetForCurrentThread()->Dispatcher;
	m_loader = ref new Windows::ApplicationModel::Resources::ResourceLoader();
	this->Focus(Windows::UI::Xaml::FocusState::Programmatic);
	SetDefaults();
	//MainPanel->CacheMode = ref new BitmapCache();

	if (auto args = dynamic_cast<Windows::ApplicationModel::Activation::FileActivatedEventArgs^>(e->Parameter))
	{
		auto file = args->Files->First()->Current;

		create_task(MainPanel->InitAsync()).then([this, file]()
		{
			return create_task(MainPanel->TryOpenImage(file));
		}).then([this](bool loadingSucceded)
		{
			if (loadingSucceded)
			{
				Update();
			}
			else
			{
				OpenFile->IsEnabled = true;
			}
		});
	}
	else
	{
		create_task(MainPanel->InitAsync()).then([this]()
		{
			OpenFile->IsEnabled = true;
		});
	}
}

void MainPage::OnNavigatedFrom(NavigationEventArgs^ e)
{
}

void MainPage::SetDefaults()
{
	ContrastFactor->Value = 0.4f;

	UndoGammaCorrection->IsOn = false;
	InvertImage->IsOn = false;
	ShowSteps->IsChecked = true;
	GlareIntensity->Value = 1.0;

	BlobThresold->Value = 0.12;
	LightThresold->Value = 0.60;

	DisableUpdate->IsChecked = false;

	MaxWavelength->Value = 770.0;
	MinWavelength->Value = 380.0;
	InvertColormap->IsOn = false;

	LensRadius->Value = 25;
}

void MainPage::PickOpen_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	OpenFile->IsEnabled = false;

	create_task(MainPanel->TryPickOpenImage()).then([this](bool loadingSucceded)
	{
		if (loadingSucceded)
		{
			Update(true);
		}
		else
		{
			OpenFile->IsEnabled = true;
		}
	});
}

void MainPage::PickSave_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	MainPanel->TryPickSaveImage();
}

void MainPage::Slider_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
	Update();
}

void MainPage::Wavelength_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
	if (MinWavelength == nullptr || MaxWavelength == nullptr || Wavelength == nullptr)
	{
		return;
	}

	if (MaxWavelength->Value < MinWavelength->Value)
	{
		MaxWavelength->Value = MinWavelength->Value;
		return;
	}

	auto stops = ref new GradientStopCollection();

	int const n = 32; // number of wavelengths sampled
	for (int i = 0; i < n + 1; i++)
	{
		auto stop = ref new GradientStop();
		stop->Offset = static_cast<double>(i) / n;
		double wavelength = stop->Offset * (MaxWavelength->Value - MinWavelength->Value) + MinWavelength->Value;
		stop->Color = ConvertToColor(WavelengthToRgb((float)wavelength));

		stops->Append(stop);
	}

	auto brush = ref new LinearGradientBrush(stops, 0.0);
	Wavelength->Fill = brush;

	Update();
}

void MainPage::LensRadiusSlider_ValueChanged(Platform::Object ^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs ^ e)
{
	if (MainPanel)
	{
		MainPanel->SetLensRadius(LensRadius->Value);
	}
}

void MainPage::Selection_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
	Update();
}

void MainPage::Maps_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
	if (Maps == nullptr || MappedRectangle == nullptr)
		return;

	if (auto colormap = safe_cast<ColorMap^>(Maps->SelectedItem))
	{
		MappedRectangle->Fill = colormap->GetFill(InvertColormap->IsOn);
	}
	else
	{
		MappedRectangle->Fill = ref new SolidColorBrush(Windows::UI::Colors::Black);
	}

	Update();
}

void MainPage::InvertColormapButton_Toggled(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	Maps_SelectionChanged(this, nullptr);
}

void MainPage::ImageDetails_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	ImageDetails->DataContext = MainPanel->GetImageDetails();
}

void HdrToolkit::MainPage::PlayAnimation_Click(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e)
{
	if (MainPanel)
	{
		ParameterView_Mantiuk06^ mantiuk06 = ref new ParameterView_Mantiuk06();
		mantiuk06->ContrastFactor = static_cast<float>(ContrastFactor->Value);
		mantiuk06->EqualizeContrast = false;

		ParameterView_Reinhard02^ reinhard02 = ref new ParameterView_Reinhard02();

		ParameterView_Output^ output = ref new ParameterView_Output();
		output->UndoDefaultGammaCorrection = UndoGammaCorrection->IsOn;
		output->InvertImage = InvertImage->IsOn;
		output->InvertColormap = InvertColormap->IsOn;
		output->OutputTexture = safe_cast<DisplayOutputView^>(DisplayOutput->SelectedItem)->AsInt();

		output->ShowSteps = ShowSteps->IsChecked->Value;
		output->GlareIntensity = static_cast<float>(GlareIntensity->Value);
		output->Map = safe_cast<ColorMap^>(Maps->SelectedItem);

		ParameterView_Glare^ glare = ref new ParameterView_Glare();
		glare->MinWavelength = static_cast<float>(MinWavelength->Value);
		glare->MaxWavelength = static_cast<float>(MaxWavelength->Value);

		ParameterView_Blob^ blob = ref new ParameterView_Blob();
		blob->BlobThreshold = static_cast<float>(BlobThresold->Value);
		blob->LightThreshold = static_cast<float>(LightThresold->Value);

		ParametersView^ parameters = ref new ParametersView();
		parameters->Output = output;
		parameters->Glare = glare;
		parameters->Blob = blob;
		parameters->Mantiuk06 = mantiuk06;
		parameters->Reinhard02 = reinhard02;
		// Highlight mode
		auto operation = MainPanel->PlayAnimationHL(parameters);
		//operation->Progress = ref new AsyncOperationProgressHandler<bool, UpdateProgress>([this](IAsyncOperationWithProgress<bool, UpdateProgress>^ action, UpdateProgress progress)
		//{
		//	m_dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this, progress]()
		//	{
		//		std::vector<double> progresses = { progress.Loading, progress.Tonemapping, progress.FindingBlobs, progress.Glaring, progress.Displaying };

		//		Progress->Value = std::accumulate(progresses.begin(), progresses.end(), 0.0) / progresses.size();

		//		Progress_Status->Text = m_loader->GetString("StatusString") + ": " + m_loader->GetString(progress.Status);

		//		Progress_Loading->Value = progress.Loading;
		//		Progress_Tonemapping->Value = progress.Tonemapping;
		//		Progress_FindingBlobs->Value = progress.FindingBlobs;
		//		Progress_Glaring->Value = progress.Glaring;
		//		Progress_Displaying->Value = progress.Displaying;
		//	}));
		//});

		create_task(operation).then([this](bool renderSucceded)
		{
			if (renderSucceded)
			{
				EnableAllControls(true);
				EnableNewControls(true);
			}
			else
			{
				OpenFile->IsEnabled = true;
				ShowSteps->IsEnabled = true;
				DisableUpdate->IsEnabled = true;
				EnableNewControls(false);
			}
			UpdateButton->IsEnabled = DisableUpdate->IsChecked->Value;
		});
	}
}

void HdrToolkit::MainPage::PlayAnimationFlicker_Click(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e)
{

	if (MainPanel)
	{
		ParameterView_Mantiuk06^ mantiuk06 = ref new ParameterView_Mantiuk06();
		mantiuk06->ContrastFactor = static_cast<float>(ContrastFactor->Value);
		mantiuk06->EqualizeContrast = false;

		ParameterView_Reinhard02^ reinhard02 = ref new ParameterView_Reinhard02();

		ParameterView_Output^ output = ref new ParameterView_Output();
		output->UndoDefaultGammaCorrection = UndoGammaCorrection->IsOn;
		output->InvertImage = InvertImage->IsOn;
		output->InvertColormap = InvertColormap->IsOn;
		output->OutputTexture = safe_cast<DisplayOutputView^>(DisplayOutput->SelectedItem)->AsInt();

		output->ShowSteps = ShowSteps->IsChecked->Value;
		output->GlareIntensity = static_cast<float>(GlareIntensity->Value);
		output->Map = safe_cast<ColorMap^>(Maps->SelectedItem);

		ParameterView_Glare^ glare = ref new ParameterView_Glare();
		glare->MinWavelength = static_cast<float>(MinWavelength->Value);
		glare->MaxWavelength = static_cast<float>(MaxWavelength->Value);

		ParameterView_Blob^ blob = ref new ParameterView_Blob();
		blob->BlobThreshold = static_cast<float>(BlobThresold->Value);
		blob->LightThreshold = static_cast<float>(LightThresold->Value);

		ParametersView^ parameters = ref new ParametersView();
		parameters->Output = output;
		parameters->Glare = glare;
		parameters->Blob = blob;
		parameters->Mantiuk06 = mantiuk06;
		parameters->Reinhard02 = reinhard02;
		 // Flicker mode
		auto operation = MainPanel->PlayAnimationFL(parameters);
	/*	operation->Progress = ref new AsyncOperationProgressHandler<bool, UpdateProgress>([this](IAsyncOperationWithProgress<bool, UpdateProgress>^ action, UpdateProgress progress)
		{
			m_dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this, progress]()
			{
				std::vector<double> progresses = { progress.Loading, progress.Tonemapping, progress.FindingBlobs, progress.Glaring, progress.Displaying };

				Progress->Value = std::accumulate(progresses.begin(), progresses.end(), 0.0) / progresses.size();

				Progress_Status->Text = m_loader->GetString("StatusString") + ": " + m_loader->GetString(progress.Status);

				Progress_Loading->Value = progress.Loading;
				Progress_Tonemapping->Value = progress.Tonemapping;
				Progress_FindingBlobs->Value = progress.FindingBlobs;
				Progress_Glaring->Value = progress.Glaring;
				Progress_Displaying->Value = progress.Displaying;
			}));
		});*/

		create_task(operation).then([this](bool renderSucceded)
		{
			if (renderSucceded)
			{
				EnableAllControls(true);
				EnableNewControls(true);
			}
			else
			{
				OpenFile->IsEnabled = true;
				ShowSteps->IsEnabled = true;
				DisableUpdate->IsEnabled = true;
				EnableNewControls(false);
			}
			UpdateButton->IsEnabled = DisableUpdate->IsChecked->Value;
		});
	}
}

void MainPage::SwitchGlareImage(bool showGlare)
{
	ParameterView_Mantiuk06^ mantiuk06 = ref new ParameterView_Mantiuk06();
	mantiuk06->ContrastFactor = static_cast<float>(ContrastFactor->Value);
	mantiuk06->EqualizeContrast = false;

	ParameterView_Reinhard02^ reinhard02 = ref new ParameterView_Reinhard02();

	ParameterView_Output^ output = ref new ParameterView_Output();
	output->UndoDefaultGammaCorrection = UndoGammaCorrection->IsOn;
	output->InvertImage = InvertImage->IsOn;
	output->InvertColormap = InvertColormap->IsOn;
	output->OutputTexture = safe_cast<DisplayOutputView^>(DisplayOutput->SelectedItem)->AsInt();

	output->ShowSteps = ShowSteps->IsChecked->Value;
	output->GlareIntensity = static_cast<float>(GlareIntensity->Value);
	output->Map = safe_cast<ColorMap^>(Maps->SelectedItem);

	ParameterView_Glare^ glare = ref new ParameterView_Glare();
	glare->MinWavelength = static_cast<float>(MinWavelength->Value);
	glare->MaxWavelength = static_cast<float>(MaxWavelength->Value);

	ParameterView_Blob^ blob = ref new ParameterView_Blob();
	blob->BlobThreshold = static_cast<float>(BlobThresold->Value);
	blob->LightThreshold = static_cast<float>(LightThresold->Value);

	ParametersView^ parameters = ref new ParametersView();
	parameters->Output = output;
	parameters->Glare = glare;
	parameters->Blob = blob;
	parameters->Mantiuk06 = mantiuk06;
	parameters->Reinhard02 = reinhard02;
	// Flicker mode
	auto operation = showGlare ? MainPanel->DispWithGlares(parameters) : MainPanel->DispNoGlares(parameters);
	/*operation->Progress = ref new AsyncOperationProgressHandler<bool, UpdateProgress>([this](IAsyncOperationWithProgress<bool, UpdateProgress>^ action, UpdateProgress progress)
	{
		m_dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this, progress]()
		{
			std::vector<double> progresses = { progress.Loading, progress.Tonemapping, progress.FindingBlobs, progress.Glaring, progress.Displaying };

			Progress->Value = std::accumulate(progresses.begin(), progresses.end(), 0.0) / progresses.size();

			Progress_Status->Text = m_loader->GetString("StatusString") + ": " + m_loader->GetString(progress.Status);

			Progress_Loading->Value = progress.Loading;
			Progress_Tonemapping->Value = progress.Tonemapping;
			Progress_FindingBlobs->Value = progress.FindingBlobs;
			Progress_Glaring->Value = progress.Glaring;
			Progress_Displaying->Value = progress.Displaying;
		}));
	});*/

	create_task(operation).then([this](bool renderSucceded)
	{
		if (renderSucceded)
		{
			EnableAllControls(true);
			EnableNewControls(true);
		}
		else
		{
			OpenFile->IsEnabled = true;
			ShowSteps->IsEnabled = true;
			DisableUpdate->IsEnabled = true;
			EnableNewControls(false);
		}
		UpdateButton->IsEnabled = DisableUpdate->IsChecked->Value;
	});
}

void HdrToolkit::MainPage::GlareToggle_Toggled(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e)
{

	if (MainPanel)
	{
		bool showGlare = GlareToggle->IsOn;
		SwitchGlareImage(showGlare);
	}
}

void HdrToolkit::MainPage::PlayFlickerToggle_Toggled(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e)
{
	if (MainPanel)
	{
		bool playFlicker = PlayFlickerToggle->IsOn;
		MainPanel->SetPlayAnim(playFlicker);
		if (playFlicker)
			PlayAnimationFlicker_Click(sender, e);
		else
			SwitchGlareImage(true); // Always show glare image
	}
}



void MainPage::CreateInfoPop(Windows::UI::Input::PointerPoint^ ptrPt)
{

	
	TextBlock^ pointerDetails = ref new TextBlock();
	pointerDetails->Name = ptrPt->PointerId.ToString();
	pointerDetails->Foreground = ref new SolidColorBrush(Windows::UI::Colors::White);


	TranslateTransform^ x = ref new TranslateTransform();

	x->X = ptrPt->Position.X + 20;
	x->Y = ptrPt->Position.Y + 20;
	pointerDetails->RenderTransform = x;

	float CanvasW = DisplayPanel->ViewportWidth / DisplayPanel->ZoomFactor;
	float CanvasH = DisplayPanel->ViewportHeight / DisplayPanel->ZoomFactor;

	Point globalPt;
	globalPt.X = DisplayPanel->HorizontalOffset + ptrPt->Position.X / (/*DisplayPanel->ViewportWidth * */DisplayPanel->ZoomFactor);
	globalPt.Y = DisplayPanel->VerticalOffset + ptrPt->Position.Y / (/*DisplayPanel->ViewportHeight * */DisplayPanel->ZoomFactor);

	wchar_t info[512];
	swprintf_s(info, 512, L"%f, %f, z=%f,vo=%f,ho=%f,gx=%f,gy=%f", ptrPt->Position.X, ptrPt->Position.Y,
		DisplayPanel->ZoomFactor,
		DisplayPanel->VerticalOffset,
		DisplayPanel->HorizontalOffset,
		globalPt.X,
		globalPt.Y
	);


	pointerDetails->Text = ref new String(info);//L"Pressed";

	if (MainPanel)
	{
		ImageDetailsView^ imgInfo = MainPanel->GetImageDetails();
		if (imgInfo)
		{
			float imageW = float(imgInfo->Width) ;
			float imageH = float(imgInfo->Height);

			Point normImgPt;
			normImgPt.X = ptrPt->Position.X / imageW;
			normImgPt.Y = ptrPt->Position.Y / imageH;

			wchar_t wcNormImgPt[512];
			swprintf_s(wcNormImgPt, 512, L"\n nx=%f, ny=%f", normImgPt.X, normImgPt.Y);
			String^ strNormImgPt = ref new String(wcNormImgPt);
			pointerDetails->Text += strNormImgPt;

		}
		MainPanel->Children->Append(pointerDetails);

	}
}

void MainPage::MainPanel_PointerMoved(Platform::Object ^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^ e)
{
	e->Handled = true;
	//CreateInfoPop(e->GetCurrentPoint(MainPanel));



	// Compute coordinates in the image
	Windows::UI::Input::PointerPoint^ ptrPt = e->GetCurrentPoint(MainPanel);
	float nImgX = -1.0f;
	float nImgY = -1.0f;

	if (MainPanel)
	{
		
		ImageDetailsView^ imgInfo = MainPanel->GetImageDetails();
		if (imgInfo)
		{
			float imageW = float(imgInfo->Width);
			float imageH = float(imgInfo->Height);


			nImgX = ptrPt->Position.X / imageW;
			nImgY = ptrPt->Position.Y / imageH;

		}
		//MainPanel->Children->Clear();
		////Draw a ellipse centered at the pointer
		//Path^ path1 = ref new Path();
		////path1->Fill = ref new Windows::UI::Xaml::Media::SolidColorBrush(Windows::UI::Colors::Azure);
		//path1->Stroke = ref new Windows::UI::Xaml::Media::SolidColorBrush(Windows::UI::Colors::Black);
		//path1->StrokeThickness = 2;

		//GeometryGroup^ geometryGroup1 = ref new GeometryGroup();
		//EllipseGeometry^ ellipseGeometry1 = ref new EllipseGeometry();
		//ellipseGeometry1->Center = Point(ptrPt->Position.X, ptrPt->Position.Y);
		//ellipseGeometry1->RadiusX = 25;
		//ellipseGeometry1->RadiusY = 25;
		//geometryGroup1->Children->Append(ellipseGeometry1);

		//path1->Data = geometryGroup1;
		//MainPanel->Children->Append(path1);

	}

	if (MainPanel)
	{
		if (nImgX >= 0.0f && nImgX < 1.0f && nImgY >= 0.0f && nImgY < 1.0f && m_isListenPointer)
			MainPanel->OnPointerInRange(sender, e);

	}
}

void HdrToolkit::MainPage::MainPanel_PointerPressed(Platform::Object ^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^ e)
{
	e->Handled = true;
	//CreateInfoPop(e->GetCurrentPoint(MainPanel));

	// Compute coordinates in the image
	Windows::UI::Input::PointerPoint^ ptrPt = e->GetCurrentPoint(MainPanel);
	float nImgX = -1.0f;
	float nImgY = -1.0f;

	if (MainPanel)
	{
		ImageDetailsView^ imgInfo = MainPanel->GetImageDetails();
		if (imgInfo)
		{
			float imageW = float(imgInfo->Width);
			float imageH = float(imgInfo->Height);

			
			nImgX = ptrPt->Position.X / imageW;
			nImgY = ptrPt->Position.Y / imageH;

		}
	}

	if (MainPanel)
	{
		if(nImgX >= 0.0f && nImgX < 1.0f && nImgY >= 0.0f && nImgY < 1.0f)
			MainPanel->OnPointerInRange(sender, e);
	}
}

bool IsKeyShowGlare = false;

void MainPage::MainPanel_KeyDown(Platform::Object ^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs ^ e)
{
	switch (e->Key)
	{
	case Windows::System::VirtualKey::S:
		if (MainPanel && GlareToggle->IsEnabled) {
			IsKeyShowGlare = !IsKeyShowGlare;
			GlareToggle->IsOn = IsKeyShowGlare;
			SwitchGlareImage(IsKeyShowGlare);
		}
		break;
	default:
		break;
	}
}

void MainPage::MainPanel_KeyUp(Platform::Object ^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs ^ e)
{

}



void MainPage::Button_Toggled(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	Update();
}

void MainPage::Update(bool forceUpdate)
{
	if (MainPanel && (forceUpdate || !DisableUpdate->IsChecked->Value))
	{
		ParameterView_Mantiuk06^ mantiuk06 = ref new ParameterView_Mantiuk06();
		mantiuk06->ContrastFactor = static_cast<float>(ContrastFactor->Value);
		mantiuk06->EqualizeContrast = false;

		ParameterView_Reinhard02^ reinhard02 = ref new ParameterView_Reinhard02();

		ParameterView_Output^ output = ref new ParameterView_Output();
		output->UndoDefaultGammaCorrection = UndoGammaCorrection->IsOn;
		output->InvertImage = InvertImage->IsOn;
		output->InvertColormap = InvertColormap->IsOn;
		output->OutputTexture = safe_cast<DisplayOutputView^>(DisplayOutput->SelectedItem)->AsInt();

		output->ShowSteps = ShowSteps->IsChecked->Value;
		output->GlareIntensity = static_cast<float>(GlareIntensity->Value);
		output->Map = safe_cast<ColorMap^>(Maps->SelectedItem);

		ParameterView_Glare^ glare = ref new ParameterView_Glare();
		glare->MinWavelength = static_cast<float>(MinWavelength->Value);
		glare->MaxWavelength = static_cast<float>(MaxWavelength->Value);

		ParameterView_Blob^ blob = ref new ParameterView_Blob();
		blob->BlobThreshold = static_cast<float>(BlobThresold->Value);
		blob->LightThreshold = static_cast<float>(LightThresold->Value);

		ParametersView^ parameters = ref new ParametersView();
		parameters->Output = output;
		parameters->Glare = glare;
		parameters->Blob = blob;
		parameters->Mantiuk06 = mantiuk06;
		parameters->Reinhard02 = reinhard02;

		//EnableAllControls(false);
		EnableAllControls(true);

		auto operation = MainPanel->Update(parameters);
		operation->Progress = ref new AsyncOperationProgressHandler<bool, UpdateProgress>([this](IAsyncOperationWithProgress<bool, UpdateProgress>^ action, UpdateProgress progress)
		{
			m_dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this, progress]()
			{
				std::vector<double> progresses = { progress.Loading, progress.Tonemapping, progress.FindingBlobs, progress.Glaring, progress.Displaying };

				Progress->Value = std::accumulate(progresses.begin(), progresses.end(), 0.0) / progresses.size();

				Progress_Status->Text = m_loader->GetString("StatusString") + ": " + m_loader->GetString(progress.Status);

				Progress_Loading->Value = progress.Loading;
				Progress_Tonemapping->Value = progress.Tonemapping;
				Progress_FindingBlobs->Value = progress.FindingBlobs;
				Progress_Glaring->Value = progress.Glaring;
				Progress_Displaying->Value = progress.Displaying;
			}));
		});

		create_task(operation).then([this](bool renderSucceded)
		{
			if (renderSucceded)
			{
				EnableAllControls(true);
				//PlayAnim->IsEnabled = true;
				//PlayAnimFlicker->IsEnabled = true;
				//m_isListenPointer = true;
				//GlareToggle->IsEnabled = true;
				//PlayFlickerToggle->IsEnabled = true;

				EnableNewControls(true);
			}
			else
			{
				OpenFile->IsEnabled = true;
				ShowSteps->IsEnabled = true;
				DisableUpdate->IsEnabled = true;
				EnableNewControls(false);
			}
			UpdateButton->IsEnabled = DisableUpdate->IsChecked->Value;
		});
	}
}

void MainPage::EnableAllControls(bool enable)
{
	ContrastFactor->IsEnabled = enable;
	InvertImage->IsEnabled = enable;

	UndoGammaCorrection->IsEnabled = enable;
	ShowSteps->IsEnabled = enable;
	GlareIntensity->IsEnabled = enable;

	BlobThresold->IsEnabled = enable;
	LightThresold->IsEnabled = enable;

	DisplayOutput->IsEnabled = enable;

	OpenFile->IsEnabled = enable;
	SaveFile->IsEnabled = enable;

	ImageDetails->IsEnabled = enable;

	DisableUpdate->IsEnabled = enable;
	UpdateButton->IsEnabled = enable;

	MinWavelength->IsEnabled = enable;
	MaxWavelength->IsEnabled = enable;

	InvertColormap->IsEnabled = enable;

	Maps->IsEnabled = enable;
}


void MainPage::EnableNewControls(bool enable)
{
	PlayAnim->IsEnabled = enable;
	PlayAnimFlicker->IsEnabled = enable;
	m_isListenPointer = enable;
	GlareToggle->IsEnabled = enable;
	PlayFlickerToggle->IsEnabled = enable;
	LensRadius->IsEnabled = enable;
}


void MainPage::DisableUpdate_Toggled(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	UpdateButton->IsEnabled = DisableUpdate->IsChecked->Value;
	Update();
}

void MainPage::Update_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	Update(true);
}

void MainPage::ShowLog_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	ShowLog->DataContext = MainPanel->GetLog();
}