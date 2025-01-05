#include <Windows.h>  // Windows API �Լ��� �ڷ����� ����ϱ� ���� ��� ����
#include <tchar.h>    // �����ڵ� �� ��Ƽ����Ʈ ���� ������ ���� ���
#include <d3d12.h>
#include <dxgi1_6.h>
#include <vector>
#include <string>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#ifdef _DEBUG
#include <iostream>   // ����� ��忡�� ǥ�� ������� ����ϱ� ���� ���
#endif
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
using namespace std;  // std ���ӽ����̽��� ����Ͽ� �ڵ� ������ ���
using namespace DirectX;

// �ܼ� ȭ�鿡 ������ ������ ���ڿ��� ����ϴ� ����� ���� �Լ�
// ����� ���¿����� �۵��ϸ�, ���ڿ� ������ �޾� �����
void DebugOutputFormatString(const char* format, ...)
{
#ifdef _DEBUG
	va_list valist;  // ���� �μ� ����� �����ϱ� ���� ��ü ����
	va_start(valist, format);  // ���� �μ� ��� �ʱ�ȭ (format ���� �μ����� ����)
	vprintf(format, valist);  // ������ ������� ���� �μ� ��� ���
	va_end(valist);  // ���� �μ� ��� ����
#endif
}
//�������� �ݵ�� ����ϴ� �Լ�
LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM Iparam)
{
	//�����찡 ���Ǿ��ٸ� �ҷ�����
	if (msg == WM_DESTROY)
	{
		PostQuitMessage(0); //OS�� ���� [���� �� ���ø��� ������]�� �����Ѵ�
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, Iparam); //������ ó���� �մϴ�.
}
//ȭ�� ũ��
const unsigned int		window_width = 1920;
const unsigned int		window_height = 1080;

ID3D12Device* _dev = nullptr;
IDXGIFactory6* _dxgiFactory = nullptr;
IDXGISwapChain4* _swapchain = nullptr;
ID3D12CommandAllocator* _cmdAllocator = nullptr;
ID3D12CommandQueue* _cmdQueue = nullptr;
ID3D12GraphicsCommandList* _cmdList = nullptr;

void EnableDebugLayer()
{
	ID3D12Debug* debugLayer = nullptr;
	auto result = D3D12GetDebugInterface(
		IID_PPV_ARGS(&debugLayer));

	debugLayer->EnableDebugLayer(); //����׷��̾ ��ȿȭ�Ѵ�
	debugLayer->Release(); //��ȿȭ�ߴٸ� �������̽��� �����Ѵ�.
}

#ifdef _DEBUG
int main()  // ����� ��忡�� ����Ǵ� main �Լ� (�ܼ� ��� ���ø����̼�)
{
#else
#include <Windows.h>
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)  // ������ ��忡�� ����Ǵ� WinMain �Լ� (������ GUI ���ø����̼�)
{
#endif
	DebugOutputFormatString("Welcome to DirectX12");  // "Show window test."��� ���ڿ��� ����� ���¿��� �ֿܼ� ���
	WNDCLASSEX w = {};

	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProcedure; //�ݹ��Լ��� ����
	w.lpszClassName = _T("DX12Sample"); //���ø����̼� Ŭ���� �̸�(������ ����)
	w.hInstance = GetModuleHandle(nullptr); // �ڵ��� ���
	RegisterClassEx(&w); //���ø����̼� Ŭ����(������ Ŭ������ ������ OS�� ����)

	RECT wrc = { 0, 0, window_width, window_height }; //������ ����� ���մϴ�

	//�Լ��� ����ؼ� �������� ����� �����մϴ�
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	//�����������Ʈ�� ����
	HWND hwnd = CreateWindow(w.lpszClassName, //Ŭ������ ����
		_T("DX12 �׽�Ʈ"), //Ÿ��Ʋ ���� ����
		WS_OVERLAPPEDWINDOW, //Ÿ��Ʋ�ٿ� ��輱�� �ִ� ������
		CW_USEDEFAULT, //ǥ�� x��ǥ�� OS�� �ñ�ϴ�
		CW_USEDEFAULT, //ǥ�� y��ǥ�� OS�� �ñ�ϴ�
		wrc.right - wrc.left, //������ ��width
		wrc.bottom - wrc.top, //������ ����
		nullptr, //���ο� ������ �ڵ�
		nullptr, //�޴��ڵ�
		w.hInstance, //�ҷ��� ���ø����̼� �ڵ�
		nullptr); //�߰��Ķ����

#ifdef _DEBUG
	//����׷��̾ ON����
	EnableDebugLayer();
#endif
	//���� DirectX12 �ֺ��� �ʱ�ȭ

	//��ó���� ����
	D3D_FEATURE_LEVEL levels[] =
	{
		D3D_FEATURE_LEVEL_12_2,
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};
	HRESULT result = S_OK;
#ifdef _DEBUG
	CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&_dxgiFactory));
#else
	CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory));
#endif
	if (FAILED(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&_dxgiFactory)))) {
		if (FAILED(CreateDXGIFactory2(0, IID_PPV_ARGS(&_dxgiFactory)))) {
			return -1;
		}
	}
	//HRESULT CreateSwapChainForHwnd(
	//	IUnknown * pDevice, //Ŀ�ǵ�ť������Ʈ
	//	HWND hWnd,		//�������ڵ�
	//	const DXGI_SWAP_CHAIN_DESC * pDesc,		//����ü�μ���
	//	const DXGI_SWAP_CHAIN_FULLSCREEN_DESC * pFullscreenDesc, //�켱 nullptr���� 
	//	IDXGIOutput * pRestrictToOutput,							//�̰͵� nullptr
	//	IDXGISwapChain1 * *ppSwapChain							//����ü�� ������Ʈ ����
	//);

	//typedef struct DXGI_SWAP_CHAIN_DESC1
	//{
	//	UINT	Width; //ȭ���ػ� (��)
	//	UINT	Height; //ȭ���ػ� (����)
	//	DXGI_FORMAT Format; //�ȼ�����
	//	BOOL Stereo; //���׷���ǥ���÷��� (3D���÷����� ���׷��� ���)
	//	DXGI_SAMPLE_DESC SampleDesc; //��Ƽ������ ����(Count = 1, Quality = 0���� �����ϴ�)
	//	DXGI_USAGE BufferUsage; //DXGI_USAGE_BACK_BUFFER ����
	//	UINT BufferCount; // ������۶�� 2�� ����
	//	DXGI_SCALING Scaling; //DXGI_SCALING_STRETCH ����
	//	DXGI_SWAP_EFFECT SwapEffect; //DXGI_SWAP_EFFECT_FLIP_DISCARD ����
	//	DXGI_ALPHA_MODE AlphaMode; //DXGI_ALPHA_MODE_UNSPECIFIED ����
	//	UINT Flags; //DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH ����
	//}DXGI_SWAP_CHAIN_DESC1;
	//void  CreateRenderTargetView(
	//	ID3D12Resource * pResource, //����
	//	const D3D12_RENDER_TARGET_VIEW_DESC * pDesc, //�̹��� nullptr�� �����ϴ�
	//	D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor //��ũ�������ڵ�
	//);
	//void OMSetRenderTargets(
	//	UINT numRTVDescriptors, //����Ÿ�ټ� (1�� �����ϴ�)
	//	const D3D12_CPU_DESCRIPTOR_HANDLE * pRTVHandles, //����Ÿ���ѵ���
	//	//ó�� �ּ�
	//	BOOL RTsSingleHandleToDescriptorRange, //�������� �����ϰ��ִ°�
	//	const D3D12_CPU_DESCRIPTOR_HANDLE //�ɵ����ٽǹ��ۺ���
	//	* pDepthStencilDescriptor			//�ڵ�(nullptr�� �����ϴ�)
	//);
	//void ExecuteCommandLists(
	//	UINT NumCommandLists, //�����ϴ� Ŀ�ǵ帮��Ʈ��(�ϳ��� �����ϴ�)
	//	ID3D12CommandList* const* ppCommandLists //Ŀ�ǵ帮��Ʈ �迭�� ó�� �ּ�
	//);
	//HRESULT CreateFence(
	//	UINT64 InitialValue, //�ʱ�ȭ ��ġ(0)
	//	D3D12_FENCE_FLAGS Flags, //�켱 D3D12_FENCE_FLAG_NONE
	//	REFIID riid,
	//	void** ppFence
	//);
	//HRESULT Signal(
	//	ID3D12Fence * pFence, //�Ʊ� ���� �潺������Ʈ
	//	UINT64 Value //GPU�� ó���� �Ϸ��� �ڿ� �Ǿ��־�߸� �ϴ� ��ġ(�潺ġ)
	//);
	//HRESULT SetEventOnCompletion(
	//	UINT64 Value, //�� ��ġ�� �ƴٸ� �̺�Ʈ�� �߻���Ų��
	//	HANDLE hEvent //�߻���Ű�� �̺�Ʈ
	//);
	//void ResourceBarrier(
	//	UINT NumBarriers, //�����踮���� ��(������������ 1��)
	//	const D3D12_RESOURCE_BARRIER * pBarriers //�����踮�� ����ü �ּ�
	//
	//);
	//typedef struct D3D12_RESOURCE_BARRIER
	//{
	//	D3D12_RESOURCE_BARRIER_TYPE Type; //�踮���� ���� (���������̹Ƿ� TRANSITION)
	//	D3D12_RESOURCE_BARRIER_FLAGS Flags; //Ư���� ���� ���ϹǷ� FLAG_NONE���� �����ϴ�
	//
	//	union //����ü�� ������ ����
	//	{
	//		D3D12_RESOURCE_TRANSITION_BARRIER Transition; //���⸦ ���
	//		D3D12_RESOURCE_ALIASING_BARRIER Aliasing;
	//		D3D12_RESOURCE_UAV_BARRIER UAV;
	//	};
	//} D3D12_RESOURCE_BARRIER;
	//
	//typedef struct D3D12_RESOURCE_TRANSITION_BARRIER
	//{
	//	ID3D12Resource* pResource; //���ҽ��� �ּ�
	//	UINT Subresource; //���긮�ҽ���ȣ (0���� �����ϴ�)
	//	D3D12_RESOURCE_STATES StateBefore; //������ ����
	//	D3D12_RESOURCE_STATES StateAfter; //������ ����
	//} D3D12_RESOURCE_TRANSITION_BARRIER;
	//union
	//{
	//	D3D12_RESOURCE_TRANSITION_BARRIER Transition; //���⸦ ���
	//	D3D12_RESOURCE_ALIASING_BARRIER Aliasing;
	//	D3D12_RESOURCE_UAV_BARRIER UAV;
	//};
	//typedef struct D3D12_RESOURCE_TRANSITION_BARRIER
	//{
	//	ID3D12Resource* pResource;
	//	UINT Subresource;
	//	D3D12_RESOURCE_STATES StateBefore;
	//	D3D12_RESOURCE_STATES StateAfter;
	//}D3D12_RESOURCE_TRANSITION_BARRIER;
	//typedef struct D3D12_RESOURCE_ALIASING_BARRIER
	//{
	//	ID3D12Resource* pResourceBefore;
	//	ID3D12Resource* pResourceAfter;
	//} 	D3D12_RESOURCE_ALIASING_BARRIER;
	//
	//typedef struct D3D12_RESOURCE_UAV_BARRIER
	//{
	//	ID3D12Resource* pResource;
	//} 	D3D12_RESOURCE_UAV_BARRIER;
	// virtual HRESULT STDMETHODCALLTYPE CreateCommittedResource( 
	//_In_  const D3D12_HEAP_PROPERTIES* pHeapProperties, //����������ü�� �ּ�
	//D3D12_HEAP_FLAGS HeapFlags,//Ư�� �������� �ʱ� ������ d3d12_heap_flag_none���� 
	//_In_  const D3D12_RESOURCE_DESC* pDesc,//���ҽ���������ü�ּ�
	//D3D12_RESOURCE_STATES InitialResourceState,//GPU���� �ҷ����� �������� GENERIC_READ
	//_In_opt_  const D3D12_CLEAR_VALUE* pOptimizedClearValue, //������� �ʱ� ������ nullptr
	//REFIID riidResource,
	//_COM_Outptr_opt_  void** ppvResource) = 0;

	//������� ���ſ�
	std::vector <IDXGIAdapter*> adapters;
	IDXGIAdapter* tmpAdapter = nullptr;
	for (int i = 0; _dxgiFactory->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND; ++i) {
		adapters.push_back(tmpAdapter);
	}
	for (auto adpt : adapters) {
		DXGI_ADAPTER_DESC adesc = {};
		adpt->GetDesc(&adesc);
		std::wstring strDesc = adesc.Description;
		if (strDesc.find(L"NVIDIA") != std::string::npos) {
			tmpAdapter = adpt;
			break;
		}
	}

	//Direct3D����̽��� �ʱ�ȭ
	D3D_FEATURE_LEVEL featureLevel;

	for (auto lv : levels)
	{
		if (D3D12CreateDevice(nullptr, lv, IID_PPV_ARGS(&_dev)) == S_OK)
		{
			featureLevel = lv;
			break; // ���������� ������ ã�Ҵٸ� ������ ����
		}
	}

	result = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_cmdAllocator));
	result = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _cmdAllocator, nullptr, IID_PPV_ARGS(&_cmdList));
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	//Ÿ�Ӿƿ�����
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	//����͸� �ϳ��ۿ� ������� ���� ������ 0���� �մϴ�.
	cmdQueueDesc.NodeMask = 0;
	//Priority�� Ư�� ���� ����
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	//Ŀ�ǵ帮��Ʈ�� �����.
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	//ť ����
	result = _dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&_cmdQueue));

	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};

	swapchainDesc.Width = window_width;
	swapchainDesc.Height = window_height;
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.Stereo = false;
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.SampleDesc.Quality = 0;
	swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swapchainDesc.BufferCount = 2;

	//����۴� �ø��� ���̱� ����
	swapchainDesc.Scaling = DXGI_SCALING_STRETCH;

	//�ø��Ĵ� �ٷ� �ı�
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	//Ư���� ���� ����
	swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

	//������ <=>Ǯ��ũ�� �ٲٱ� ����
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	result = _dxgiFactory->CreateSwapChainForHwnd(
		_cmdQueue, hwnd,
		&swapchainDesc, nullptr, nullptr,
		(IDXGISwapChain1**)&_swapchain); //������ QueryInterface �� �̿��ؼ� 
	//IDXGISwapChain4* ���� ��ȯ�� üũ�ϴ°�, 
	// ���⿡�� �˱� ������ �߽��ϱ� ���� �ؽ�Ʈ�� ����
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};

	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; //����Ÿ�ٺ��̹Ƿ� RTV
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 2; //�� �ڷ� 2����
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE; //Ư���� ���� ����
	ID3D12DescriptorHeap* rtvHeaps = nullptr;
	result = _dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeaps));

	DXGI_SWAP_CHAIN_DESC swcDesc = {};

	result = _swapchain->GetDesc(&swcDesc);

	std::vector<ID3D12Resource*> _backBuffers(swcDesc.BufferCount);
	D3D12_CPU_DESCRIPTOR_HANDLE handle = rtvHeaps->GetCPUDescriptorHandleForHeapStart();

	for (int idx = 0; idx < swcDesc.BufferCount; ++idx)
	{
		result = _swapchain->GetBuffer(idx, IID_PPV_ARGS(&_backBuffers[idx]));
		handle.ptr += idx * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		_dev->CreateRenderTargetView(_backBuffers[idx], nullptr, handle);
	}
	ID3D12Fence* _fence = nullptr;
	UINT64 _fenceVal = 0;

	result = _dev->CreateFence(
		_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));
	//������ ǥ��
	ShowWindow(hwnd, SW_SHOW);

	XMFLOAT3 vertices[] =
	{
		{-0.4f, -0.7f, 0.0f}, //���ʾƷ� �ε���: 0
		{-0.4f, 0.7f, 0.0f}, //������ �ε���: 1
		{0.4f, -0.7f, 0.0f}, //�����ʾƷ� �ε���: 2
		{0.4f, 0.7f, 0.0f}, //�������� �ε���: 3
	}; //4����
	D3D12_HEAP_PROPERTIES heapprop = {};

	heapprop.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC resdesc = {};

	resdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resdesc.Width = sizeof(vertices); //���������� ���� ���� ������
	resdesc.Height = 1;
	resdesc.DepthOrArraySize = 1;
	resdesc.MipLevels = 1;
	resdesc.Format = DXGI_FORMAT_UNKNOWN;
	resdesc.SampleDesc.Count = 1;
	resdesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	resdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	ID3D12Resource* vertBuff = nullptr;

	result = _dev->CreateCommittedResource(
		&heapprop,
		D3D12_HEAP_FLAG_NONE,
		&resdesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff)
	);
	ID3DBlob* vsBlob = nullptr;
	ID3DBlob* psBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	result = D3DCompileFromFile(L"BasicVertexShader.hlsl", //���̴���
		nullptr, //define ����
		D3D_COMPILE_STANDARD_FILE_INCLUDE, //��Ŭ���� ����Ʈ
		"BasicVS", "vs_5_0", //�Լ��� BasicVS, ��� ���̴��� vs_5_0
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, //����׿� ���� ����ȭ����
		0, &vsBlob, &errorBlob); //�������� errorBlob �� �޽����� ���ϴ�.
	if (FAILED(result))
	{
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
		{
			::OutputDebugStringA("������ ã�� �� �����ϴ�");
			return 0; //exit() ���� ������ ������ ��ü�ϴ� ���� ����
		}
		else
		{
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());

			std::copy_n((char*)errorBlob->GetBufferPointer(),
				errorBlob->GetBufferSize(),
				errstr.begin());
			errstr += "\n";

			::OutputDebugStringA(errstr.c_str());
		}
	}

	result = D3DCompileFromFile(
		L"BasicPixelShader.hlsl", //���̴���
		nullptr, //define ����
		D3D_COMPILE_STANDARD_FILE_INCLUDE, //��Ŭ���� ����Ʈ
		"BasicPS", "ps_5_0", //�Լ��� BasicPS, ��� ���̴��� ps_5_0
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, //����׿� ���� ����ȭ ����
		0,
		&psBlob, &errorBlob); //���������� errorBlob �� �޽����� ���ϴ�.
	if (FAILED(result))
	{
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
		{
			::OutputDebugStringA("������ ã�� �� �����ϴ�");
			return 0; //exit() ���� ������ ������ ��ü�ϴ� ���� ����
		}
		else
		{
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());

			std::copy_n((char*)errorBlob->GetBufferPointer(),
				errorBlob->GetBufferSize(),
				errstr.begin());
			errstr += "\n";

			::OutputDebugStringA(errstr.c_str());
		}
	}
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{
			"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
	};
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline = {};

	gpipeline.pRootSignature = nullptr; // ���߿� �����մϴ�

	gpipeline.VS.pShaderBytecode = vsBlob->GetBufferPointer();
	gpipeline.VS.BytecodeLength = vsBlob->GetBufferSize();
	gpipeline.PS.pShaderBytecode = psBlob->GetBufferPointer();
	gpipeline.PS.BytecodeLength = psBlob->GetBufferSize();

	//����Ʈ ���� ����ũ�� ��Ÿ���� ����(0xffffffff)
	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	gpipeline.BlendState.AlphaToCoverageEnable = false;
	gpipeline.BlendState.IndependentBlendEnable = false;

	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};
	renderTargetBlendDesc.BlendEnable = false;
	renderTargetBlendDesc.LogicOpEnable = false;
	renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	gpipeline.BlendState.RenderTarget[0] = renderTargetBlendDesc;
	//���� ��Ƽ������� ������� �ʱ� ������ false
	gpipeline.RasterizerState.MultisampleEnable = false;
	gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE; //�ø����� �ʽ��ϴ�
	gpipeline.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID; //���� ĥ�մϴ�.
	gpipeline.RasterizerState.DepthClipEnable = true; //�ɵ������� Ŭ������ ��ȿ�ϰ�

	//InputLayout
	gpipeline.InputLayout.pInputElementDescs = inputLayout; //���̾ƿ� ù �ּ�
	gpipeline.InputLayout.NumElements = _countof(inputLayout); //���̾ƿ� �迭�� ���Ҽ�
	gpipeline.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED; //�� ����
	//�ﰢ���� ����
	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	gpipeline.NumRenderTargets = 1; //������ �ϳ���
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; //0~1�� ����ȭ�� RGBA
	//��Ƽ��������� ���� ���ü� ����
	gpipeline.SampleDesc.Count = 1; //���ø��� 1�ȼ��� 1
	gpipeline.SampleDesc.Quality = 0; //����Ƽ�� �ּ�

	ID3D12RootSignature* rootsignature = nullptr;

	//D3D12_ROOT_SIGNATURE_DESC �ۼ�
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	//���̳ʸ��ڵ� �ۼ�
	ID3DBlob* rootSigBlob = nullptr;

	result = D3D12SerializeRootSignature(
		&rootSignatureDesc, //��Ʈ�ñ״�ó�� ����
		D3D_ROOT_SIGNATURE_VERSION_1_0, //��Ʈ�ñ״�ó ����
		&rootSigBlob, //���̴��� ������� ���� ����
		&errorBlob //����ó���� ����
	);
	result = _dev->CreateRootSignature(
		0, //nodemask. 0���� �մϴ�
		rootSigBlob->GetBufferPointer(), //���̴����� ����
		rootSigBlob->GetBufferSize(), //���̴����� ����
		IID_PPV_ARGS(&rootsignature));
	rootSigBlob->Release(); //�ʿ������ ����

	gpipeline.pRootSignature = rootsignature;
	//�׷��Ƚ� ���������� ������Ʈ�� ����
	ID3D12PipelineState* _pipelinestate = nullptr;
	result = _dev->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&_pipelinestate));

	//����Ʈ
	D3D12_VIEWPORT viewport = {};

	viewport.Width = window_width; //��´���� �� (�ȼ� ��)
	viewport.Height = window_height; //��´���� ���� (�ȼ� ��)
	viewport.TopLeftX = 0; //��´���� �»���ǥX
	viewport.TopLeftY = 0; //��´���� �»���ǥY
	viewport.MaxDepth = 1.0f; //�ɵ��ִ�ġ
	viewport.MinDepth = 0.0f; //�ɵ��ּ�ġ

	//�������� ǥ��
	D3D12_RECT scissorrect = {};

	scissorrect.top = 0; //������ �� ��ǥ
	scissorrect.left = 0; //������ �� ��ǥ
	scissorrect.right = scissorrect.left + window_width; //������ ����ǥ
	scissorrect.bottom = scissorrect.top + window_height; //������ ����ǥ


	XMFLOAT3* vertMap = nullptr;
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);
	std::copy(std::begin(vertices), std::end(vertices), vertMap);
	vertBuff->Unmap(0, nullptr);

	D3D12_VERTEX_BUFFER_VIEW vbView = {};

	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress(); //������ ���� �ּ�
	vbView.SizeInBytes = sizeof(vertices); //�� ����Ʈ ��
	vbView.StrideInBytes = sizeof(vertices[0]); //1���� �� ����Ʈ ��

	unsigned short indices[] = {
		0, 1, 2,
		2, 1, 3
	};
	ID3D12Resource* idxBuff = nullptr;

	//������, ������ ������ �̿�, ���������� ������ �����ص� �����ϴ�
	resdesc.Width = sizeof(indices);

	result = _dev->CreateCommittedResource(
		&heapprop,
		D3D12_HEAP_FLAG_NONE,
		&resdesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&idxBuff)
	);

	//���� ���۷� �ε��� �����͸� ����
	unsigned short* mappedIdx = nullptr;
	idxBuff->Map(0, nullptr, (void**)&mappedIdx);
	std::copy(std::begin(indices), std::end(indices), mappedIdx);
	idxBuff->Unmap(0, nullptr);

	//�ε��� ���ۺ��� �ۼ�
	D3D12_INDEX_BUFFER_VIEW ibView = {};

	ibView.BufferLocation = idxBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = sizeof(indices);

	MSG msg = {};

	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		//���÷����̼��� ������ �� message�� WM_QUIT�� �˴ϴ�
		if (msg.message == WM_QUIT)
		{
			break;
		}
		//����Ÿ�� ����
		auto bbIdx = _swapchain->GetCurrentBackBufferIndex();
		D3D12_RESOURCE_BARRIER BarrierDesc = {};

		BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION; //����
		BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE; //Ư���� ���� ����
		BarrierDesc.Transition.pResource = _backBuffers[bbIdx]; //����۸��ҽ�
		BarrierDesc.Transition.Subresource = 0;

		BarrierDesc.Transition.StateBefore
			= D3D12_RESOURCE_STATE_PRESENT; // ������ PRESENT����
		BarrierDesc.Transition.StateAfter
			= D3D12_RESOURCE_STATE_RENDER_TARGET; //���ݺ��� ����Ÿ�� ����

		_cmdList->ResourceBarrier(1, &BarrierDesc); //�踮����������

		//��ȭ���
		_cmdList->SetPipelineState(_pipelinestate);

		auto rtvH = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
		rtvH.ptr += bbIdx * _dev->GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		_cmdList->OMSetRenderTargets(1, &rtvH, true, nullptr);

		//ȭ�� Ŭ����
		float clearColor[] = { 1.0f, 1.0f, 0.0f, 1.0f }; //ȭ���� ����
		_cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);
		//����Ʈ�� ������ ������ ����
		_cmdList->RSSetViewports(1, &viewport);
		_cmdList->RSSetScissorRects(1, &scissorrect);
		//��Ʈ�ñ״�ó ��Ʈ
		_cmdList->SetGraphicsRootSignature(rootsignature);

		//������Ƽ�� �������� �޼ҵ� ����
		_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		_cmdList->IASetVertexBuffers(0, 1, &vbView);
		_cmdList->IASetIndexBuffer(&ibView);

		//_cmdList->DrawInstanced(4, 1, 0, 0);
		_cmdList->DrawIndexedInstanced(6, 1, 0, 0, 0);
		//�յڸ� �ٲ۴�
		BarrierDesc.Transition.StateBefore
			= D3D12_RESOURCE_STATE_RENDER_TARGET; // ������ PRESENT����
		BarrierDesc.Transition.StateAfter
			= D3D12_RESOURCE_STATE_PRESENT; //���ݺ��� ����Ÿ�� ����




		_cmdList->ResourceBarrier(1, &BarrierDesc); //�踮����������

		// ����� Ŭ����
		_cmdList->Close();
		//Ŀ�ǵ� ����Ʈ�� ����
		ID3D12CommandList* cmdlists[] = { _cmdList };
		_cmdQueue->ExecuteCommandLists(1, cmdlists);
		_cmdQueue->Signal(_fence, ++_fenceVal);
		if (_fence->GetCompletedValue() != _fenceVal)
		{
			//�̺�Ʈ�ڵ��� ���
			auto event = CreateEvent(nullptr, false, false, nullptr);
			_fence->SetEventOnCompletion(_fenceVal, event);
			//�̺�Ʈ�� �߻��ϱ���� ��� ��ٸ��� (INFINITE)
			WaitForSingleObject(event, INFINITE);

			//�̺�Ʈ�ڵ��� �ݴ´�
			CloseHandle(event);
		}
		_cmdAllocator->Reset(); //ť�� Ŭ����
		_cmdList->Reset(_cmdAllocator, nullptr); //�ٽ� Ŀ�ǵ帮��Ʈ�� ���� �غ�
		//�ø�
		_swapchain->Present(1, 0);
		//result = _cmdAllocator->Reset();
	}
	//D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&_dev));

	//���� Ŭ������ ������� �ʱ� ������ ��������մϴ�.
	UnregisterClass(w.lpszClassName, w.hInstance);
	getchar();  // ����� �Է��� ��� (���α׷� ���� ����)
	return 0;  // ���α׷� ���� (0�� ������ ���� �ǹ�)
}
