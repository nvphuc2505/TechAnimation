﻿#include <DXUT.h>
#include <SDKmisc.h>
#include <DXUTcamera.h>
#include "Vertex.h"
#include "SkinnedMesh.h"
#include "RenderState.h"
#include "Common/MathHelper.h"
using namespace DirectX;



//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
CModelViewerCamera g_Camera;
SkinnedMesh* g_SkinnedMesh;
XMFLOAT4X4 g_World;


ID3D11VertexShader* g_pModelVertexShader = nullptr;
ID3D11PixelShader*	g_pModelPixelShader = nullptr;
ID3D11InputLayout*	g_pModelVertexLayout = nullptr;

ID3D11VertexShader* g_pBoneVertexShader = nullptr;
ID3D11PixelShader*  g_pBonePixelShader = nullptr;
ID3D11InputLayout*  g_pBoneVertexLayout = nullptr;

ID3D11Buffer*		g_pCBChangesEveryFrame = nullptr;






//--------------------------------------------------------------------------------------
// Reject any D3D11 devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D11DeviceAcceptable(const CD3D11EnumAdapterInfo* AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo* DeviceInfo,
	DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext)
{
	return true;
}





//--------------------------------------------------------------------------------------
// Called right before creating a D3D device, allowing the app to modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings(DXUTDeviceSettings* pDeviceSettings, void* pUserContext)
{
	return true;
}





//--------------------------------------------------------------------------------------
// Create any D3D11 resources that aren't dependant on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
	void* pUserContext)
{
	HRESULT hr = S_OK;

	RenderState::InitAll(pd3dDevice);

	//
	// Generate Model
	//
	g_SkinnedMesh = new SkinnedMesh(pd3dDevice, "Models/boy.dae");


	//
	// Generate Camera
	//
	static const XMVECTORF32 s_Eye = { 0.0f, 3.0f, -10.0f, 0.f };
	static const XMVECTORF32 s_At = { 0.0f, 3.0f, 0.0f, 0.f };
	g_Camera.SetViewParams(s_Eye, s_At);


	//
	// Generate Shader 
	//
	auto pd3dImmediateContext = DXUTGetD3D11DeviceContext();

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	dwShaderFlags |= D3DCOMPILE_DEBUG;
	dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	// Compile the vertex shader
	ID3DBlob* pVSBlob = nullptr;
	V_RETURN(DXUTCompileFromFile(L"FX/Mesh.fx", nullptr, "VS", "vs_5_0", dwShaderFlags, 0, &pVSBlob));
	// Create the vertex shader
	hr = pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &g_pModelVertexShader);
	if (FAILED(hr))
	{
		SAFE_RELEASE(pVSBlob);
		return hr;
	}

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	hr = pd3dDevice->CreateInputLayout(layout, ARRAYSIZE(layout), pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &g_pModelVertexLayout);
	// SAFE_RELEASE(pVSBlob);
	if (FAILED(hr))
		return hr;
	// Set the input layout
	pd3dImmediateContext->IASetInputLayout(g_pModelVertexLayout);

	// Compile the pixel shader
	ID3DBlob* pPSBlob = nullptr;
	V_RETURN(DXUTCompileFromFile(L"FX/Mesh.fx", nullptr, "PS", "ps_5_0", dwShaderFlags, 0, &pPSBlob));
	// Create the pixel shader
	hr = pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &g_pModelPixelShader);
	SAFE_RELEASE(pPSBlob);
	if (FAILED(hr))
		return hr;



	// Compile the vertex shader
	// ID3DBlob* pVSBlob = nullptr;
	V_RETURN(DXUTCompileFromFile(L"FX/Line.fx", nullptr, "VS", "vs_5_0", dwShaderFlags, 0, &pVSBlob));
	// Create the vertex shader
	hr = pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &g_pBoneVertexShader);
	if (FAILED(hr))
	{
		SAFE_RELEASE(pVSBlob);
		return hr;
	}

	D3D11_INPUT_ELEMENT_DESC boneLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	hr = pd3dDevice->CreateInputLayout(boneLayout, ARRAYSIZE(boneLayout), pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &g_pBoneVertexLayout);
	SAFE_RELEASE(pVSBlob);
	if (FAILED(hr))
		return hr;
	// Set the input layout
	pd3dImmediateContext->IASetInputLayout(g_pBoneVertexLayout);

	// Compile the pixel shader
	// ID3DBlob* pPSBlob = nullptr;
	V_RETURN(DXUTCompileFromFile(L"FX/Line.fx", nullptr, "PS", "ps_5_0", dwShaderFlags, 0, &pPSBlob));
	// Create the pixel shader
	hr = pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &g_pBonePixelShader);
	SAFE_RELEASE(pPSBlob);
	if (FAILED(hr))
		return hr;





	// Create the constant buffers
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bd.ByteWidth = sizeof(CBChangesEveryFrame);
	V_RETURN(pd3dDevice->CreateBuffer(&bd, nullptr, &g_pCBChangesEveryFrame));
	

	return S_OK;
}





// --------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext,
	double fTime, float fElapsedTime, void* pUserContext)
{
	auto pRTV = DXUTGetD3D11RenderTargetView();
	pd3dImmediateContext->ClearRenderTargetView(pRTV, Colors::Silver);

	auto pDSV = DXUTGetD3D11DepthStencilView();
	pd3dImmediateContext->ClearDepthStencilView(pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0, 0);
	
	
	XMMATRIX world = XMLoadFloat4x4(&g_World);
	XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
	XMMATRIX view = g_Camera.GetViewMatrix();
	XMMATRIX proj = g_Camera.GetProjMatrix();
	XMMATRIX worldViewProj = world * view * proj;
	XMMATRIX texCoordTransform = XMMatrixIdentity();

	// Update Constant Buffer
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	pd3dImmediateContext->Map(g_pCBChangesEveryFrame, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	CBChangesEveryFrame* pData = (CBChangesEveryFrame*)mappedResource.pData;
	XMStoreFloat4x4(&pData->mWorld, XMMatrixTranspose(world));
	XMStoreFloat4x4(&pData->mWorldInvTranspose, XMMatrixTranspose(worldInvTranspose));
	XMStoreFloat4x4(&pData->mWorldViewProj, XMMatrixTranspose(worldViewProj));
	XMStoreFloat4x4(&pData->TexcoordTransform, XMMatrixTranspose(texCoordTransform));
	pd3dImmediateContext->Unmap(g_pCBChangesEveryFrame, 0);


	pd3dImmediateContext->IASetInputLayout(g_pBoneVertexLayout);
	pd3dImmediateContext->VSSetShader(g_pBoneVertexShader, nullptr, 0);
	pd3dImmediateContext->VSSetConstantBuffers(0, 1, &g_pCBChangesEveryFrame);
	pd3dImmediateContext->PSSetShader(g_pBonePixelShader, nullptr, 0);
	pd3dImmediateContext->PSSetConstantBuffers(0, 1, &g_pCBChangesEveryFrame);
	g_SkinnedMesh->RenderSkeleton(pd3dDevice, pd3dImmediateContext, nullptr, nullptr, world);



	world = XMMatrixRotationX(XMConvertToRadians(-90.0f)) * world;
	worldInvTranspose = MathHelper::InverseTranspose(world);
	view = g_Camera.GetViewMatrix();
	proj = g_Camera.GetProjMatrix();
	worldViewProj = world * view * proj;
	texCoordTransform = XMMatrixIdentity();

	// Update Constant Buffer
	pd3dImmediateContext->Map(g_pCBChangesEveryFrame, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	pData = (CBChangesEveryFrame*)mappedResource.pData;
	XMStoreFloat4x4(&pData->mWorld, XMMatrixTranspose(world));
	XMStoreFloat4x4(&pData->mWorldInvTranspose, XMMatrixTranspose(worldInvTranspose));
	XMStoreFloat4x4(&pData->mWorldViewProj, XMMatrixTranspose(worldViewProj));
	XMStoreFloat4x4(&pData->TexcoordTransform, XMMatrixTranspose(texCoordTransform));
	pd3dImmediateContext->Unmap(g_pCBChangesEveryFrame, 0);


	pd3dImmediateContext->IASetInputLayout(g_pModelVertexLayout);
	pd3dImmediateContext->VSSetShader(g_pModelVertexShader, nullptr, 0);
	pd3dImmediateContext->VSSetConstantBuffers(0, 1, &g_pCBChangesEveryFrame);
	pd3dImmediateContext->PSSetShader(g_pModelPixelShader, nullptr, 0);
	pd3dImmediateContext->PSSetConstantBuffers(0, 1, &g_pCBChangesEveryFrame);
	g_SkinnedMesh->Render(pd3dDevice, pd3dImmediateContext);
}




//--------------------------------------------------------------------------------------
// Handle updates to the scene.
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove(double fTime, float fElapsedTime, void* pUserContext)
{
	g_Camera.FrameMove(fElapsedTime);
	
	XMMATRIX Rotate = XMMatrixRotationY(45.0f * XMConvertToRadians((float)fTime));
	XMStoreFloat4x4(&g_World, Rotate);
	
}

//--------------------------------------------------------------------------------------
	// Release D3D11 resources created in OnD3D11CreateDevice 
	//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice(void* pUserContext)
{

}

//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain(void* pUserContext)
{

}

//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
	bool* pbNoFurtherProcessing, void* pUserContext)
{
	g_Camera.HandleMessages(hWnd, uMsg, wParam, lParam);

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK OnKeyboard(UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext)
{
	if (bKeyDown)
	{
		switch (nChar)
		{
		case VK_F1: // Change as needed                
			break;
		}
	}


}

//--------------------------------------------------------------------------------------
// Call if device was removed.  Return true to find a new device, false to quit
//--------------------------------------------------------------------------------------
bool CALLBACK OnDeviceRemoved(void* pUserContext)
{
	return true;
}





//--------------------------------------------------------------------------------------
// Create any D3D11 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext)
{
	HRESULT hr = S_OK;

	float fNearPlane = 0.1f;
	float fFarPlane = 1000.0f;
	float fFOV = XM_PI / 4;
	float fAspect = static_cast<float>(pBackBufferSurfaceDesc->Width) / static_cast<float>(pBackBufferSurfaceDesc->Height);

	g_Camera.SetProjParams(fFOV, fAspect, fNearPlane, fFarPlane);
	g_Camera.SetWindow(pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height);

	return S_OK;
}





//--------------------------------------------------------------------------------------
// Initialize everything and go into a render loop
//--------------------------------------------------------------------------------------
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif


	DXUTSetCallbackFrameMove(OnFrameMove);
	DXUTSetCallbackKeyboard(OnKeyboard);
	DXUTSetCallbackMsgProc(MsgProc);
	DXUTSetCallbackDeviceChanging(ModifyDeviceSettings);
	DXUTSetCallbackDeviceRemoved(OnDeviceRemoved);
	DXUTSetCallbackD3D11DeviceAcceptable(IsD3D11DeviceAcceptable);
	DXUTSetCallbackD3D11DeviceCreated(OnD3D11CreateDevice);
	DXUTSetCallbackD3D11SwapChainResized(OnD3D11ResizedSwapChain);
	DXUTSetCallbackD3D11FrameRender(OnD3D11FrameRender);
	DXUTSetCallbackD3D11SwapChainReleasing(OnD3D11ReleasingSwapChain);
	DXUTSetCallbackD3D11DeviceDestroyed(OnD3D11DestroyDevice);

	DXUTInit(true, true, nullptr);
	DXUTSetCursorSettings(true, true);

	DXUTCreateWindow(L"Example 3_1");

	DXUTCreateDevice(D3D_FEATURE_LEVEL_10_0, true, 800, 600);
	DXUTMainLoop();

	return DXUTGetExitCode();
}