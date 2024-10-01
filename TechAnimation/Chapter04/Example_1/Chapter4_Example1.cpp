#include <DXUT.h>
#include <SDKmisc.h>
#include <DXUTcamera.h>
#include "Vertex.h"
#include "SkinnedMesh.h"
#include "Common/MathHelper.h"

using namespace DirectX;



//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
CModelViewerCamera g_Camera;
SkinnedModel* g_SkinnedModel;
XMFLOAT4X4 g_World;


ID3D11VertexShader* g_pModelVertexShader = nullptr;
ID3D11PixelShader*	g_pModelPixelShader = nullptr;
ID3D11InputLayout*	g_pModelVertexLayout = nullptr;

ID3D11Buffer*		g_pCBChangesEveryFrame = nullptr;
ID3D11Buffer*		g_pCBSkinned = nullptr;

std::vector<XMFLOAT4X4> currentPose;
XMFLOAT4X4 identity;
XMFLOAT4X4 globalInverseTransform;
XMFLOAT4X4 globalTransform;





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
	// Generate Model
	//
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile("Models/Hip Hop Dancing.dae", aiProcess_Triangulate | aiProcess_GenSmoothNormals);

	g_SkinnedModel = new SkinnedModel(scene);
	XMStoreFloat4x4(&identity, XMMatrixIdentity());
	currentPose.resize(g_SkinnedModel->BoneCount, identity);

	globalTransform = Utils::AssimpToXMFLOAT4X4(scene->mRootNode->mTransformation);
	globalInverseTransform = Utils::AssimpToXMFLOAT4X4(scene->mRootNode->mTransformation);
	XMMATRIX MatrixInverse = XMLoadFloat4x4(&globalInverseTransform);
	XMStoreFloat4x4(&globalInverseTransform, XMMatrixInverse(nullptr, MatrixInverse));


	//
	// Generate Camera
	//
	static const XMVECTORF32 s_Eye = { 0.0f, 0.0f, -10.0f, 0.f };
	static const XMVECTORF32 s_At = { 0.0f, 0.0f, 0.0f, 0.f };
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

	ID3DBlob* pPSBlob = nullptr;
	V_RETURN(DXUTCompileFromFile(L"FX/Mesh.fx", nullptr, "PS", "ps_5_0", dwShaderFlags, 0, &pPSBlob));
	hr = pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &g_pModelPixelShader);
	SAFE_RELEASE(pPSBlob);
	if (FAILED(hr))
		return hr;

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BONEID",  0, DXGI_FORMAT_R8G8B8A8_UINT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BONEWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	hr = pd3dDevice->CreateInputLayout(layout, ARRAYSIZE(layout), pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &g_pModelVertexLayout);
	if (FAILED(hr))
		return hr;
	pd3dImmediateContext->IASetInputLayout(g_pModelVertexLayout);


	// Create the constant buffers
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bd.ByteWidth = sizeof(CBChangesEveryFrame);
	V_RETURN(pd3dDevice->CreateBuffer(&bd, nullptr, &g_pCBChangesEveryFrame));

	D3D11_BUFFER_DESC cbd = {};
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbd.ByteWidth = sizeof(CBSkinned);
	V_RETURN(pd3dDevice->CreateBuffer(&cbd, nullptr, &g_pCBSkinned));




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

	XMMATRIX world = XMMatrixIdentity();
	XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
	XMMATRIX view = g_Camera.GetViewMatrix();
	XMMATRIX proj = g_Camera.GetProjMatrix();
	XMMATRIX worldViewProj = world * view * proj;
	XMMATRIX texCoordTransform = XMMatrixIdentity();

	pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	UINT stride = sizeof(SkinnedVertex::Mesh);
	UINT offset = 0;

	// Update constant buffer
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	pd3dImmediateContext->Map(g_pCBChangesEveryFrame, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	CBChangesEveryFrame* pData = (CBChangesEveryFrame*)mappedResource.pData;
	XMStoreFloat4x4(&pData->mWorld, XMMatrixTranspose(world));
	XMStoreFloat4x4(&pData->mWorldInvTranspose, XMMatrixTranspose(worldInvTranspose));
	XMStoreFloat4x4(&pData->mWorldViewProj, XMMatrixTranspose(worldViewProj));
	XMStoreFloat4x4(&pData->TexcoordTransform, XMMatrixTranspose(texCoordTransform));
	pd3dImmediateContext->Unmap(g_pCBChangesEveryFrame, 0);

	// Update constant buffer
	pd3dImmediateContext->Map(g_pCBSkinned, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	CBSkinned* Data = (CBSkinned*)mappedResource.pData;
	for (int j = 0; j < g_SkinnedModel->BoneCount; j++)
	{
		XMStoreFloat4x4(&Data->gBoneTransforms[j], XMMatrixTranspose(XMLoadFloat4x4(&currentPose[j])));
	}
	pd3dImmediateContext->Unmap(g_pCBSkinned, 0);

	pd3dImmediateContext->IASetInputLayout(g_pModelVertexLayout);
	pd3dImmediateContext->VSSetShader(g_pModelVertexShader, nullptr, 0);
	pd3dImmediateContext->PSSetShader(g_pModelPixelShader, nullptr, 0);
	pd3dImmediateContext->VSSetConstantBuffers(0, 1, &g_pCBChangesEveryFrame);
	pd3dImmediateContext->VSSetConstantBuffers(1, 1, &g_pCBSkinned);

	for (size_t i = 0; i < g_SkinnedModel->g_ModelVertexBuffers.size(); ++i)
	{

		pd3dImmediateContext->IASetVertexBuffers(0, 1, &g_SkinnedModel->g_ModelVertexBuffers[i], &stride, &offset);
		pd3dImmediateContext->IASetIndexBuffer(g_SkinnedModel->g_ModelIndexBuffers[i], DXGI_FORMAT_R32_UINT, 0);

		pd3dImmediateContext->DrawIndexed(g_SkinnedModel->g_ModelIndexCounts[i], 0, 0);
	}
}




//--------------------------------------------------------------------------------------
// Handle updates to the scene.
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove(double fTime, float fElapsedTime, void* pUserContext)
{
	g_Camera.FrameMove(fElapsedTime);
	
	XMMATRIX Rotate = XMMatrixRotationY(45.0f * XMConvertToRadians((float)fTime));
	XMStoreFloat4x4(&g_World, Rotate);
	
	g_SkinnedModel->getPose(g_SkinnedModel->mAnimation, g_SkinnedModel->BoneInfo, fTime * 1000.0f, currentPose, identity, globalInverseTransform);
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

	DXUTCreateWindow(L"Example 4");

	DXUTCreateDevice(D3D_FEATURE_LEVEL_10_0, true, 800, 600);
	DXUTMainLoop();

	return DXUTGetExitCode();
}