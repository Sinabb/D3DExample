#pragma once
#include <d3dcompiler.h>
#include "D3DFramework.h"


class DrawTriangle :public D3DFramework
{
	struct VERTEX
	{
		FLOAT x, y, z;
		FLOAT clolor[4];
	};

	Microsoft::WRL::ComPtr<ID3D11InputLayout> mspInputLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> mspVerTexBuffer;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> mspVerTexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> mspPixelShader;

public:
	void Initialize(HINSTANCE hinstance, int width = 800, int Height = 600) override;
	void Destroy() override;

private:
	void InitTriangle();
	void InitPipeline();

protected:
	void Render() override;

};