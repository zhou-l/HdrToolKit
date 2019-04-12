#include "pch.h"
#include "RenderTexture.h"

#include "RenderToolkit.h"

#include "DirectXHelper.h"

using namespace HdrToolkit;

void IRenderTexture::Create(UINT width, UINT height, DXGI_FORMAT textureFormat, DXGI_FORMAT depthFormat)
{
	m_width = width;
	m_height = height;
	m_textureFormat = textureFormat;
	m_depthFormat = depthFormat;
}

IRenderTexture* IRenderTexture::Get()
{
	return this;
}

ID3D11Texture2D* IRenderTexture::GetTexture()
{
	DX::ThrowIfFailed(E_FAIL);
	return nullptr;
}
ID3D11Texture2D* IRenderTexture::GetDepthStencil()
{
	DX::ThrowIfFailed(E_FAIL);
	return nullptr;
}
ID3D11ShaderResourceView* IRenderTexture::GetShaderResourceView()
{
	DX::ThrowIfFailed(E_FAIL);
	return nullptr;
}
ID3D11RenderTargetView* IRenderTexture::GetRenderTargetView()
{
	DX::ThrowIfFailed(E_FAIL);
	return nullptr;
}
ID3D11UnorderedAccessView* IRenderTexture::GetUnorderedAccessView()
{
	DX::ThrowIfFailed(E_FAIL);
	return nullptr;
}
ID3D11DepthStencilView* IRenderTexture::GetDepthStencilView()
{
	DX::ThrowIfFailed(E_FAIL);
	return nullptr;
}

UINT IRenderTexture::GetWidth() const
{
	return m_width;
}

UINT IRenderTexture::GetHeight() const
{
	return m_height;
}

D3D11_VIEWPORT IRenderTexture::GetFullViewport()
{
	return CD3D11_VIEWPORT(0.0f, 0.0f, static_cast<float>(m_width), static_cast<float>(m_height));
}

DXGI_FORMAT IRenderTexture::GetTextureFormat() const
{
	return m_textureFormat;
}

DXGI_FORMAT IRenderTexture::GetDepthFormat() const
{
	return m_depthFormat;
}

HRESULT SourceRenderTexture::Create(ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT textureFormat, D3D11_SUBRESOURCE_DATA* data)
{
	HRESULT result = S_OK;

	IRenderTexture::Create(width, height, textureFormat, DXGI_FORMAT_UNKNOWN);

	result = RenderToolkitHelper::CreateRenderTexture(device, width, height, textureFormat, data, m_texture.ReleaseAndGetAddressOf(), nullptr, m_shaderResourceView.ReleaseAndGetAddressOf(), nullptr);
	return result;
}

ID3D11Texture2D* SourceRenderTexture::GetTexture()
{
	return m_texture.Get();
}

ID3D11ShaderResourceView* SourceRenderTexture::GetShaderResourceView()
{
	return m_shaderResourceView.Get();
}

HRESULT SourceRenderTexture::Create(ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT textureFormat)
{
	return Create(device, width, height, textureFormat, nullptr);
}

HRESULT SourceRenderTexture::Create(ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT textureFormat, void* data, UINT pitch)
{
	D3D11_SUBRESOURCE_DATA resourceData;
	resourceData.pSysMem = data;
	resourceData.SysMemPitch = pitch;
	resourceData.SysMemSlicePitch = pitch * height;

	return Create(device, width, height, textureFormat, &resourceData);
}

ID3D11Texture2D* TargetRenderTexture::GetTexture()
{
	return m_texture.Get();
}

ID3D11Texture2D* TargetRenderTexture::GetDepthStencil()
{
	return m_depthStencil.Get();
}

ID3D11RenderTargetView* TargetRenderTexture::GetRenderTargetView()
{
	return m_renderTargetView.Get();
}

ID3D11DepthStencilView* TargetRenderTexture::GetDepthStencilView()
{
	return m_depthStencilView.Get();
}

HRESULT TargetRenderTexture::Create(ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT textureFormat, DXGI_FORMAT depthFormat)
{
	HRESULT result = S_OK;

	IRenderTexture::Create(width, height, textureFormat, depthFormat);

	result = RenderToolkitHelper::CreateRenderTexture(device, width, height, textureFormat, nullptr, m_texture.ReleaseAndGetAddressOf(), m_renderTargetView.ReleaseAndGetAddressOf(), nullptr, nullptr);
	if (FAILED(result))
		return result;

	if (depthFormat != DXGI_FORMAT_UNKNOWN)
		result = RenderToolkitHelper::CreateDepthStencil(device, width, height, depthFormat, m_depthStencil.ReleaseAndGetAddressOf(), m_depthStencilView.ReleaseAndGetAddressOf());
	return result;
}

ID3D11ShaderResourceView* SourceViewRenderTexture::GetShaderResourceView()
{
	return *m_otherShaderResourceView;
}

SourceViewRenderTexture::SourceViewRenderTexture(ID3D11ShaderResourceView** shaderResourceView)
	: m_otherShaderResourceView(shaderResourceView)
{
	IRenderTexture::Create(0, 0, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN);
}

ID3D11RenderTargetView* TargetViewRenderTexture::GetRenderTargetView()
{
	return m_otherRenderTargetView.Get();
}

ID3D11DepthStencilView* TargetViewRenderTexture::GetDepthStencilView()
{
	return m_otherDepthStencilView.Get();
}

D3D11_VIEWPORT TargetViewRenderTexture::GetFullViewport()
{
	return m_otherViewport;
}

TargetViewRenderTexture::TargetViewRenderTexture(Microsoft::WRL::ComPtr<ID3D11RenderTargetView>& renderTargetView, Microsoft::WRL::ComPtr<ID3D11DepthStencilView>& depthStencilView, D3D11_VIEWPORT& viewport)
	: m_otherRenderTargetView(renderTargetView)
	, m_otherDepthStencilView(depthStencilView)
	, m_otherViewport(viewport)
{
	IRenderTexture::Create(0, 0, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN);
}

HRESULT RenderTexture::Create(ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT textureFormat, DXGI_FORMAT depthFormat, D3D11_SUBRESOURCE_DATA* data)
{
	HRESULT result = S_OK;

	IRenderTexture::Create(width, height, textureFormat, depthFormat);

	result = RenderToolkitHelper::CreateRenderTexture(device, width, height, textureFormat, data, m_texture.ReleaseAndGetAddressOf(), m_renderTargetView.ReleaseAndGetAddressOf(), m_shaderResourceView.ReleaseAndGetAddressOf(), m_unorderedAccessView.ReleaseAndGetAddressOf());
	if (FAILED(result))
		return result;

	if (depthFormat != DXGI_FORMAT_UNKNOWN)
		result = RenderToolkitHelper::CreateDepthStencil(device, width, height, depthFormat, m_depthStencil.ReleaseAndGetAddressOf(), m_depthStencilView.ReleaseAndGetAddressOf());
	return result;
}

ID3D11Texture2D* RenderTexture::GetTexture()
{
	return m_texture.Get();
}

ID3D11Texture2D* RenderTexture::GetDepthStencil()
{
	return m_depthStencil.Get();
}

ID3D11ShaderResourceView* RenderTexture::GetShaderResourceView()
{
	return m_shaderResourceView.Get();
}

ID3D11RenderTargetView* RenderTexture::GetRenderTargetView()
{
	return m_renderTargetView.Get();
}

ID3D11UnorderedAccessView* RenderTexture::GetUnorderedAccessView()
{
	return m_unorderedAccessView.Get();
}

ID3D11DepthStencilView* RenderTexture::GetDepthStencilView()
{
	return m_depthStencilView.Get();
}

HRESULT RenderTexture::Create(ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT textureFormat, DXGI_FORMAT depthFormat)
{
	return Create(device, width, height, textureFormat, depthFormat, nullptr);
}

HRESULT RenderTexture::Create(ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT textureFormat, DXGI_FORMAT depthFormat, void* data, UINT pitch)
{
	D3D11_SUBRESOURCE_DATA resourceData;
	resourceData.pSysMem = data;
	resourceData.SysMemPitch = pitch;
	resourceData.SysMemSlicePitch = pitch * height;

	return Create(device, width, height, textureFormat, depthFormat, &resourceData);
}

ViewRenderTexture::ViewRenderTexture(ID3D11ShaderResourceView** otherShaderResourceView, ID3D11RenderTargetView** otherRenderTargetView, ID3D11UnorderedAccessView** otherUnorderedAccessView, ID3D11DepthStencilView** otherDepthStencilView)
	: m_otherShaderResourceView(otherShaderResourceView)
	, m_otherRenderTargetView(otherRenderTargetView)
	, m_otherUnorderedAccessView(otherUnorderedAccessView)
	, m_otherDepthStencilView(otherDepthStencilView)
{
}

ViewRenderTexture::ViewRenderTexture(ID3D11ShaderResourceView** otherShaderResourceView, ID3D11RenderTargetView** otherRenderTargetView, ID3D11UnorderedAccessView** otherUnorderedAccessView, ID3D11DepthStencilView** otherDepthStencilView, UINT width, UINT height)
	: ViewRenderTexture(otherShaderResourceView, otherRenderTargetView, otherUnorderedAccessView, otherDepthStencilView)
{
	IRenderTexture::Create(width, height, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN);
}

ID3D11ShaderResourceView* ViewRenderTexture::GetShaderResourceView()
{
	return *m_otherShaderResourceView;
}

ID3D11RenderTargetView* ViewRenderTexture::GetRenderTargetView()
{
	return *m_otherRenderTargetView;
}

ID3D11UnorderedAccessView* ViewRenderTexture::GetUnorderedAccessView()
{
	return *m_otherUnorderedAccessView;
}

ID3D11DepthStencilView* ViewRenderTexture::GetDepthStencilView()
{
	return *m_otherDepthStencilView;
}