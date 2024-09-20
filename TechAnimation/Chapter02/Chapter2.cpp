#include <DXUT.h>
#include <SDKmisc.h>
#include <DXUTcamera.h>
#include "Vertex.h"
#include "MeshGenerate.h"



//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
struct CBChangesEveryFrame
{
	XMFLOAT4X4 mWorldViewProj;
	XMFLOAT4X4 mWorld;
};



CModelViewerCamera g_Camera;
MeshGenerator* g_Model;



ID3D11VertexShader* g_pVertexShader = nullptr;
ID3D11PixelShader*	g_pPixelShader = nullptr;
ID3D11InputLayout*	g_pVertexLayout = nullptr;
ID3D11Buffer*		g_pCBChangesEveryFrame = nullptr;


//--------------------------------------------------------------------------------------
// Function Helper
//--------------------------------------------------------------------------------------






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



	//
	//
	//
	auto pd3dImmediateContext = DXUTGetD3D11DeviceContext();

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	dwShaderFlags |= D3DCOMPILE_DEBUG;
	dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	// Compile the vertex shader
	ID3DBlob* pVSBlob = nullptr;
	V_RETURN(DXUTCompileFromFile(L"FX/FX.fx", nullptr, "VS", "vs_5_0", dwShaderFlags, 0, &pVSBlob));

	// Create the vertex shader
	hr = pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &g_pVertexShader);
	if (FAILED(hr))
	{
		SAFE_RELEASE(pVSBlob);
		return hr;
	}

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(layout);

	// Create the input layout
	hr = pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &g_pVertexLayout);
	SAFE_RELEASE(pVSBlob);
	if (FAILED(hr))
		return hr;

	// Set the input layout
	pd3dImmediateContext->IASetInputLayout(g_pVertexLayout);

	// Compile the pixel shader
	ID3DBlob* pPSBlob = nullptr;
	V_RETURN(DXUTCompileFromFile(L"FX/FX.fx", nullptr, "PS", "ps_5_0", dwShaderFlags, 0, &pPSBlob));

	// Create the pixel shader
	hr = pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &g_pPixelShader);
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
	//
	//
	//



	g_Model = new MeshGenerator(pd3dDevice, "Models/solider.obj");

	return S_OK;
}





// --------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext,
	double fTime, float fElapsedTime, void* pUserContext)
{
	auto pRTV = DXUTGetD3D11RenderTargetView();
	pd3dImmediateContext->ClearRenderTargetView(pRTV, Colors::MidnightBlue);

	auto pDSV = DXUTGetD3D11DepthStencilView();
	pd3dImmediateContext->ClearDepthStencilView(pDSV, D3D11_CLEAR_DEPTH, 1.0, 0);
	
	pd3dImmediateContext->IASetInputLayout(g_pVertexLayout);
	pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	XMMATRIX world = XMMatrixIdentity();
	XMMATRIX view = g_Camera.GetViewMatrix();
	XMMATRIX proj = g_Camera.GetProjMatrix();
	XMMATRIX mWorldViewProjection = world * view * proj;

	UINT stride = sizeof(Vertex::Basic32);
	UINT offset = 0;

	pd3dImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
	pd3dImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);
	pd3dImmediateContext->VSSetConstantBuffers(0, 1, &g_pCBChangesEveryFrame);
	pd3dImmediateContext->PSSetConstantBuffers(0, 1, &g_pCBChangesEveryFrame);



	for (size_t i = 0; i < g_Model->GetVertexBuffer().size(); ++i)
	{
		D3D11_MAPPED_SUBRESOURCE MappedResource;
		HRESULT hr = pd3dImmediateContext->Map(g_pCBChangesEveryFrame, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
		auto pCB = reinterpret_cast<CBChangesEveryFrame*>(MappedResource.pData);
		XMStoreFloat4x4(&pCB->mWorldViewProj, XMMatrixTranspose(mWorldViewProjection));
		XMStoreFloat4x4(&pCB->mWorld, XMMatrixTranspose(world));
		pd3dImmediateContext->Unmap(g_pCBChangesEveryFrame, 0);

		g_Model->Draw(pd3dImmediateContext, stride, offset, i);
	}


}




//--------------------------------------------------------------------------------------
// Handle updates to the scene.
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove(double fTime, float fElapsedTime, void* pUserContext)
{
	g_Camera.FrameMove(fElapsedTime);
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

	DXUTCreateWindow(L"Chapter2");

	DXUTCreateDevice(D3D_FEATURE_LEVEL_10_0, true, 800, 600);
	DXUTMainLoop();

	return DXUTGetExitCode();
}