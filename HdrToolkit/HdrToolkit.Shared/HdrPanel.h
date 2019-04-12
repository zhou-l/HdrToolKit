/// Class based on
/// XAML SwapChainPanel DirectX interop sample (https://code.msdn.microsoft.com/windowsapps/XAML-SwapChainPanel-00cb688b)
/// with licence
// MICROSOFT LIMITED PUBLIC LICENSE version 1.1
// This license governs use of code marked as “sample?or “example?available on this web site without a license agreement, as provided under the section above titled “NOTICE SPECIFIC TO SOFTWARE AVAILABLE ON THIS WEB SITE.?If you use such code(the “software?, you accept this license.If you do not accept the license, do not use the software.
// 1. Definitions
// The terms “reproduce, ?“reproduction, ?“derivative works, ?and “distribution?have the same meaning here as under U.S.copyright law.
// A “contribution?is the original software, or any additions or changes to the software.
// A “contributor?is any person that distributes its contribution under this license.
// “Licensed patents?are a contributor’s patent claims that read directly on its contribution.
// 2. Grant of Rights
// (A) Copyright Grant - Subject to the terms of this license, including the license conditions and limitations in section 3, each contributor grants you a non - exclusive, worldwide, royalty - free copyright license to reproduce its contribution, prepare derivative works of its contribution, and distribute its contribution or any derivative works that you create.
// (B)Patent Grant - Subject to the terms of this license, including the license conditions and limitations in section 3, each contributor grants you a non - exclusive, worldwide, royalty - free license under its licensed patents to make, have made, use, sell, offer for sale, import, and / or otherwise dispose of its contribution in the software or derivative works of the contribution in the software.
// 3. Conditions and Limitations
// (A) No Trademark License - This license does not grant you rights to use any contributors?name, logo, or trademarks.
// (B)If you bring a patent claim against any contributor over patents that you claim are infringed by the software, your patent license from such contributor to the software ends automatically.
// (C)If you distribute any portion of the software, you must retain all copyright, patent, trademark, and attribution notices that are present in the software.
// (D)If you distribute any portion of the software in source code form, you may do so only under this license by including a complete copy of this license with your distribution.If you distribute any portion of the software in compiled or object code form, you may only do so under a license that complies with this license.
// (E)The software is licensed “as - is.?You bear the risk of using it.The contributors give no express warranties, guarantees or conditions.You may have additional consumer rights under your local laws which this license cannot change.To the extent permitted under your local laws, the contributors exclude the implied warranties of merchantability, fitness for a particular purpose and non - infringement.
// (F)Platform Limitation - The licenses granted in sections 2(A) and 2(B)extend only to the software or derivative works that you create that run directly on a Microsoft Windows operating system product, Microsoft run - time technology(such as the.NET Framework or Silverlight), or Microsoft application platform(such as Microsoft Office or Microsoft Dynamics).


#pragma once
#include "pch.h"

#include <chrono>
#include <sstream>

#include "DirectXPanelBase.h"
#include "StepTimer.h"

#include "RenderTexture.h"
#include "RenderToolkit.h"

#include "ImageDetails.h"

#include "Parameters.h"
#include "DisplayParameters.h"

#include "fft.h"

#define MAX_LEVELS 16
#define MAX_GLARES 8
namespace HdrToolkit
{
	public value struct UpdateProgress sealed
	{
	public:
		double Loading;
		double Tonemapping;
		double FindingBlobs;
		double Glaring;
		double Displaying;
		Platform::String^ Status;
	};

	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class HdrPanel sealed : public DirectXPanels::DirectXPanelBase
	{
	public:
		HdrPanel();

		Windows::Foundation::IAsyncAction^ InitAsync();

		Windows::Foundation::IAsyncOperation<bool>^ TryPickOpenImage();
		Windows::Foundation::IAsyncOperation<bool>^ TryOpenImage(Windows::Storage::IStorageItem^ item);

		void TryPickSaveImage();

		Windows::Foundation::IAsyncOperationWithProgress<bool, UpdateProgress>^ Update(ParametersView^ parameters);
		Windows::Foundation::IAsyncOperationWithProgress<bool, UpdateProgress>^ DispNoGlares(ParametersView^ parameters);
		Windows::Foundation::IAsyncOperationWithProgress<bool, UpdateProgress>^ DispWithGlares(ParametersView^ parameters);

		Windows::Foundation::IAsyncOperationWithProgress<bool, UpdateProgress>^ PlayAnimationHL(ParametersView^ parameters);
		Windows::Foundation::IAsyncOperationWithProgress<bool, UpdateProgress>^ PlayAnimationFL(ParametersView^ parameters);


		void OnPointerInRange(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e);
		void SetPlayAnim(bool isPlay);
		void SetLensRadius(int Radius);

		ImageDetailsView^ GetImageDetails();
		Platform::String^ GetLog();


	private protected:

		virtual void Render() override;
		virtual void CreateDeviceResources() override;
		virtual void CreateSizeDependentResources() override;

		virtual void OnDeviceLost() override;

		void Log(std::chrono::time_point<std::chrono::system_clock> time, std::wstring message);
		void Log(std::wstring message);
		std::wstringstream m_log;

		Microsoft::WRL::ComPtr<IDXGIOutput>                 m_dxgiOutput;

		Microsoft::WRL::ComPtr<ID3D11RenderTargetView>      m_renderTargetView;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView>      m_depthStencilView;

		// Glare
		Parameters_Glare m_parametersGlare;
		CFFT m_fft;
		RenderTexture m_glareTexture;
		RenderTexture m_apertureTexture;
		RenderTexture m_glareOverlay;
		RenderTexture m_tonemappedImage;
		RenderTexture m_tonemappedImage2; // a second version of the tone-mapped image
		// For individual glares
		RenderTexture m_accumTemp[2];
		std::vector<RenderTexture> m_glareList;
		RenderTexture m_colormapped; // color mapped image
		RenderTexture m_temp;

		bool m_glareLoaded;
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_psfComputeShader;
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_eComputeShader;

		// Pupil
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_matricesConstantBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_pupilConstantBuffer;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_pupilInputLayout;

		Microsoft::WRL::ComPtr<ID3D11Buffer> m_pupilVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_pupilVertexShader;
		Microsoft::WRL::ComPtr<ID3D11GeometryShader> m_pupilFiberGeometryShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pupilFiberPixelShader;
		Microsoft::WRL::ComPtr<ID3D11GeometryShader> m_pupilParticleGeometryShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pupilParticlePixelShader;

		// Blob
		Parameters_Blob m_parametersBlob;
		RenderTexture m_blobMask;
		RenderTexture m_overlay;
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_findExtremaComputeShader;
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_makeMaskComputeShader;
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_maskComputeShader;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_blobBuffer;

		// Composition
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_glareLensComputeShader;

		// Texture of input image
		SourceRenderTexture m_inputTexture;

		// Texture of input image as float4
		RenderTexture m_texture;
		bool m_textureLoaded;

		// Details of input image
		ImageDetails m_imageDetails;

		Parameters_Output m_parameters;

		// Reinhard02
		Parameters_Reinhard02 m_parametersReinhard02;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBufferReinhard02;
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_reinhardComputeShader;

		// Mantiuk06
		Parameters_Mantiuk06 m_parametersMantiuk06;
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_divergenceComputeShader;
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_downSampleComputeShader;
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_upSampleComputeShader;
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_gradientComputeShader;
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_scaleFactorComputeShader;
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_transformToRComputeShader;
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_transformToGComputeShader;

		// Texture for output of operator
		RenderTexture m_outputTexture;
		// Texture for outputting glare, aperture, and other textures that have other format than input
		RenderTexture m_outputIntermediateTexture;
		// Texture for saving textures
		RenderTexture m_outputSaveTexture;

		// Render target of application
		TargetViewRenderTexture m_renderTarget;

		RenderToolkit m_renderToolkit;
		D3D11_VIEWPORT m_viewport;

		Windows::Foundation::IAsyncAction^					m_renderLoopWorker;
		// Rendering loop timer.
		DX::StepTimer                                       m_timer;
		// Transformations
		float   m_zoom;
		// Connected components
		std::vector<std::vector<D2D1_POINT_2F>>             m_brightPixCCList; // store locations of CC of bright pixels
		std::vector<std::vector<float>>                     m_brightPixValList; // Store values of CC of bright pixels
		std::vector<bool>                                   m_showGlares; // whether to show glares
		std::vector<std::vector<DirectX::XMFLOAT4>>                  m_connectedCompsImages;
		// glare lens params
		int												   m_glareLensPixRad; // radius of the glare lens in pixels
		bool                                               m_playAnim; // whether to play animation

		bool                                               m_dispGlares; // display with glare? 
	private:
		~HdrPanel();

		void Init();

		void Init_Glare();
		void Init_Blob();
		void Init_Reinhard02();
		void Init_Mantiuk06();

		void ProcessLoadedImage(HRESULT imageLoadingResult);

		void CreateTextures(UINT width, UINT height);

		void GetConnectedComps(DirectX::ScratchImage& src, std::vector<std::vector<DirectX::XMFLOAT4>>& dstImgs);


		IRenderTexture* GetDisplayOutputTexture();

		void Update_ImageDetails(concurrency::progress_reporter<UpdateProgress>& reporter, UpdateProgress& progress);
		void Update_Glare(concurrency::progress_reporter<UpdateProgress>& reporter, UpdateProgress& progress, IRenderTexture* target, IRenderTexture* glare, IRenderTexture* aperture, IRenderTexture* glareOverlay, IRenderTexture* overlay, IRenderTexture* source);
		void Update_indvidualGlares(concurrency::progress_reporter<UpdateProgress>& reporter, UpdateProgress& progress);

		void Update_Blob(concurrency::progress_reporter<UpdateProgress>& reporter, UpdateProgress& progress, IRenderTexture* blobMask, IRenderTexture* overlay, IRenderTexture* source, IRenderTexture* tonemappedSource);

		void Update_Reinhard02(concurrency::progress_reporter<UpdateProgress>& reporter, UpdateProgress& progress, IRenderTexture* target, IRenderTexture* source);
		void Update_Mantiuk06(concurrency::progress_reporter<UpdateProgress>& reporter, UpdateProgress& progress, IRenderTexture* target, IRenderTexture* source);
		void Update_Mai11(concurrency::progress_reporter<UpdateProgress>& reporter, UpdateProgress& progress, IRenderTexture* target, IRenderTexture* source);

		void Show(IRenderTexture* texture, bool show);
		void Show(IRenderTexture* texture);

		void AnimateGlaresHighlight();
		void AnimateGlaresFlicker();

		void ApplyColormap(IRenderTexture* target, IRenderTexture* source);
		void ShowWithGlares(const std::vector<bool>& showGlareList);
		void GlareLensRender(float radius, float centerX, float centerY, IRenderTexture* tonemappedImg, bool showBrightPixsOnly);
		HRESULT saveTextureImage(ID3D11Device1*  device, ID3D11DeviceContext1* context,
			HdrToolkit::RenderTexture texture, Platform::String^ filename);
	};
}