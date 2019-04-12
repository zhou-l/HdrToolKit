#include "pch.h"
#include "RenderToolkit.h"

#include "DirectXHelper.h"

using namespace DirectX;
using namespace DX;
using namespace HdrToolkit;
using namespace concurrency;
using namespace Microsoft::WRL;

HRESULT RenderToolkitHelper::CreateRenderTexture(ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT format, D3D11_SUBRESOURCE_DATA* data, ID3D11Texture2D** texture, ID3D11RenderTargetView** renderTargetView, ID3D11ShaderResourceView** shaderResourceView, ID3D11UnorderedAccessView** unorderedAccessView)
{
	if (device == nullptr)
		return E_POINTER;
	if (width == 0 || height == 0)
		return E_INVALIDARG;

	HRESULT result = S_OK;

	UINT bindFlags = 0;
	if (shaderResourceView != nullptr)
	{
		bindFlags |= D3D11_BIND_SHADER_RESOURCE;
	}
	if (renderTargetView != nullptr)
	{
		bindFlags |= D3D11_BIND_RENDER_TARGET;
	}
	if (unorderedAccessView != nullptr)
	{
		bindFlags |= D3D11_BIND_UNORDERED_ACCESS;
	}

	CD3D11_TEXTURE2D_DESC textureDesc(
		format,
		width,
		height,
		1,
		1,
		bindFlags
	);

	result = device->CreateTexture2D(&textureDesc, data, texture);
	if (FAILED(result))
		return result;

	if (renderTargetView != nullptr)
	{
		result = device->CreateRenderTargetView(*texture, nullptr, renderTargetView);
		if (FAILED(result))
			return result;
	}

	if (shaderResourceView != nullptr)
	{
		result = device->CreateShaderResourceView(*texture, nullptr, shaderResourceView);
		if (FAILED(result))
			return result;
	}

	if (unorderedAccessView != nullptr)
	{
		result = device->CreateUnorderedAccessView(*texture, nullptr, unorderedAccessView);
	}

	return result;
}

HRESULT RenderToolkitHelper::CreateDepthStencil(ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT format, ID3D11Texture2D** texture, ID3D11DepthStencilView** depthStencilView)
{
	if (device == nullptr)
		return E_POINTER;
	if (width == 0 || height == 0)
		return E_INVALIDARG;

	HRESULT result = S_OK;

	CD3D11_TEXTURE2D_DESC depthStencilDesc(
		format,
		width,
		height,
		1,
		1,
		D3D11_BIND_DEPTH_STENCIL
	);

	result = device->CreateTexture2D(&depthStencilDesc, nullptr, texture);
	if (FAILED(result))
		return result;

	CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = CD3D11_DEPTH_STENCIL_VIEW_DESC(D3D11_DSV_DIMENSION_TEXTURE2D);
	result = device->CreateDepthStencilView(*texture, &depthStencilViewDesc, depthStencilView);
	return result;
}

HRESULT RenderToolkitHelper::SetShaders(ID3D11DeviceContext* context, ID3D11VertexShader* vertexShader, ID3D11GeometryShader* geometryShader, ID3D11PixelShader* pixelShader)
{
	if (context == nullptr)
		return E_POINTER;

	context->VSSetShader(vertexShader, nullptr, 0);
	context->GSSetShader(geometryShader, nullptr, 0);
	context->PSSetShader(pixelShader, nullptr, 0);

	return S_OK;
}

HRESULT RenderToolkitHelper::SetShaderBuffers(ID3D11DeviceContext* context, std::vector<ID3D11Buffer*> vertexShaderBuffers, std::vector<ID3D11Buffer*> geometryShaderBuffers, std::vector<ID3D11Buffer*> pixelShaderBuffers)
{
	if (context == nullptr)
		return E_POINTER;

	context->VSSetConstantBuffers(0, vertexShaderBuffers.size(), vertexShaderBuffers.data());
	context->GSSetConstantBuffers(0, geometryShaderBuffers.size(), geometryShaderBuffers.data());
	context->PSSetConstantBuffers(0, pixelShaderBuffers.size(), pixelShaderBuffers.data());

	return S_OK;
}

HRESULT RenderToolkitHelper::SetShaderSamplers(ID3D11DeviceContext* context, std::vector<ID3D11SamplerState*> vertexShaderSamplers, std::vector<ID3D11SamplerState*> geometryShaderSamplers, std::vector<ID3D11SamplerState*> pixelShaderSamplers)
{
	if (context == nullptr)
		return E_POINTER;

	context->VSSetSamplers(0, vertexShaderSamplers.size(), vertexShaderSamplers.data());
	context->GSSetSamplers(0, geometryShaderSamplers.size(), geometryShaderSamplers.data());
	context->PSSetSamplers(0, pixelShaderSamplers.size(), pixelShaderSamplers.data());

	return S_OK;
}

HRESULT RenderToolkitHelper::SetShaderResources(ID3D11DeviceContext* context, std::vector<ID3D11ShaderResourceView*> vertexShaderResourceViews, std::vector<ID3D11ShaderResourceView*> geometryShaderResourceViews, std::vector<ID3D11ShaderResourceView*> pixelShaderResourceViews)
{
	if (context == nullptr)
		return E_POINTER;

	context->VSSetShaderResources(0, vertexShaderResourceViews.size(), vertexShaderResourceViews.data());
	context->GSSetShaderResources(0, geometryShaderResourceViews.size(), geometryShaderResourceViews.data());
	context->PSSetShaderResources(0, pixelShaderResourceViews.size(), pixelShaderResourceViews.data());

	return S_OK;
}

HRESULT RenderToolkitHelper::ClearRenderTarget(ID3D11DeviceContext* context, ID3D11RenderTargetView* renderTargetView, float red, float green, float blue, float alpha, ID3D11DepthStencilView* depthStencilView, UINT depthClearFlags, float depth, UINT8 depthStencil)
{
	if (context == nullptr)
		return E_POINTER;

	float clearColor[4] =
	{
		red,
		green,
		blue,
		alpha
	};

	if (renderTargetView != nullptr)
	{
		context->ClearRenderTargetView(renderTargetView, clearColor);
	}

	if (depthStencilView != nullptr)
	{
		context->ClearDepthStencilView(depthStencilView, depthClearFlags, depth, depthStencil);
	}

	return S_OK;
}

HRESULT RenderToolkitHelper::UpdateResource(ID3D11DeviceContext* context, ID3D11Resource* resource, void* data)
{
	if (context == nullptr)
		return E_POINTER;
	if (resource == nullptr)
		return E_POINTER;
	if (data == nullptr)
		return E_POINTER;

	context->UpdateSubresource(resource, 0, nullptr, data, 0, 0);

	return S_OK;
}

HRESULT RenderToolkitHelper::SetComputeShaderResources(ID3D11DeviceContext* context, ID3D11ComputeShader* computeShader, std::vector<ID3D11Buffer*> buffers, std::vector<ID3D11UnorderedAccessView*> unorderedAccessViews, std::vector<ID3D11ShaderResourceView*> shaderResourceViews)
{
	if (context == nullptr)
		return E_POINTER;

	context->CSSetShader(computeShader, nullptr, 0);
	context->CSSetConstantBuffers(0, buffers.size(), buffers.data());
	context->CSSetUnorderedAccessViews(0, unorderedAccessViews.size(), unorderedAccessViews.data(), nullptr);
	context->CSSetShaderResources(0, shaderResourceViews.size(), shaderResourceViews.data());

	return S_OK;
}

RenderToolkit::RenderToolkit()
{

}

void RenderToolkit::Init(ID3D11Device* device)
{
	Init_TextureRender(device);
	Init_Misc(device);
	Init_Blur(device);
	Init_GammaCorrection(device);
	Init_GlareLens(device);
	Init_ComputeOperations(device);

}

HRESULT RenderToolkit::SetRasterState(ID3D11DeviceContext* context)
{
	if (context == nullptr)
		return E_POINTER;

	context->RSSetState(m_rasterState.Get());

	return S_OK;
}

HRESULT RenderToolkit::RenderTexture(ID3D11DeviceContext* context, ID3D11RenderTargetView* renderTargetView, ID3D11DepthStencilView* depthStencilView, D3D11_VIEWPORT viewport, ID3D11PixelShader* pixelShader, std::vector<ID3D11Buffer*> pixelShaderBuffers, std::vector<ID3D11ShaderResourceView*> pixelShaderShaderResourceViews)
{
	if (context == nullptr)
		return E_POINTER;

	HRESULT result = S_OK;

	context->RSSetState(m_rasterState.Get());
	context->RSSetViewports(1, &viewport);
	result = RenderToolkitHelper::ClearRenderTarget(context, renderTargetView, 0.0f, 0.0f, 0.0f, 1.0f, depthStencilView);
	if (FAILED(result))
		return result;

	UINT stride = sizeof(RenderToolkitStructures::TextureRenderVertex);
	UINT offset = 0;
	ID3D11Buffer* vertexBuffer = m_textureRenderVertexBuffer.Get();
	context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	context->IASetInputLayout(m_textureRenderInputLayout.Get());
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	result = RenderToolkitHelper::SetShaders(context, m_textureRenderVertexShader.Get(), nullptr, pixelShader);
	if (FAILED(result))
		return result;

	result = RenderToolkitHelper::SetShaderBuffers(context, { m_textureRenderConstantBuffer.Get() }, {}, pixelShaderBuffers);
	if (FAILED(result))
		return result;

	result = RenderToolkitHelper::SetShaderSamplers(context, {}, {}, { m_textureRenderSampler.Get() });
	if (FAILED(result))
		return result;

	result = RenderToolkitHelper::SetShaderResources(context, {}, {}, pixelShaderShaderResourceViews);
	if (FAILED(result))
		return result;

	context->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
	context->Draw(4, 0);
	context->OMSetRenderTargets(0, nullptr, nullptr);
	RenderToolkitHelper::SetShaderResources(context, {}, {}, std::vector<ID3D11ShaderResourceView*>(pixelShaderShaderResourceViews.size()));
	RenderToolkitHelper::SetShaderSamplers(context, {}, {}, { nullptr });
	RenderToolkitHelper::SetShaderBuffers(context, { nullptr }, {}, std::vector<ID3D11Buffer*>(pixelShaderBuffers.size()));
	RenderToolkitHelper::SetShaders(context, nullptr, nullptr, nullptr);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED);
	context->IASetInputLayout(nullptr);
	vertexBuffer = nullptr;
	stride = 0;
	offset = 0;
	context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	context->RSSetViewports(0, nullptr);

	return result;
}

HRESULT RenderToolkit::RenderTexture(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* source, ID3D11PixelShader* pixelShader, std::vector<ID3D11Buffer*> pixelShaderBuffers, std::vector<ID3D11ShaderResourceView*> pixelShaderShaderResourceViews)
{
	pixelShaderShaderResourceViews.insert(pixelShaderShaderResourceViews.begin(), source->GetShaderResourceView());
	return RenderTexture(context, target->GetRenderTargetView(), target->GetDepthStencilView(), target->GetFullViewport(), pixelShader, pixelShaderBuffers, pixelShaderShaderResourceViews);
}

HRESULT RenderToolkit::RenderTexture(ID3D11DeviceContext* context, ID3D11RenderTargetView* renderTargetView, ID3D11DepthStencilView* depthStencilView, D3D11_VIEWPORT viewport, ID3D11ShaderResourceView* textureView)
{
	return RenderTexture(context, renderTargetView, depthStencilView, viewport, m_textureRenderPixelShader.Get(), {}, { textureView });
}

HRESULT RenderToolkit::RenderTexture(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* source)
{
	ID3D11PixelShader* pixelShader;
	if (DX::IsSingleChannel(source->GetTextureFormat()))
	{
		pixelShader = m_scalarTextureRenderPixelShader.Get();
	}
	else
	{
		pixelShader = m_textureRenderPixelShader.Get();
	}

	return RenderTexture(context, target, source, pixelShader, {}, {});
}

HRESULT RenderToolkit::GammaCorrectTexture(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* source, float gamma, bool invertGamma, float constantMutliplier)
{
	HRESULT result = S_OK;

	ID3D11PixelShader* pixelShader;
	if (DX::IsSingleChannel(source->GetTextureFormat()))
	{
		pixelShader = m_scalarGammaCorrectionPixelShader.Get();
	}
	else
	{
		pixelShader = m_gammaCorrectionPixelShader.Get();
	}

	RenderToolkitStructures::GammaCorrectionConstantBuffer constantBuffer;
	constantBuffer.Gamma = invertGamma ? (1.0f / gamma) : gamma;
	constantBuffer.ConstantMultiplier = constantMutliplier;
	result = RenderToolkitHelper::UpdateResource(context, m_gammaCorrectionConstantBuffer.Get(), &constantBuffer);
	if (FAILED(result))
		return result;

	result = RenderTexture(context, target, source, pixelShader, { m_gammaCorrectionConstantBuffer.Get() }, {});
	return result;
}

HRESULT RenderToolkit::GlareLensTexture(ID3D11DeviceContext * context, IRenderTexture * target, IRenderTexture * sourceInLens, IRenderTexture * sourceAtBack, float radius, float centerX, float centerY)
{
	HRESULT result = S_OK;
	ID3D11PixelShader* pixelShader = m_glareLensPixelShader.Get();
	RenderToolkitStructures::GlareLensConstantBuffer constantBuffer;
	constantBuffer.CircleInfo = DirectX::XMFLOAT4(centerX, centerY, radius, radius);
	constantBuffer.TexInfo = DirectX::XMFLOAT4(float(target->GetWidth()), float(target->GetHeight()), 1, 1);

	result = RenderToolkitHelper::UpdateResource(context, m_glareLensConstantBuffer.Get(), &constantBuffer);
	if (FAILED(result))
		return result;
	result = RenderGlareLensTexture(context, target, { sourceInLens, sourceAtBack }, pixelShader, { m_glareLensConstantBuffer.Get() });
	return result;

}

HRESULT RenderToolkit::RenderGlareLensTexture(ID3D11DeviceContext * context, IRenderTexture * target, const std::vector<IRenderTexture*>& sourceList, ID3D11PixelShader* pixelShader, const std::vector<ID3D11Buffer*>& pixelShaderBuffers)
{

	if (context == nullptr)
		return E_POINTER;

	HRESULT result = S_OK;
	ID3D11RenderTargetView* renderTargetView = target->GetRenderTargetView();
	ID3D11DepthStencilView* depthStencilView = target->GetDepthStencilView();
	D3D11_VIEWPORT viewport = target->GetFullViewport();
	// Set shader resource views
	std::vector<ID3D11ShaderResourceView*> pixelShaderShaderResourceViews;
	for (size_t i = 0; i < sourceList.size(); i++)
		//pixelShaderShaderResourceViews.insert(pixelShaderShaderResourceViews.begin(), sourceList[i]->GetShaderResourceView());
		pixelShaderShaderResourceViews.push_back(sourceList[i]->GetShaderResourceView());

	context->RSSetState(m_rasterState.Get());
	context->RSSetViewports(1, &viewport);
	result = RenderToolkitHelper::ClearRenderTarget(context, renderTargetView, 0.0f, 0.0f, 0.0f, 1.0f, depthStencilView);
	if (FAILED(result))
		return result;

	UINT stride = sizeof(RenderToolkitStructures::TextureRenderVertex);
	UINT offset = 0;
	ID3D11Buffer* vertexBuffer = m_textureRenderVertexBuffer.Get();
	context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	context->IASetInputLayout(m_textureRenderInputLayout.Get());
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	result = RenderToolkitHelper::SetShaders(context, m_textureRenderVertexShader.Get(), nullptr, pixelShader);
	if (FAILED(result))
		return result;

	result = RenderToolkitHelper::SetShaderBuffers(context, { m_textureRenderConstantBuffer.Get() }, {}, pixelShaderBuffers);
	if (FAILED(result))
		return result;

	result = RenderToolkitHelper::SetShaderSamplers(context, {}, {}, { m_textureRenderSampler.Get() });
	if (FAILED(result))
		return result;

	result = RenderToolkitHelper::SetShaderResources(context, {}, {}, pixelShaderShaderResourceViews);
	if (FAILED(result))
		return result;

	context->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
	context->Draw(4, 0);
	context->OMSetRenderTargets(0, nullptr, nullptr);
	RenderToolkitHelper::SetShaderResources(context, {}, {}, std::vector<ID3D11ShaderResourceView*>(pixelShaderShaderResourceViews.size()));
	RenderToolkitHelper::SetShaderSamplers(context, {}, {}, { nullptr });
	RenderToolkitHelper::SetShaderBuffers(context, { nullptr }, {}, std::vector<ID3D11Buffer*>(pixelShaderBuffers.size()));
	RenderToolkitHelper::SetShaders(context, nullptr, nullptr, nullptr);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED);
	context->IASetInputLayout(nullptr);
	vertexBuffer = nullptr;
	stride = 0;
	offset = 0;
	context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	context->RSSetViewports(0, nullptr);

	return result;
}

HRESULT RenderToolkit::ComputeTexture(ID3D11DeviceContext* context, ID3D11ComputeShader* computeShader, std::vector<ID3D11RenderTargetView*> renderTargetViews, std::vector<ID3D11UnorderedAccessView*> unorderedAccessViews, std::vector<ID3D11Buffer*> buffers, std::vector<ID3D11ShaderResourceView*> shaderResourceViews, UINT width, UINT height, UINT depth, UINT x, UINT y, UINT z)
{
	if (context == nullptr)
		return E_POINTER;
	if (computeShader == nullptr)
		return E_POINTER;

	HRESULT result = S_OK;
	for (auto renderTargetView : renderTargetViews)
	{
		result = RenderToolkitHelper::ClearRenderTarget(context, renderTargetView);
		if (FAILED(result))
			return result;
	}

	result = RenderToolkitHelper::SetComputeShaderResources(context, computeShader, buffers, unorderedAccessViews, shaderResourceViews);
	if (FAILED(result))
		return result;

	context->Dispatch(DivRoundUp(width, x), DivRoundUp(height, y), DivRoundUp(depth, z));

	result = RenderToolkitHelper::SetComputeShaderResources(context, nullptr, std::vector<ID3D11Buffer*>(buffers.size()), std::vector<ID3D11UnorderedAccessView*>(unorderedAccessViews.size()), std::vector<ID3D11ShaderResourceView*>(shaderResourceViews.size()));

	return result;
}

HRESULT RenderToolkit::CSLuminance(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* source)
{
	return ComputeTexture(context, m_luminanceComputeShader.Get(), { target->GetRenderTargetView() }, { target->GetUnorderedAccessView() }, {}, { source->GetShaderResourceView() }, target->GetWidth(), target->GetHeight());
}

HRESULT RenderToolkit::CSComplexMultiply(ID3D11DeviceContext* context, IRenderTexture* realTarget, IRenderTexture* imagTarget, IRenderTexture* firstRealSource, IRenderTexture* firstImagSource, IRenderTexture* secondRealSource, IRenderTexture* secondImagSource)
{
	return ComputeTexture(context, m_complexMultiplicationComputeShader.Get(), { realTarget->GetRenderTargetView(), imagTarget->GetRenderTargetView() }, { realTarget->GetUnorderedAccessView(), imagTarget->GetUnorderedAccessView() }, {}, { firstRealSource->GetShaderResourceView(), firstImagSource->GetShaderResourceView(), secondRealSource->GetShaderResourceView(), secondImagSource->GetShaderResourceView() }, realTarget->GetWidth(), realTarget->GetHeight());
}

HRESULT RenderToolkit::CSSum(ID3D11DeviceContext* context, std::vector<HdrToolkit::RenderTexture>& pyramid, IRenderTexture* source)
{
	HRESULT result = S_OK;

	for (int i = pyramid.size() - 1; i >= 0; i--)
	{
		IRenderTexture* currentTexture = pyramid.at(i).Get();
		IRenderTexture* lastTexture = (i == pyramid.size() - 1) ? source : pyramid.at(i + 1).Get();

		UINT width = currentTexture->GetWidth();
		UINT height = currentTexture->GetHeight();

		result = ComputeTexture(context, m_sumComputeShader.Get(), {}, { currentTexture->GetUnorderedAccessView() }, {}, { lastTexture->GetShaderResourceView() }, width, height);
		if (FAILED(result))
			return result;
	}

	return result;
}

HRESULT RenderToolkit::CSMin(ID3D11DeviceContext* context, std::vector<HdrToolkit::RenderTexture>& pyramid, IRenderTexture* source)
{
	HRESULT result = S_OK;

	for (int i = pyramid.size() - 1; i >= 0; i--)
	{
		IRenderTexture* currentTexture = pyramid.at(i).Get();
		IRenderTexture* lastTexture = (i == pyramid.size() - 1) ? source : pyramid.at(i + 1).Get();

		UINT width = currentTexture->GetWidth();
		UINT height = currentTexture->GetHeight();

		result = ComputeTexture(context, m_minComputeShader.Get(), {}, { currentTexture->GetUnorderedAccessView() }, {}, { lastTexture->GetShaderResourceView() }, width, height);
		if (FAILED(result))
			return result;
	}

	return result;
}

HRESULT RenderToolkit::CSMax(ID3D11DeviceContext* context, std::vector<HdrToolkit::RenderTexture>& pyramid, IRenderTexture* source)
{
	HRESULT result = S_OK;

	for (int i = pyramid.size() - 1; i >= 0; i--)
	{
		IRenderTexture* currentTexture = pyramid.at(i).Get();
		IRenderTexture* lastTexture = (i == pyramid.size() - 1) ? source : pyramid.at(i + 1).Get();

		UINT width = currentTexture->GetWidth();
		UINT height = currentTexture->GetHeight();

		result = ComputeTexture(context, m_maxComputeShader.Get(), {}, { currentTexture->GetUnorderedAccessView() }, {}, { lastTexture->GetShaderResourceView() }, width, height);
		if (FAILED(result))
			return result;
	}

	return result;
}

HRESULT RenderToolkit::CSMinMax(ID3D11DeviceContext* context, std::vector<HdrToolkit::RenderTexture>& minPyramid, std::vector<HdrToolkit::RenderTexture>& maxPyramid, IRenderTexture* source)
{
	HRESULT result = S_OK;
	result = CSMin(context, minPyramid, source);
	if (FAILED(result))
		return result;

	result = CSMax(context, maxPyramid, source);
	return result;
}

HRESULT RenderToolkit::CSCopy(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* source)
{
	return ComputeTexture(context, m_copyComputeShader.Get(), { target->GetRenderTargetView() }, { target->GetUnorderedAccessView() }, {}, { source->GetShaderResourceView() }, target->GetWidth(), target->GetHeight());
}

HRESULT RenderToolkit::CSAdd(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* firstSource, IRenderTexture* secondSource)
{
	return ComputeTexture(context, m_additionComputeShader.Get(), { target->GetRenderTargetView() }, { target->GetUnorderedAccessView() }, {}, { firstSource->GetShaderResourceView(), secondSource->GetShaderResourceView() }, target->GetWidth(), target->GetHeight());
}
HRESULT RenderToolkit::CSAdd(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* source, XMFLOAT3 constant)
{
	HRESULT result = S_OK;

	RenderToolkitStructures::OperationConstantBuffer constantBuffer;
	constantBuffer.Constant.x = constant.x;
	constantBuffer.Constant.y = constant.y;
	constantBuffer.Constant.z = constant.z;
	result = RenderToolkitHelper::UpdateResource(context, m_operationConstantBuffer.Get(), &constantBuffer);
	if (FAILED(result))
		return result;

	return ComputeTexture(context, m_constantAdditionComputeShader.Get(), { target->GetRenderTargetView() }, { target->GetUnorderedAccessView() }, { m_operationConstantBuffer.Get() }, { source->GetShaderResourceView() }, target->GetWidth(), target->GetHeight());
}
HRESULT RenderToolkit::CSAdd(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* source, float constant)
{
	return CSAdd(context, target, source, XMFLOAT3(constant, constant, constant));
}
HRESULT RenderToolkit::CSSubtract(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* firstSource, IRenderTexture* secondSource)
{
	return ComputeTexture(context, m_subtractionComputeShader.Get(), { target->GetRenderTargetView() }, { target->GetUnorderedAccessView() }, {}, { firstSource->GetShaderResourceView(), secondSource->GetShaderResourceView() }, target->GetWidth(), target->GetHeight());
}
HRESULT RenderToolkit::CSMultiply(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* firstSource, IRenderTexture* secondSource)
{
	return ComputeTexture(context, m_multiplicationComputeShader.Get(), { target->GetRenderTargetView() }, { target->GetUnorderedAccessView() }, {}, { firstSource->GetShaderResourceView(), secondSource->GetShaderResourceView() }, target->GetWidth(), target->GetHeight());
}
HRESULT RenderToolkit::CSMultiply(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* source, XMFLOAT3 constant)
{
	HRESULT result = S_OK;

	RenderToolkitStructures::OperationConstantBuffer constantBuffer;
	constantBuffer.Constant.x = constant.x;
	constantBuffer.Constant.y = constant.y;
	constantBuffer.Constant.z = constant.z;
	result = RenderToolkitHelper::UpdateResource(context, m_operationConstantBuffer.Get(), &constantBuffer);
	if (FAILED(result))
		return result;

	return ComputeTexture(context, m_constantMultiplicationComputeShader.Get(), { target->GetRenderTargetView() }, { target->GetUnorderedAccessView() }, { m_operationConstantBuffer.Get() }, { source->GetShaderResourceView() }, target->GetWidth(), target->GetHeight());
}
HRESULT RenderToolkit::CSMultiply(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* source, float constant)
{
	return CSMultiply(context, target, source, XMFLOAT3(constant, constant, constant));
}

HRESULT RenderToolkit::CSMultiplyEqual(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* source)
{
	return ComputeTexture(context, m_multiplicationEqualComputeShader.Get(), {}, { target->GetUnorderedAccessView() }, {}, { source->GetShaderResourceView() }, target->GetWidth(), target->GetHeight());
}

HRESULT RenderToolkit::CSMultiplyEqual(ID3D11DeviceContext* context, IRenderTexture* target, float constant)
{
	HRESULT result = S_OK;

	RenderToolkitStructures::OperationConstantBuffer constantBuffer;
	constantBuffer.Constant.x = constant;
	result = RenderToolkitHelper::UpdateResource(context, m_operationConstantBuffer.Get(), &constantBuffer);
	if (FAILED(result))
		return result;

	return ComputeTexture(context, m_constantMultiplicationEqualComputeShader.Get(), {}, { target->GetUnorderedAccessView() }, { m_operationConstantBuffer.Get() }, {}, target->GetWidth(), target->GetHeight());
}

HRESULT RenderToolkit::CSMultiplyScalar(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* firstSource, IRenderTexture* secondSource)
{
	return ComputeTexture(context, m_scalarMultiplicationComputeShader.Get(), { target->GetRenderTargetView() }, { target->GetUnorderedAccessView() }, {}, { firstSource->GetShaderResourceView(), secondSource->GetShaderResourceView() }, target->GetWidth(), target->GetHeight());
}

HRESULT RenderToolkit::CSDivide(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* source, XMFLOAT3 constant)
{
	HRESULT result = S_OK;

	RenderToolkitStructures::OperationConstantBuffer constantBuffer;
	constantBuffer.Constant.x = constant.x;
	constantBuffer.Constant.y = constant.y;
	constantBuffer.Constant.z = constant.z;
	result = RenderToolkitHelper::UpdateResource(context, m_operationConstantBuffer.Get(), &constantBuffer);
	if (FAILED(result))
		return result;

	return ComputeTexture(context, m_constantDivisionComputeShader.Get(), { target->GetRenderTargetView() }, { target->GetUnorderedAccessView() }, { m_operationConstantBuffer.Get() }, { source->GetShaderResourceView() }, target->GetWidth(), target->GetHeight());
}
HRESULT RenderToolkit::CSDivide(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* source, float constant)
{
	return CSDivide(context, target, source, XMFLOAT3(constant, constant, constant));
}

HRESULT RenderToolkit::CSDivide1(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* source, IRenderTexture* otherSource)
{
	return ComputeTexture(context, m_division1ComputeShader.Get(), { target->GetRenderTargetView() }, { target->GetUnorderedAccessView() }, {}, { source->GetShaderResourceView(), otherSource->GetShaderResourceView() }, target->GetWidth(), target->GetHeight());
}

HRESULT RenderToolkit::CSDivideScalar(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* firstSource, IRenderTexture* secondSource)
{
	return ComputeTexture(context, m_scalarDivisionComputeShader.Get(), { target->GetRenderTargetView() }, { target->GetUnorderedAccessView() }, {}, { firstSource->GetShaderResourceView(), secondSource->GetShaderResourceView() }, target->GetWidth(), target->GetHeight());
}

HRESULT RenderToolkit::CSCombineLinear(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* firstSource, XMFLOAT3 firstConstant, IRenderTexture* secondSource, XMFLOAT3 secondConstant)
{
	HRESULT result = S_OK;

	RenderToolkitStructures::OperationConstantBuffer constantBuffer;
	constantBuffer.Constant.x = firstConstant.x;
	constantBuffer.Constant.y = firstConstant.y;
	constantBuffer.Constant.z = firstConstant.z;
	constantBuffer.OtherConstant.x = secondConstant.x;
	constantBuffer.OtherConstant.y = secondConstant.y;
	constantBuffer.OtherConstant.z = secondConstant.z;
	result = RenderToolkitHelper::UpdateResource(context, m_operationConstantBuffer.Get(), &constantBuffer);
	if (FAILED(result))
		return result;

	return ComputeTexture(context, m_linearCombinationComputeShader.Get(), { target->GetRenderTargetView() }, { target->GetUnorderedAccessView() }, { m_operationConstantBuffer.Get() }, { firstSource->GetShaderResourceView(), secondSource->GetShaderResourceView() }, target->GetWidth(), target->GetHeight());
}

HRESULT RenderToolkit::CSCombineLinear(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* firstSource, float firstConstant, IRenderTexture* secondSource, float secondConstant)
{
	return CSCombineLinear(context, target, firstSource, XMFLOAT3(firstConstant, firstConstant, firstConstant), secondSource, XMFLOAT3(secondConstant, secondConstant, secondConstant));
}

HRESULT RenderToolkit::CSCombineLinearEqual(ID3D11DeviceContext* context, IRenderTexture* target, float targetConstant, IRenderTexture* source, float sourceConstant)
{
	HRESULT result = S_OK;

	RenderToolkitStructures::OperationConstantBuffer constantBuffer;
	constantBuffer.Constant.x = targetConstant;
	constantBuffer.Constant.y = sourceConstant;
	result = RenderToolkitHelper::UpdateResource(context, m_operationConstantBuffer.Get(), &constantBuffer);
	if (FAILED(result))
		return result;

	return ComputeTexture(context, m_linearCombinationEqualComputeShader.Get(), {}, { target->GetUnorderedAccessView() }, { m_operationConstantBuffer.Get() }, { source->GetShaderResourceView() }, target->GetWidth(), target->GetHeight());
}

HRESULT RenderToolkit::CSLowerBound(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* source, XMFLOAT3 lowerBound)
{
	HRESULT result = S_OK;

	RenderToolkitStructures::OperationConstantBuffer constantBuffer;
	constantBuffer.Constant.x = lowerBound.x;
	constantBuffer.Constant.y = lowerBound.y;
	constantBuffer.Constant.z = lowerBound.z;
	result = RenderToolkitHelper::UpdateResource(context, m_operationConstantBuffer.Get(), &constantBuffer);
	if (FAILED(result))
		return result;

	return ComputeTexture(context, m_lowerBoundComputeShader.Get(), { target->GetRenderTargetView() }, { target->GetUnorderedAccessView() }, { m_operationConstantBuffer.Get() }, { source->GetShaderResourceView() }, target->GetWidth(), target->GetHeight());
}

HRESULT RenderToolkit::CSLowerBound(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* source, float lowerBound)
{
	return CSLowerBound(context, target, source, XMFLOAT3(lowerBound, lowerBound, lowerBound));
}

HRESULT RenderToolkit::CSUpperBound(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* source, XMFLOAT3 upperBound)
{
	HRESULT result = S_OK;

	RenderToolkitStructures::OperationConstantBuffer constantBuffer;
	constantBuffer.Constant.x = upperBound.x;
	constantBuffer.Constant.y = upperBound.y;
	constantBuffer.Constant.z = upperBound.z;
	result = RenderToolkitHelper::UpdateResource(context, m_operationConstantBuffer.Get(), &constantBuffer);
	if (FAILED(result))
		return result;

	return ComputeTexture(context, m_upperBoundComputeShader.Get(), { target->GetRenderTargetView() }, { target->GetUnorderedAccessView() }, { m_operationConstantBuffer.Get() }, { source->GetShaderResourceView() }, target->GetWidth(), target->GetHeight());
}

HRESULT RenderToolkit::CSUpperBound(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* source, float upperBound)
{
	return CSUpperBound(context, target, source, XMFLOAT3(upperBound, upperBound, upperBound));
}

HRESULT RenderToolkit::CSClamp(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* source, XMFLOAT3 lowerBound, XMFLOAT3 upperBound)
{
	HRESULT result = S_OK;

	RenderToolkitStructures::OperationConstantBuffer constantBuffer;
	constantBuffer.Constant.x = lowerBound.x;
	constantBuffer.Constant.y = lowerBound.y;
	constantBuffer.Constant.z = lowerBound.z;
	constantBuffer.OtherConstant.x = upperBound.x;
	constantBuffer.OtherConstant.y = upperBound.y;
	constantBuffer.OtherConstant.z = upperBound.z;
	result = RenderToolkitHelper::UpdateResource(context, m_operationConstantBuffer.Get(), &constantBuffer);
	if (FAILED(result))
		return result;

	return ComputeTexture(context, m_clampComputeShader.Get(), { target->GetRenderTargetView() }, { target->GetUnorderedAccessView() }, { m_operationConstantBuffer.Get() }, { source->GetShaderResourceView() }, target->GetWidth(), target->GetHeight());
}

HRESULT RenderToolkit::CSClamp(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* source, float lowerBound, float upperBound)
{
	return CSClamp(context, target, source, XMFLOAT3(lowerBound, lowerBound, lowerBound), XMFLOAT3(upperBound, upperBound, upperBound));
}

HRESULT RenderToolkit::CSLog(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* source)
{
	return ComputeTexture(context, m_logComputeShader.Get(), { target->GetRenderTargetView() }, { target->GetUnorderedAccessView() }, {}, { source->GetShaderResourceView() }, target->GetWidth(), target->GetHeight());
}

HRESULT RenderToolkit::CSLog10(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* source)
{
	return ComputeTexture(context, m_log10ComputeShader.Get(), { target->GetRenderTargetView() }, { target->GetUnorderedAccessView() }, {}, { source->GetShaderResourceView() }, target->GetWidth(), target->GetHeight());
}

HRESULT RenderToolkit::CSExp10(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* source)
{
	return ComputeTexture(context, m_exp10ComputeShader.Get(), { target->GetRenderTargetView() }, { target->GetUnorderedAccessView() }, {}, { source->GetShaderResourceView() }, target->GetWidth(), target->GetHeight());
}

HRESULT RenderToolkit::CSBlur(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* intermediateTexture, IRenderTexture* source, float standardDeviation, float radiusInStandardDeviations)
{
	HRESULT result = S_OK;

	RenderToolkitStructures::BlurConstantBuffer constantBuffer;
	constantBuffer.StandardDeviation = standardDeviation;
	constantBuffer.RadiusInStandardDeviations = radiusInStandardDeviations;
	constantBuffer.Direction = XMFLOAT2(1.0f, 0.0f);
	result = RenderToolkitHelper::UpdateResource(context, m_blurConstantBuffer.Get(), &constantBuffer);
	if (FAILED(result))
		return result;

	result = ComputeTexture(context, m_blurComputeShader.Get(), { intermediateTexture->GetRenderTargetView() }, { intermediateTexture->GetUnorderedAccessView() }, { m_blurConstantBuffer.Get() }, { source->GetShaderResourceView() }, target->GetWidth(), target->GetHeight());
	if (FAILED(result))
		return result;

	constantBuffer.Direction = XMFLOAT2(0.0f, 1.0f);
	result = RenderToolkitHelper::UpdateResource(context, m_blurConstantBuffer.Get(), &constantBuffer);
	if (FAILED(result))
		return result;

	result = ComputeTexture(context, m_blurComputeShader.Get(), { target->GetRenderTargetView() }, { target->GetUnorderedAccessView() }, { m_blurConstantBuffer.Get() }, { intermediateTexture->GetShaderResourceView() }, target->GetWidth(), target->GetHeight());
	return result;
}

HRESULT RenderToolkit::CSLaplace(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* source)
{
	return ComputeTexture(context, m_laplaceComputeShader.Get(), { target->GetRenderTargetView() }, { target->GetUnorderedAccessView() }, {}, { source->GetShaderResourceView() }, target->GetWidth(), target->GetHeight());
}

HRESULT RenderToolkit::CSAbs(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* source)
{
	return ComputeTexture(context, m_absoluteComputeShader.Get(), { target->GetRenderTargetView() }, { target->GetUnorderedAccessView() }, {}, { source->GetShaderResourceView() }, target->GetWidth(), target->GetHeight());
}

HRESULT RenderToolkit::CSShift(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* source, int xShift, int yShift)
{
	HRESULT result = S_OK;

	RenderToolkitStructures::OperationConstantBuffer constantBuffer;
	*reinterpret_cast<int*>(&constantBuffer.Constant.x) = xShift;
	*reinterpret_cast<int*>(&constantBuffer.Constant.y) = yShift;
	result = RenderToolkitHelper::UpdateResource(context, m_operationConstantBuffer.Get(), &constantBuffer);
	if (FAILED(result))
		return result;

	return ComputeTexture(context, m_shiftComputeShader.Get(), { target->GetRenderTargetView() }, { target->GetUnorderedAccessView() }, { m_operationConstantBuffer.Get() }, { source->GetShaderResourceView() }, target->GetWidth(), target->GetHeight());
}

HRESULT RenderToolkit::CSColorMap(ID3D11DeviceContext* context, IRenderTexture* target, IRenderTexture* source, IRenderTexture* colorMap)
{
	return ComputeTexture(context, m_colorMapComputeShader.Get(), { target->GetRenderTargetView() }, { target->GetUnorderedAccessView() }, {}, { source->GetShaderResourceView(), colorMap->GetShaderResourceView() }, target->GetWidth(), target->GetHeight());
}

std::vector<HdrToolkit::RenderTexture> RenderToolkit::Pyramid(ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT textureFormat, DXGI_FORMAT depthFormat)
{
	HRESULT result = S_OK;

	std::vector<HdrToolkit::RenderTexture> pyramid;
	for (UINT size = 1; size < std::max(width, height); size *= 2)
	{
		HdrToolkit::RenderTexture texture;
		ThrowIfFailed(texture.Create(device, size, size, textureFormat, depthFormat));

		pyramid.push_back(texture);
	}

	return pyramid;
}

std::vector<float> RenderToolkit::CopyToVector(ID3D11Device* device, ID3D11DeviceContext* context, IRenderTexture* texture)
{
	ScratchImage scratchImage;
	ThrowIfFailed(
		CaptureTexture(
			device,
			context,
			texture->GetTexture(),
			scratchImage
		)
	);

	auto image = scratchImage.GetImages();

	if (image->format != DXGI_FORMAT_R32G32B32A32_FLOAT && image->format != DXGI_FORMAT_R32_FLOAT)
		ThrowIfFailed(E_INVALIDARG);

	auto pixels = reinterpret_cast<float*>(image->pixels);
	return std::vector<float>(pixels, pixels + image->width * image->height * (image->format == DXGI_FORMAT_R32G32B32A32_FLOAT ? 4 : 1));
}

void RenderToolkit::Init_Misc(ID3D11Device* device)
{
	D3D11_RASTERIZER_DESC rasterDesc;
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_NONE;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = true;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	ThrowIfFailed(
		device->CreateRasterizerState(
			&rasterDesc,
			m_rasterState.ReleaseAndGetAddressOf()
		)
	);
}

void RenderToolkit::Init_TextureRender(ID3D11Device* device)
{
	std::vector<std::function<void()>> work
	{
		// Create vertex shader and input layout
		[=]()
		{
			auto shaderCode = DX::ReadData(L"TextureRenderVertexShader.cso");

			ThrowIfFailed(
				device->CreateVertexShader(
					shaderCode.data(),
					shaderCode.size(),
					nullptr,
					m_textureRenderVertexShader.ReleaseAndGetAddressOf()
				)
			);

			static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
			{
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};

			ThrowIfFailed(
				device->CreateInputLayout(
					vertexDesc,
					ARRAYSIZE(vertexDesc),
					shaderCode.data(),
					shaderCode.size(),
					m_textureRenderInputLayout.ReleaseAndGetAddressOf()
				)
			);
		},
		// Create pixel shader
		[=]() { DX::CreateShader(device, m_textureRenderPixelShader.ReleaseAndGetAddressOf(), L"TextureRenderPixelShader.cso"); },
			// Create scalar pixel shader
			[=]() { DX::CreateShader(device, m_scalarTextureRenderPixelShader.ReleaseAndGetAddressOf(), L"ScalarTextureRenderPixelShader.cso"); },
			// Create contant buffer
			[=]()
			{
				RenderToolkitStructures::TextureRenderConstantBuffer constantBuffer;

				XMMATRIX perspectiveMatrix = XMMatrixOrthographicLH(1.0f, 1.0f, 1.0f, 2.0f);
				XMFLOAT4X4 orientation(
					1.0f, 0.0f, 0.0f, 0.0f,
					0.0f, 1.0f, 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f,
					0.0f, 0.0f, 0.0f, 1.0f
				);
				XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);
				XMStoreFloat4x4(
					&constantBuffer.ProjectionMatrix,
					XMMatrixTranspose(perspectiveMatrix * orientationMatrix)
				);

				static const XMVECTORF32 eye = { 0.5f, 0.5f, -1.0f, 0.0f };
				static const XMVECTORF32 at = { 0.5f, 0.5f, 0.0f, 0.0f };
				static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };
				XMStoreFloat4x4(
					&constantBuffer.ViewMatrix,
					XMMatrixTranspose(XMMatrixLookAtLH(eye, at, up))
				);

				D3D11_SUBRESOURCE_DATA constantBufferData;
				constantBufferData.pSysMem = &constantBuffer;
				constantBufferData.SysMemPitch = sizeof(RenderToolkitStructures::TextureRenderConstantBuffer);
				constantBufferData.SysMemSlicePitch = sizeof(RenderToolkitStructures::TextureRenderConstantBuffer);

				CD3D11_BUFFER_DESC constantBufferDesc(sizeof(RenderToolkitStructures::TextureRenderConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
				ThrowIfFailed(
					device->CreateBuffer(
						&constantBufferDesc,
						&constantBufferData,
						m_textureRenderConstantBuffer.ReleaseAndGetAddressOf()
					)
				);
			},
			// Create sampler
			[=]()
			{
				D3D11_SAMPLER_DESC samplerDesc;
				samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
				samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
				samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
				samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
				samplerDesc.MipLODBias = 0.0f;
				samplerDesc.MaxAnisotropy = 1;
				samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
				samplerDesc.BorderColor[0] = 1.0f;
				samplerDesc.BorderColor[1] = 1.0f;
				samplerDesc.BorderColor[2] = 1.0f;
				samplerDesc.BorderColor[3] = 1.0f;
				samplerDesc.MinLOD = -FLT_MAX;
				samplerDesc.MaxLOD = FLT_MAX;

				ThrowIfFailed(
					device->CreateSamplerState(
						&samplerDesc,
						m_textureRenderSampler.ReleaseAndGetAddressOf()
					)
				);
			},
				// Create quad
				[=]()
				{
					static const RenderToolkitStructures::TextureRenderVertex vertices[] =
					{
						{ XMFLOAT2(0.0f, 0.0f) },
						{ XMFLOAT2(1.0f, 0.0f) },
						{ XMFLOAT2(0.0f, 1.0f) },
						{ XMFLOAT2(1.0f, 1.0f) },
					};

					D3D11_SUBRESOURCE_DATA vertexBufferData;
					vertexBufferData.pSysMem = vertices;
					vertexBufferData.SysMemPitch = sizeof(RenderToolkitStructures::TextureRenderVertex);
					vertexBufferData.SysMemSlicePitch = 0;

					CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(vertices), D3D11_BIND_VERTEX_BUFFER);
					ThrowIfFailed(
						device->CreateBuffer(
							&vertexBufferDesc,
							&vertexBufferData,
							m_textureRenderVertexBuffer.ReleaseAndGetAddressOf()
						)
					);
				}
	};

	concurrency::parallel_for_each(work.begin(), work.end(), [](std::function<void()>& f) { f(); });
}

void RenderToolkit::Init_Blur(ID3D11Device* device)
{
	CD3D11_BUFFER_DESC constantBufferDesc(sizeof(RenderToolkitStructures::BlurConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
	ThrowIfFailed(
		device->CreateBuffer(
			&constantBufferDesc,
			nullptr,
			m_blurConstantBuffer.ReleaseAndGetAddressOf()
		)
	);
}

void RenderToolkit::Init_GammaCorrection(ID3D11Device* device)
{
	CD3D11_BUFFER_DESC constantBufferDesc(sizeof(RenderToolkitStructures::GammaCorrectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
	ThrowIfFailed(
		device->CreateBuffer(
			&constantBufferDesc,
			nullptr,
			m_gammaCorrectionConstantBuffer.ReleaseAndGetAddressOf()
		)
	);

	DX::CreateShader(device, m_gammaCorrectionPixelShader.ReleaseAndGetAddressOf(), L"GammaCorrectionPixelShader.cso");
	DX::CreateShader(device, m_scalarGammaCorrectionPixelShader.ReleaseAndGetAddressOf(), L"ScalarGammaCorrectionPixelShader.cso");
}

void RenderToolkit::Init_GlareLens(ID3D11Device* device)
{
	CD3D11_BUFFER_DESC constantBufferDesc(sizeof(RenderToolkitStructures::GlareLensConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
	ThrowIfFailed(
		device->CreateBuffer(
			&constantBufferDesc,
			nullptr,
			m_glareLensConstantBuffer.ReleaseAndGetAddressOf()
		)
	);

	/*CD3D11_BUFFER_DESC constantBufferDesc(sizeof(RenderToolkitStructures::GlareLensConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
	ThrowIfFailed(
		device->CreateBuffer(
			&constantBufferDesc,
			nullptr,
			m_glareLensConstantBuffer.ReleaseAndGetAddressOf()
		)
	);*/

	DX::CreateShader(device, m_glareLensPixelShader.ReleaseAndGetAddressOf(), L"GlareLensPixelShader.cso");
}


void RenderToolkit::Init_ComputeOperations(ID3D11Device* device)
{
	CD3D11_BUFFER_DESC constantBufferDesc(sizeof(RenderToolkitStructures::OperationConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
	ThrowIfFailed(
		device->CreateBuffer(
			&constantBufferDesc,
			nullptr,
			m_operationConstantBuffer.ReleaseAndGetAddressOf()
		)
	);

	std::vector<std::pair<ComPtr<ID3D11ComputeShader>&, std::wstring>> shaders
	{
		{ m_luminanceComputeShader, L"LuminanceComputeShader.cso" },
		{ m_complexMultiplicationComputeShader, L"ComplexMultiplicationComputeShader.cso" },
		{ m_sumComputeShader, L"SumComputeShader.cso" },
		{ m_minComputeShader, L"MinComputeShader.cso" },
		{ m_maxComputeShader, L"MaxComputeShader.cso" },
		{ m_copyComputeShader, L"CopyComputeShader.cso" },
		{ m_constantDivisionComputeShader, L"ConstantDivisionComputeShader.cso" },
		{ m_division1ComputeShader, L"Division1ComputeShader.cso" },
		{ m_scalarDivisionComputeShader, L"ScalarDivisionComputeShader.cso" },
		{ m_additionComputeShader, L"AdditionComputeShader.cso" },
		{ m_constantAdditionComputeShader, L"ConstantAdditionComputeShader.cso" },
		{ m_subtractionComputeShader, L"SubtractionComputeShader.cso" },
		{ m_multiplicationComputeShader, L"MultiplicationComputeShader.cso" },
		{ m_constantMultiplicationComputeShader, L"ConstantMultiplicationComputeShader.cso" },
		{ m_scalarMultiplicationComputeShader, L"ScalarMultiplicationComputeShader.cso" },
		{ m_multiplicationEqualComputeShader, L"MultiplicationEqualComputeShader.cso" },
		{ m_constantMultiplicationEqualComputeShader, L"ConstantMultiplicationEqualComputeShader.cso" },
		{ m_linearCombinationComputeShader, L"LinearCombinationComputeShader.cso" },
		{ m_linearCombinationEqualComputeShader, L"LinearCombinationEqualComputeShader.cso" },
		{ m_lowerBoundComputeShader, L"LowerBoundComputeShader.cso" },
		{ m_upperBoundComputeShader, L"UpperBoundComputeShader.cso" },
		{ m_clampComputeShader, L"ClampComputeShader.cso" },
		{ m_logComputeShader, L"LogComputeShader.cso" },
		{ m_log10ComputeShader, L"Log10ComputeShader.cso" },
		{ m_exp10ComputeShader, L"Exp10ComputeShader.cso" },
		{ m_blurComputeShader, L"BlurComputeShader.cso" },
		{ m_laplaceComputeShader, L"LaplaceComputeShader.cso" },
		{ m_absoluteComputeShader, L"AbsoluteComputeShader.cso" },
		{ m_shiftComputeShader, L"ShiftComputeShader.cso" },
		{ m_colorMapComputeShader, L"ColorMapComputeShader.cso" },
	};

	concurrency::parallel_for_each(shaders.begin(), shaders.end(), [=](std::pair<ComPtr<ID3D11ComputeShader>&, std::wstring>& pair)
	{
		DX::CreateShader(device, pair.first.ReleaseAndGetAddressOf(), pair.second);
	});
}