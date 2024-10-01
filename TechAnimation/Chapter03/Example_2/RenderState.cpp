#include "RenderState.h"

ID3D11RasterizerState* RenderState::mWireframe = 0;

void RenderState::InitAll(ID3D11Device* device)
{
	D3D11_RASTERIZER_DESC wireframeDesc;
	ZeroMemory(&wireframeDesc, sizeof(D3D11_RASTERIZER_DESC));
	wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
	wireframeDesc.CullMode = D3D11_CULL_FRONT;
	wireframeDesc.FrontCounterClockwise = false;
	wireframeDesc.DepthClipEnable = true;
	HRESULT hr = (device->CreateRasterizerState(&wireframeDesc, &mWireframe));
}

void RenderState::DestroyAll()
{
	SAFE_RELEASE(mWireframe);
}
