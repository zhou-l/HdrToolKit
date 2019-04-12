/// Code from
/// XAML SwapChainPanel DirectX interop sample (https://code.msdn.microsoft.com/windowsapps/XAML-SwapChainPanel-00cb688b)
/// with licence
// MICROSOFT LIMITED PUBLIC LICENSE version 1.1
// This license governs use of code marked as “sample” or “example” available on this web site without a license agreement, as provided under the section above titled “NOTICE SPECIFIC TO SOFTWARE AVAILABLE ON THIS WEB SITE.” If you use such code(the “software”), you accept this license.If you do not accept the license, do not use the software.
// 1. Definitions
// The terms “reproduce, ” “reproduction, ” “derivative works, ” and “distribution” have the same meaning here as under U.S.copyright law.
// A “contribution” is the original software, or any additions or changes to the software.
// A “contributor” is any person that distributes its contribution under this license.
// “Licensed patents” are a contributor’s patent claims that read directly on its contribution.
// 2. Grant of Rights
// (A) Copyright Grant - Subject to the terms of this license, including the license conditions and limitations in section 3, each contributor grants you a non - exclusive, worldwide, royalty - free copyright license to reproduce its contribution, prepare derivative works of its contribution, and distribute its contribution or any derivative works that you create.
// (B)Patent Grant - Subject to the terms of this license, including the license conditions and limitations in section 3, each contributor grants you a non - exclusive, worldwide, royalty - free license under its licensed patents to make, have made, use, sell, offer for sale, import, and / or otherwise dispose of its contribution in the software or derivative works of the contribution in the software.
// 3. Conditions and Limitations
// (A) No Trademark License - This license does not grant you rights to use any contributors’ name, logo, or trademarks.
// (B)If you bring a patent claim against any contributor over patents that you claim are infringed by the software, your patent license from such contributor to the software ends automatically.
// (C)If you distribute any portion of the software, you must retain all copyright, patent, trademark, and attribution notices that are present in the software.
// (D)If you distribute any portion of the software in source code form, you may do so only under this license by including a complete copy of this license with your distribution.If you distribute any portion of the software in compiled or object code form, you may only do so under a license that complies with this license.
// (E)The software is licensed “as - is.” You bear the risk of using it.The contributors give no express warranties, guarantees or conditions.You may have additional consumer rights under your local laws which this license cannot change.To the extent permitted under your local laws, the contributors exclude the implied warranties of merchantability, fitness for a particular purpose and non - infringement.
// (F)Platform Limitation - The licenses granted in sections 2(A) and 2(B)extend only to the software or derivative works that you create that run directly on a Microsoft Windows operating system product, Microsoft run - time technology(such as the.NET Framework or Silverlight), or Microsoft application platform(such as Microsoft Office or Microsoft Dynamics).

#pragma once
#include "pch.h"
#include <concrt.h>

namespace DirectXPanels
{
	// Base class for a SwapChainPanel-based DirectX rendering surface to be used in XAML apps.
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class DirectXPanelBase : public Windows::UI::Xaml::Controls::SwapChainPanel
	{
	protected private:
		DirectXPanelBase();

		virtual void CreateDeviceIndependentResources();
		virtual void CreateDeviceResources();
		virtual void CreateSizeDependentResources();

		virtual void OnDeviceLost();
		virtual void OnSizeChanged(Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ e);
		virtual void OnCompositionScaleChanged(Windows::UI::Xaml::Controls::SwapChainPanel ^sender, Platform::Object ^args);
		virtual void OnSuspending(Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ e);
		virtual void OnResuming(Platform::Object^ sender, Platform::Object^ args) { };

		virtual void Render() { };
		virtual void Present();

		Microsoft::WRL::ComPtr<ID3D11Device1>                               m_d3dDevice;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext1>                        m_d3dContext;
		Microsoft::WRL::ComPtr<IDXGISwapChain2>                             m_swapChain;

		Microsoft::WRL::ComPtr<ID2D1Factory2>                               m_d2dFactory;
		Microsoft::WRL::ComPtr<ID2D1Device>                                 m_d2dDevice;
		Microsoft::WRL::ComPtr<ID2D1DeviceContext>                          m_d2dContext;
		Microsoft::WRL::ComPtr<ID2D1Bitmap1>                                m_d2dTargetBitmap;

		D2D1_COLOR_F                                                        m_backgroundColor;
		DXGI_ALPHA_MODE                                                     m_alphaMode;

		bool                                                                m_loadingComplete;

		Concurrency::critical_section                                       m_criticalSection;

		float                                                               m_renderTargetHeight;
		float                                                               m_renderTargetWidth;

		float                                                               m_compositionScaleX;
		float                                                               m_compositionScaleY;

		float                                                               m_height;
		float                                                               m_width;

	};
}