#pragma once

#include <DirectXMath.h>

#include "RenderTexture.h"

namespace HdrToolkit
{
	namespace RenderToolkitStructures
	{
		struct TextureRenderConstantBuffer
		{
			DirectX::XMFLOAT4X4 ViewMatrix;
			DirectX::XMFLOAT4X4 ProjectionMatrix;
		};

		struct BlurConstantBuffer
		{
			float StandardDeviation;
			float RadiusInStandardDeviations;
			DirectX::XMFLOAT2 Direction;
		};

		struct GammaCorrectionConstantBuffer
		{
			float Gamma;
			float ConstantMultiplier;
			DirectX::XMFLOAT2 padding;
		};

		struct GlareLensConstantBuffer
		{
			DirectX::XMFLOAT4 CircleInfo;
			DirectX::XMFLOAT4 TexInfo;

		};

		struct OperationConstantBuffer
		{
			DirectX::XMFLOAT4 Constant;
			DirectX::XMFLOAT4 OtherConstant;
		};

		struct TextureRenderVertex
		{
			DirectX::XMFLOAT2 TexCoord;
		};

	}

	namespace RenderToolkitHelper
	{
		HRESULT CreateRenderTexture(ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT format, D3D11_SUBRESOURCE_DATA* data, ID3D11Texture2D** texture, ID3D11RenderTargetView** renderTargetView, ID3D11ShaderResourceView** shaderResourceView, ID3D11UnorderedAccessView** unorderedAccessView);
		HRESULT CreateDepthStencil(ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT format, ID3D11Texture2D** texture, ID3D11DepthStencilView** depthStencilView);
		HRESULT SetShaders(ID3D11DeviceContext* context, ID3D11VertexShader* vertexShader, ID3D11GeometryShader* geometryShader, ID3D11PixelShader* pixelShader);
		HRESULT SetShaderBuffers(ID3D11DeviceContext* context, std::vector<ID3D11Buffer*> vertexShaderBuffers, std::vector<ID3D11Buffer*> geometryShaderBuffers, std::vector<ID3D11Buffer*> pixelShaderBuffers);
		HRESULT SetShaderSamplers(ID3D11DeviceContext* context, std::vector<ID3D11SamplerState*> vertexShaderSamplers, std::vector<ID3D11SamplerState*> geometryShaderSamplers, std::vector<ID3D11SamplerState*> pixelShaderSamplers);
		HRESULT SetShaderResources(ID3D11DeviceContext* context, std::vector<ID3D11ShaderResourceView*> vertexShaderResourceViews, std::vector<ID3D11ShaderResourceView*> geometryShaderResourceViews, std::vector<ID3D11ShaderResourceView*> pixelShaderResourceViews);
		HRESULT ClearRenderTarget(ID3D11DeviceContext* context, ID3D11RenderTargetView* renderTargetView, float red = 0.0f, float green = 0.0f, float blue = 0.0f, float alpha = 1.0f, ID3D11DepthStencilView* depthStencilView = nullptr, UINT depthClearFlags = D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, float depth = 1.0f, UINT8 depthStencil = 0);
		HRESULT UpdateResource(ID3D11DeviceContext* context, ID3D11Resource* resource, void* data);

		HRESULT SetComputeShaderResources(ID3D11DeviceContext* context, ID3D11ComputeShader* computeShader, std::vector<ID3D11Buffer*> buffers, std::vector<ID3D11UnorderedAccessView*> unorderedAccessViews, std::vector<ID3D11ShaderResourceView*> shaderResourceViews);
	}

	class RenderToolkit
	{
	public:
		RenderToolkit();

		void Init(ID3D11Device* device);

		HRESULT SetRasterState(ID3D11DeviceContext* context);
		HRESULT RenderTexture(ID3D11DeviceContext* context, ID3D11RenderTargetView* renderTargetView, ID3D11DepthStencilView* depthStencilView, D3D11_VIEWPORT viewport, ID3D11PixelShader* pixelShader, std::vector<ID3D11Buffer*> pixelShaderBuffers, std::vector<ID3D11ShaderResourceView*> pixelShaderShaderResourceViews);
		HRESULT RenderTexture(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* source, ID3D11PixelShader* pixelShader, std::vector<ID3D11Buffer*> pixelShaderBuffers, std::vector<ID3D11ShaderResourceView*> pixelShaderShaderResourceViews);
		HRESULT RenderTexture(ID3D11DeviceContext* context, ID3D11RenderTargetView* renderTargetView, ID3D11DepthStencilView* depthStencilView, D3D11_VIEWPORT viewport, ID3D11ShaderResourceView* textureView);
		HRESULT RenderTexture(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* source);
	

		HRESULT GammaCorrectTexture(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* source, float gamma, bool invertGamma = false, float constantMultiplier = 1.0f);

		HRESULT GlareLensTexture(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* sourceInLens, IRenderTexture* sourceAtBack, float radius, float centerX, float centerY);
		HRESULT RenderGlareLensTexture(ID3D11DeviceContext* context, IRenderTexture* target, const std::vector<IRenderTexture*>&  sourceList, ID3D11PixelShader* pixelShader, const std::vector<ID3D11Buffer*>& pixelShaderBuffers);

		HRESULT ComputeTexture(ID3D11DeviceContext* context, ID3D11ComputeShader* computeShader, std::vector<ID3D11RenderTargetView*> renderTargetViews, std::vector<ID3D11UnorderedAccessView*> unorderedAccessViews, std::vector<ID3D11Buffer*> buffers, std::vector<ID3D11ShaderResourceView*> shaderResourceViews, UINT width, UINT height, UINT depth = 1, UINT x = 16, UINT y = 16, UINT z = 1);

		HRESULT CSLuminance(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* source);
		HRESULT CSComplexMultiply(ID3D11DeviceContext* context, IRenderTexture* realTarget, IRenderTexture* imagTarget, IRenderTexture* firstRealSource, IRenderTexture* firstImagSource, IRenderTexture* secondRealSource, IRenderTexture* secondImagSource);
		HRESULT CSSum(ID3D11DeviceContext* context, std::vector<HdrToolkit::RenderTexture>& pyramid, IRenderTexture* source);
		HRESULT CSMin(ID3D11DeviceContext* context, std::vector<HdrToolkit::RenderTexture>& pyramid, IRenderTexture* source);
		HRESULT CSMax(ID3D11DeviceContext* context, std::vector<HdrToolkit::RenderTexture>& pyramid, IRenderTexture* source);
		HRESULT CSMinMax(ID3D11DeviceContext* context, std::vector<HdrToolkit::RenderTexture>& minPyramid, std::vector<HdrToolkit::RenderTexture>& maxPyramid, IRenderTexture* source);

		HRESULT CSCopy(ID3D11DeviceContext* context, IRenderTexture* taget, IRenderTexture* source);

		HRESULT CSAdd(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* firstSource, IRenderTexture* secondSource);
		HRESULT CSAdd(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* source, DirectX::XMFLOAT3 constant);
		HRESULT CSAdd(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* source, float constant);
		HRESULT CSSubtract(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* firstSource, IRenderTexture* secondSource);
		HRESULT CSMultiply(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* firstSource, IRenderTexture* secondSource);
		HRESULT CSMultiply(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* source, DirectX::XMFLOAT3 constant);
		HRESULT CSMultiply(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* source, float constant);
		HRESULT CSMultiplyEqual(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* source);
		HRESULT CSMultiplyEqual(ID3D11DeviceContext* context, IRenderTexture* target, float constant);
		HRESULT CSMultiplyScalar(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* firstSource, IRenderTexture* secondSource);
		HRESULT CSDivide(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* source, DirectX::XMFLOAT3 constant);
		HRESULT CSDivide(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* source, float constant);
		HRESULT CSDivide1(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* source, IRenderTexture* otherSource);
		HRESULT CSDivideScalar(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* firstSource, IRenderTexture* secondSource);

		HRESULT CSCombineLinear(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* firstSource, DirectX::XMFLOAT3 firstConstant, IRenderTexture* secondSource, DirectX::XMFLOAT3 secondConstant);
		HRESULT CSCombineLinear(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* firstSource, float firstConstant, IRenderTexture* secondSource, float secondConstant);
		HRESULT CSCombineLinearEqual(ID3D11DeviceContext* context, IRenderTexture* target, float targetConstant, IRenderTexture* source, float sourceConstant);

		HRESULT CSLowerBound(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* source, DirectX::XMFLOAT3 lowerBound);
		HRESULT CSLowerBound(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* source, float lowerBound);
		HRESULT CSUpperBound(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* source, DirectX::XMFLOAT3 upperBound);
		HRESULT CSUpperBound(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* source, float upperBound);
		HRESULT CSClamp(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* source, DirectX::XMFLOAT3 lowerBound, DirectX::XMFLOAT3 upperBound);
		HRESULT CSClamp(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* source, float lowerBound, float upperBound);

		HRESULT CSLog(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* source);
		HRESULT CSLog10(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* source);
		HRESULT CSExp10(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* source);

		HRESULT CSBlur(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* intermediateTexture, IRenderTexture* source, float standardDeviation, float radiusInStandardDeviations = 3.0f);
		HRESULT CSLaplace(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* source);
		HRESULT CSAbs(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* source);

		HRESULT CSShift(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* source, int xShift, int yShift);

		HRESULT CSColorMap(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* source, IRenderTexture* colorMap);

		std::vector<HdrToolkit::RenderTexture> Pyramid(ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT textureFormat, DXGI_FORMAT depthFormat = DXGI_FORMAT_UNKNOWN);
		std::vector<float> CopyToVector(ID3D11Device* device, ID3D11DeviceContext* context, IRenderTexture* texture);

	private:
		void Init_Misc(ID3D11Device* device);
		void Init_TextureRender(ID3D11Device* device);
		void Init_Blur(ID3D11Device* device);
		void Init_GammaCorrection(ID3D11Device* device);
		void Init_GlareLens(ID3D11Device* device);
		void Init_ComputeOperations(ID3D11Device* device);

		Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterState;

		Microsoft::WRL::ComPtr<ID3D11Buffer> m_textureRenderConstantBuffer;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> m_textureRenderSampler;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_textureRenderInputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_textureRenderVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_textureRenderVertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_textureRenderPixelShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_scalarTextureRenderPixelShader;

		Microsoft::WRL::ComPtr<ID3D11Buffer> m_blurConstantBuffer;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_blurPixelShader;

		Microsoft::WRL::ComPtr<ID3D11Buffer> m_gammaCorrectionConstantBuffer;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_gammaCorrectionPixelShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_scalarGammaCorrectionPixelShader;
		// Glare lens pixel shader
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_glareLensConstantBuffer;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_glareLensPixelShader;

		Microsoft::WRL::ComPtr<ID3D11Buffer> m_operationConstantBuffer;

		Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_luminanceComputeShader;
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_complexMultiplicationComputeShader;
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_sumComputeShader;
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_minComputeShader;
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_maxComputeShader;
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_copyComputeShader;
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_division1ComputeShader;
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_scalarDivisionComputeShader;
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_constantDivisionComputeShader;
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_additionComputeShader;
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_constantAdditionComputeShader;
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_subtractionComputeShader;
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_multiplicationComputeShader;
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_constantMultiplicationComputeShader;
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_scalarMultiplicationComputeShader;
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_multiplicationEqualComputeShader;
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_constantMultiplicationEqualComputeShader;
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_linearCombinationComputeShader;
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_linearCombinationEqualComputeShader;
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_lowerBoundComputeShader;
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_upperBoundComputeShader;
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_clampComputeShader;
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_logComputeShader;
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_log10ComputeShader;
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_exp10ComputeShader;
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_blurComputeShader;
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_laplaceComputeShader;
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_absoluteComputeShader;
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_shiftComputeShader;
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_colorMapComputeShader;
	};
}