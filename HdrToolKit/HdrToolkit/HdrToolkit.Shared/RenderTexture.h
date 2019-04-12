#pragma once
#include <string>
namespace HdrToolkit
{
	class IRenderTexture
	{
	protected:
		UINT m_width;
		UINT m_height;
		DXGI_FORMAT m_textureFormat;
		DXGI_FORMAT m_depthFormat;

		void Create(UINT width, UINT height, DXGI_FORMAT textureFormat, DXGI_FORMAT depthFormat);

	public:
		IRenderTexture* Get();

		virtual ID3D11Texture2D* GetTexture();
		virtual ID3D11Texture2D* GetDepthStencil();
		virtual ID3D11ShaderResourceView* GetShaderResourceView();
		virtual ID3D11RenderTargetView* GetRenderTargetView();
		virtual ID3D11UnorderedAccessView* GetUnorderedAccessView();
		virtual ID3D11DepthStencilView* GetDepthStencilView();
		UINT GetWidth() const;
		UINT GetHeight() const;
		virtual D3D11_VIEWPORT GetFullViewport();
		DXGI_FORMAT GetTextureFormat() const;
		DXGI_FORMAT GetDepthFormat() const;
	};

	class SourceRenderTexture : public IRenderTexture
	{
	private:
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shaderResourceView;

		HRESULT Create(ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT textureFormat, D3D11_SUBRESOURCE_DATA* data);

	public:
		virtual ID3D11Texture2D* GetTexture() override;
		virtual ID3D11ShaderResourceView* GetShaderResourceView() override;

		HRESULT Create(ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT textureFormat);
		HRESULT Create(ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT textureFormat, void* data, UINT pitch);
	};

	class TargetRenderTexture : public IRenderTexture
	{
	private:
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_depthStencil;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthStencilView;

	public:
		virtual ID3D11Texture2D* GetTexture() override;
		virtual ID3D11Texture2D* GetDepthStencil() override;
		virtual ID3D11RenderTargetView* GetRenderTargetView() override;
		virtual ID3D11DepthStencilView* GetDepthStencilView() override;

		HRESULT Create(ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT textureFormat, DXGI_FORMAT depthFormat);
	};

	class SourceViewRenderTexture : public IRenderTexture
	{
	private:
		ID3D11ShaderResourceView** m_otherShaderResourceView;

	public:
		virtual ID3D11ShaderResourceView* GetShaderResourceView() override;

		SourceViewRenderTexture(ID3D11ShaderResourceView** shaderResourceView);
	};

	class TargetViewRenderTexture : public IRenderTexture
	{
	private:
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView>& m_otherRenderTargetView;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView>& m_otherDepthStencilView;
		D3D11_VIEWPORT& m_otherViewport;

	public:
		virtual ID3D11RenderTargetView* GetRenderTargetView() override;
		virtual ID3D11DepthStencilView* GetDepthStencilView() override;

		virtual D3D11_VIEWPORT GetFullViewport() override;

		TargetViewRenderTexture(Microsoft::WRL::ComPtr<ID3D11RenderTargetView>& renderTargetView, Microsoft::WRL::ComPtr<ID3D11DepthStencilView>& depthStencilView, D3D11_VIEWPORT& viewport);
	};

	class RenderTexture : public IRenderTexture
	{
	private:
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_depthStencil;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shaderResourceView;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;
		Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_unorderedAccessView;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthStencilView;

		HRESULT Create(ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT textureFormat, DXGI_FORMAT depthFormat, D3D11_SUBRESOURCE_DATA* data);

		HRESULT Write(const std::string& filename);

	public:
		virtual ID3D11Texture2D* GetTexture() override;
		virtual ID3D11Texture2D* GetDepthStencil() override;
		virtual ID3D11ShaderResourceView* GetShaderResourceView() override;
		virtual ID3D11RenderTargetView* GetRenderTargetView() override;
		virtual ID3D11UnorderedAccessView* GetUnorderedAccessView() override;
		virtual ID3D11DepthStencilView* GetDepthStencilView() override;

		HRESULT Create(ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT textureFormat, DXGI_FORMAT depthFormat = DXGI_FORMAT_UNKNOWN);
		HRESULT Create(ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT textureFormat, DXGI_FORMAT depthFormat, void* data, UINT pitch);
	};

	class ViewRenderTexture : public IRenderTexture
	{
	private:
		ID3D11ShaderResourceView** m_otherShaderResourceView;
		ID3D11RenderTargetView** m_otherRenderTargetView;
		ID3D11UnorderedAccessView** m_otherUnorderedAccessView;
		ID3D11DepthStencilView** m_otherDepthStencilView;

	public:
		ViewRenderTexture(ID3D11ShaderResourceView** otherShaderResourceView, ID3D11RenderTargetView** otherRenderTargetView, ID3D11UnorderedAccessView** otherUnorderedAccessView, ID3D11DepthStencilView** otherDepthStencilView);
		ViewRenderTexture(ID3D11ShaderResourceView** otherShaderResourceView, ID3D11RenderTargetView** otherRenderTargetView, ID3D11UnorderedAccessView** otherUnorderedAccessView, ID3D11DepthStencilView** otherDepthStencilView, UINT width, UINT height);

		virtual ID3D11ShaderResourceView* GetShaderResourceView() override;
		virtual ID3D11RenderTargetView* GetRenderTargetView() override;
		virtual ID3D11UnorderedAccessView* GetUnorderedAccessView() override;
		virtual ID3D11DepthStencilView* GetDepthStencilView() override;
	};
}
