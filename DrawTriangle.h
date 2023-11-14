#pragma once
#include <d3dcompiler.h>
#include "D3DFramework.h"


class DrawTriangle :public D3DFramework
{
	struct VERTEX
	{
		FLOAT x, y, z;
		FLOAT U,V;
	};

	Microsoft::WRL::ComPtr<ID3D11InputLayout> mspInputLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> mspVerTexBuffer;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> mspVerTexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> mspPixelShader;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> mspTexture; // resource
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mspTextureView;

	Microsoft::WRL::ComPtr<ID3D11SamplerState> mspSamplerState;
	Microsoft::WRL::ComPtr<ID3D11BlendState> mspBlendState;


public:
	void Initialize(HINSTANCE hinstance, int width = 800, int Height = 600) override;
	void Destroy() override;

private:
	void InitTriangle();
	void InitPipeline();

	HRESULT CreateTextureFormBMP();

protected:
	void Render() override;
};