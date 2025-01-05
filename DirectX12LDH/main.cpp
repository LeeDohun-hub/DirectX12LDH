#include <Windows.h>  // Windows API 함수와 자료형을 사용하기 위한 헤더 파일
#include <tchar.h>    // 유니코드 및 멀티바이트 문자 관리를 위한 헤더
#include <d3d12.h>
#include <dxgi1_6.h>
#include <vector>
#include <string>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#ifdef _DEBUG
#include <iostream>   // 디버그 모드에서 표준 입출력을 사용하기 위한 헤더
#endif
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
using namespace std;  // std 네임스페이스를 사용하여 코드 가독성 향상
using namespace DirectX;

// 콘솔 화면에 포맷이 지정된 문자열을 출력하는 디버그 전용 함수
// 디버그 상태에서만 작동하며, 문자열 포맷을 받아 출력함
void DebugOutputFormatString(const char* format, ...)
{
#ifdef _DEBUG
	va_list valist;  // 가변 인수 목록을 저장하기 위한 객체 선언
	va_start(valist, format);  // 가변 인수 목록 초기화 (format 이후 인수부터 시작)
	vprintf(format, valist);  // 포맷을 기반으로 가변 인수 목록 출력
	va_end(valist);  // 가변 인수 목록 종료
#endif
}
//귀찮더라도 반드시 써야하는 함수
LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM Iparam)
{
	//윈도우가 폐기되었다면 불러오기
	if (msg == WM_DESTROY)
	{
		PostQuitMessage(0); //OS에 대해 [이제 이 어플리를 끝낸다]라 전달한다
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, Iparam); //기정의 처리를 합니다.
}
//화면 크기
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

	debugLayer->EnableDebugLayer(); //디버그레이어를 유효화한다
	debugLayer->Release(); //유효화했다면 인터페이스를 해제한다.
}

#ifdef _DEBUG
int main()  // 디버그 모드에서 실행되는 main 함수 (콘솔 기반 애플리케이션)
{
#else
#include <Windows.h>
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)  // 릴리즈 모드에서 실행되는 WinMain 함수 (윈도우 GUI 애플리케이션)
{
#endif
	DebugOutputFormatString("Welcome to DirectX12");  // "Show window test."라는 문자열을 디버그 상태에서 콘솔에 출력
	WNDCLASSEX w = {};

	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProcedure; //콜백함수의 지정
	w.lpszClassName = _T("DX12Sample"); //어플리케이션 클래스 이름(적절히 짓기)
	w.hInstance = GetModuleHandle(nullptr); // 핸들의 취득
	RegisterClassEx(&w); //어플리케이션 클래스(윈도우 클래스의 지정을 OS에 전달)

	RECT wrc = { 0, 0, window_width, window_height }; //윈도우 사이즈를 정합니다

	//함수를 사용해서 윈도우의 사이즈를 보정합니다
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	//윈도우오브젝트의 생성
	HWND hwnd = CreateWindow(w.lpszClassName, //클래스명 지정
		_T("DX12 테스트"), //타이틀 바의 문자
		WS_OVERLAPPEDWINDOW, //타이틀바와 경계선이 있는 윈도우
		CW_USEDEFAULT, //표시 x좌표는 OS에 맡깁니다
		CW_USEDEFAULT, //표시 y좌표는 OS에 맡깁니다
		wrc.right - wrc.left, //윈도우 폭width
		wrc.bottom - wrc.top, //윈도우 높이
		nullptr, //새로운 윈도우 핸들
		nullptr, //메뉴핸들
		w.hInstance, //불러온 어플리케이션 핸들
		nullptr); //추가파라미터

#ifdef _DEBUG
	//디버그레이어를 ON으로
	EnableDebugLayer();
#endif
	//이하 DirectX12 주변의 초기화

	//피처레벨 열거
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
	//	IUnknown * pDevice, //커맨드큐오브젝트
	//	HWND hWnd,		//윈도우핸들
	//	const DXGI_SWAP_CHAIN_DESC * pDesc,		//스왑체인설정
	//	const DXGI_SWAP_CHAIN_FULLSCREEN_DESC * pFullscreenDesc, //우선 nullptr으로 
	//	IDXGIOutput * pRestrictToOutput,							//이것도 nullptr
	//	IDXGISwapChain1 * *ppSwapChain							//스왑체인 오브젝트 취득용
	//);

	//typedef struct DXGI_SWAP_CHAIN_DESC1
	//{
	//	UINT	Width; //화면해상도 (폭)
	//	UINT	Height; //화면해상도 (높이)
	//	DXGI_FORMAT Format; //픽셀포맷
	//	BOOL Stereo; //스테레오표시플러그 (3D디스플레이의 스테레오 모드)
	//	DXGI_SAMPLE_DESC SampleDesc; //멀티샘플의 지정(Count = 1, Quality = 0으로 좋습니다)
	//	DXGI_USAGE BufferUsage; //DXGI_USAGE_BACK_BUFFER 가능
	//	UINT BufferCount; // 더블버퍼라면 2개 가능
	//	DXGI_SCALING Scaling; //DXGI_SCALING_STRETCH 가능
	//	DXGI_SWAP_EFFECT SwapEffect; //DXGI_SWAP_EFFECT_FLIP_DISCARD 가능
	//	DXGI_ALPHA_MODE AlphaMode; //DXGI_ALPHA_MODE_UNSPECIFIED 가능
	//	UINT Flags; //DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH 가능
	//}DXGI_SWAP_CHAIN_DESC1;
	//void  CreateRenderTargetView(
	//	ID3D12Resource * pResource, //버퍼
	//	const D3D12_RENDER_TARGET_VIEW_DESC * pDesc, //이번은 nullptr로 좋습니다
	//	D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor //디스크립터힙핸들
	//);
	//void OMSetRenderTargets(
	//	UINT numRTVDescriptors, //렌더타겟수 (1로 좋습니다)
	//	const D3D12_CPU_DESCRIPTOR_HANDLE * pRTVHandles, //렌더타겟한들의
	//	//처음 주소
	//	BOOL RTsSingleHandleToDescriptorRange, //복수떄의 연계하고있는가
	//	const D3D12_CPU_DESCRIPTOR_HANDLE //심도스텐실버퍼뷰의
	//	* pDepthStencilDescriptor			//핸들(nullptr로 좋습니다)
	//);
	//void ExecuteCommandLists(
	//	UINT NumCommandLists, //실행하는 커맨드리스트수(하나로 좋습니다)
	//	ID3D12CommandList* const* ppCommandLists //커맨드리스트 배열의 처음 주소
	//);
	//HRESULT CreateFence(
	//	UINT64 InitialValue, //초기화 수치(0)
	//	D3D12_FENCE_FLAGS Flags, //우선 D3D12_FENCE_FLAG_NONE
	//	REFIID riid,
	//	void** ppFence
	//);
	//HRESULT Signal(
	//	ID3D12Fence * pFence, //아까 만든 펜스오브젝트
	//	UINT64 Value //GPU의 처리가 완료한 뒤에 되어있어야만 하는 수치(펜스치)
	//);
	//HRESULT SetEventOnCompletion(
	//	UINT64 Value, //이 수치가 됐다면 이벤트를 발생시킨다
	//	HANDLE hEvent //발생시키는 이벤트
	//);
	//void ResourceBarrier(
	//	UINT NumBarriers, //설정배리어의 수(현시점에서는 1로)
	//	const D3D12_RESOURCE_BARRIER * pBarriers //설정배리어 구조체 주소
	//
	//);
	//typedef struct D3D12_RESOURCE_BARRIER
	//{
	//	D3D12_RESOURCE_BARRIER_TYPE Type; //배리어의 종별 (상태전이이므로 TRANSITION)
	//	D3D12_RESOURCE_BARRIER_FLAGS Flags; //특별한 것은 안하므로 FLAG_NONE으로 좋습니다
	//
	//	union //공용체인 것으로 주의
	//	{
	//		D3D12_RESOURCE_TRANSITION_BARRIER Transition; //여기를 사용
	//		D3D12_RESOURCE_ALIASING_BARRIER Aliasing;
	//		D3D12_RESOURCE_UAV_BARRIER UAV;
	//	};
	//} D3D12_RESOURCE_BARRIER;
	//
	//typedef struct D3D12_RESOURCE_TRANSITION_BARRIER
	//{
	//	ID3D12Resource* pResource; //리소스의 주소
	//	UINT Subresource; //서브리소스번호 (0으로 좋습니다)
	//	D3D12_RESOURCE_STATES StateBefore; //원래의 상태
	//	D3D12_RESOURCE_STATES StateAfter; //다음의 상태
	//} D3D12_RESOURCE_TRANSITION_BARRIER;
	//union
	//{
	//	D3D12_RESOURCE_TRANSITION_BARRIER Transition; //여기를 사용
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
	//_In_  const D3D12_HEAP_PROPERTIES* pHeapProperties, //힙설정구조체의 주소
	//D3D12_HEAP_FLAGS HeapFlags,//특히 지정하지 않기 때문에 d3d12_heap_flag_none으로 
	//_In_  const D3D12_RESOURCE_DESC* pDesc,//리소스설정구조체주소
	//D3D12_RESOURCE_STATES InitialResourceState,//GPU에서 불러오기 전용으로 GENERIC_READ
	//_In_opt_  const D3D12_CLEAR_VALUE* pOptimizedClearValue, //사용하지 않기 때문에 nullptr
	//REFIID riidResource,
	//_COM_Outptr_opt_  void** ppvResource) = 0;

	//어댑터의 열거용
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

	//Direct3D디바이스의 초기화
	D3D_FEATURE_LEVEL featureLevel;

	for (auto lv : levels)
	{
		if (D3D12CreateDevice(nullptr, lv, IID_PPV_ARGS(&_dev)) == S_OK)
		{
			featureLevel = lv;
			break; // 생성가능한 버전을 찾았다면 루프를 끊기
		}
	}

	result = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_cmdAllocator));
	result = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _cmdAllocator, nullptr, IID_PPV_ARGS(&_cmdList));
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	//타임아웃없음
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	//어댑터를 하나밖에 사용하지 않을 때에는 0으로 합니다.
	cmdQueueDesc.NodeMask = 0;
	//Priority는 특히 지정 없음
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	//커맨드리스트에 맞춘다.
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	//큐 생성
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

	//백버퍼는 늘리고 줄이기 가능
	swapchainDesc.Scaling = DXGI_SCALING_STRETCH;

	//플립후는 바로 파괴
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	//특별히 지정 없음
	swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

	//윈도우 <=>풀스크린 바꾸기 가능
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	result = _dxgiFactory->CreateSwapChainForHwnd(
		_cmdQueue, hwnd,
		&swapchainDesc, nullptr, nullptr,
		(IDXGISwapChain1**)&_swapchain); //본래는 QueryInterface 를 이용해서 
	//IDXGISwapChain4* 로의 변환을 체크하는가, 
	// 여기에서 알기 쉬움을 중시하기 위한 텍스트로 대응
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};

	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; //렌더타겟뷰이므로 RTV
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 2; //앞 뒤로 2가지
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE; //특별히 지정 없음
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
	//윈도우 표시
	ShowWindow(hwnd, SW_SHOW);

	XMFLOAT3 vertices[] =
	{
		{-0.4f, -0.7f, 0.0f}, //왼쪽아래 인덱스: 0
		{-0.4f, 0.7f, 0.0f}, //왼쪽위 인덱스: 1
		{0.4f, -0.7f, 0.0f}, //오른쪽아래 인덱스: 2
		{0.4f, 0.7f, 0.0f}, //오른쪽위 인덱스: 3
	}; //4초점
	D3D12_HEAP_PROPERTIES heapprop = {};

	heapprop.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC resdesc = {};

	resdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resdesc.Width = sizeof(vertices); //초점정보가 들어갔을 뿐인 사이즈
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

	result = D3DCompileFromFile(L"BasicVertexShader.hlsl", //세이더명
		nullptr, //define 없음
		D3D_COMPILE_STANDARD_FILE_INCLUDE, //인클루드는 디폴트
		"BasicVS", "vs_5_0", //함수는 BasicVS, 대상 세이더는 vs_5_0
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, //디버그용 포함 최적화없음
		0, &vsBlob, &errorBlob); //에러때는 errorBlob 에 메시지가 들어갑니다.
	if (FAILED(result))
	{
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
		{
			::OutputDebugStringA("파일을 찾을 수 없습니다");
			return 0; //exit() 같은 것으로 적절히 대체하는 편이 좋음
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
		L"BasicPixelShader.hlsl", //세이더명
		nullptr, //define 없음
		D3D_COMPILE_STANDARD_FILE_INCLUDE, //인클루드는 디폴트
		"BasicPS", "ps_5_0", //함수는 BasicPS, 대상 세이더는 ps_5_0
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, //디버그용 포함 최적화 없음
		0,
		&psBlob, &errorBlob); //에러때에는 errorBlob 에 메시지가 들어갑니다.
	if (FAILED(result))
	{
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
		{
			::OutputDebugStringA("파일을 찾을 수 없습니다");
			return 0; //exit() 같은 것으로 적절히 대체하는 편이 좋음
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

	gpipeline.pRootSignature = nullptr; // 나중에 설정합니다

	gpipeline.VS.pShaderBytecode = vsBlob->GetBufferPointer();
	gpipeline.VS.BytecodeLength = vsBlob->GetBufferSize();
	gpipeline.PS.pShaderBytecode = psBlob->GetBufferPointer();
	gpipeline.PS.BytecodeLength = psBlob->GetBufferSize();

	//디폴트 샘플 마스크를 나타내는 정수(0xffffffff)
	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	gpipeline.BlendState.AlphaToCoverageEnable = false;
	gpipeline.BlendState.IndependentBlendEnable = false;

	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};
	renderTargetBlendDesc.BlendEnable = false;
	renderTargetBlendDesc.LogicOpEnable = false;
	renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	gpipeline.BlendState.RenderTarget[0] = renderTargetBlendDesc;
	//아직 안티엘리어스는 사용하지 않기 때문에 false
	gpipeline.RasterizerState.MultisampleEnable = false;
	gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE; //컬링하지 않습니다
	gpipeline.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID; //안을 칠합니다.
	gpipeline.RasterizerState.DepthClipEnable = true; //심도방향의 클리핑은 유효하게

	//InputLayout
	gpipeline.InputLayout.pInputElementDescs = inputLayout; //레이아웃 첫 주소
	gpipeline.InputLayout.NumElements = _countof(inputLayout); //레이아웃 배열의 원소수
	gpipeline.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED; //컷 없음
	//삼각형의 구성
	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	gpipeline.NumRenderTargets = 1; //지금은 하나만
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; //0~1로 정규화된 RGBA
	//안티엘리어싱을 위한 샘플수 설정
	gpipeline.SampleDesc.Count = 1; //샘플링은 1픽셀당 1
	gpipeline.SampleDesc.Quality = 0; //퀄리티는 최소

	ID3D12RootSignature* rootsignature = nullptr;

	//D3D12_ROOT_SIGNATURE_DESC 작성
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	//바이너리코드 작성
	ID3DBlob* rootSigBlob = nullptr;

	result = D3D12SerializeRootSignature(
		&rootSignatureDesc, //루트시그니처의 설정
		D3D_ROOT_SIGNATURE_VERSION_1_0, //루트시그니처 버전
		&rootSigBlob, //세이더를 만들었을 때와 같음
		&errorBlob //에러처리도 같음
	);
	result = _dev->CreateRootSignature(
		0, //nodemask. 0으로 합니다
		rootSigBlob->GetBufferPointer(), //세이더때와 같음
		rootSigBlob->GetBufferSize(), //세이더때와 같음
		IID_PPV_ARGS(&rootsignature));
	rootSigBlob->Release(); //필요없으니 해제

	gpipeline.pRootSignature = rootsignature;
	//그래픽스 파이프라인 오브젝트의 생성
	ID3D12PipelineState* _pipelinestate = nullptr;
	result = _dev->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&_pipelinestate));

	//뷰포트
	D3D12_VIEWPORT viewport = {};

	viewport.Width = window_width; //출력대상의 폭 (픽셀 수)
	viewport.Height = window_height; //출력대상의 높이 (픽셀 수)
	viewport.TopLeftX = 0; //출력대상의 좌상좌표X
	viewport.TopLeftY = 0; //출력대상의 좌상좌표Y
	viewport.MaxDepth = 1.0f; //심도최대치
	viewport.MinDepth = 0.0f; //심도최소치

	//시저도형 표시
	D3D12_RECT scissorrect = {};

	scissorrect.top = 0; //오려낸 상 좌표
	scissorrect.left = 0; //오려낸 좌 좌표
	scissorrect.right = scissorrect.left + window_width; //오려낸 우좌표
	scissorrect.bottom = scissorrect.top + window_height; //오려낸 하좌표


	XMFLOAT3* vertMap = nullptr;
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);
	std::copy(std::begin(vertices), std::end(vertices), vertMap);
	vertBuff->Unmap(0, nullptr);

	D3D12_VERTEX_BUFFER_VIEW vbView = {};

	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress(); //버퍼의 가상 주소
	vbView.SizeInBytes = sizeof(vertices); //전 바이트 수
	vbView.StrideInBytes = sizeof(vertices[0]); //1초점 당 바이트 수

	unsigned short indices[] = {
		0, 1, 2,
		2, 1, 3
	};
	ID3D12Resource* idxBuff = nullptr;

	//설정은, 버퍼의 사이즈 이외, 초점버퍼의 설정을 재사용해도 좋습니다
	resdesc.Width = sizeof(indices);

	result = _dev->CreateCommittedResource(
		&heapprop,
		D3D12_HEAP_FLAG_NONE,
		&resdesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&idxBuff)
	);

	//만든 버퍼로 인덱스 데이터를 복사
	unsigned short* mappedIdx = nullptr;
	idxBuff->Map(0, nullptr, (void**)&mappedIdx);
	std::copy(std::begin(indices), std::end(indices), mappedIdx);
	idxBuff->Unmap(0, nullptr);

	//인덱스 버퍼뷰의 작성
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
		//어플레케이션이 끝났을 때 message가 WM_QUIT이 됩니다
		if (msg.message == WM_QUIT)
		{
			break;
		}
		//렌더타겟 설정
		auto bbIdx = _swapchain->GetCurrentBackBufferIndex();
		D3D12_RESOURCE_BARRIER BarrierDesc = {};

		BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION; //전이
		BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE; //특벌한 지정 없음
		BarrierDesc.Transition.pResource = _backBuffers[bbIdx]; //백버퍼리소스
		BarrierDesc.Transition.Subresource = 0;

		BarrierDesc.Transition.StateBefore
			= D3D12_RESOURCE_STATE_PRESENT; // 직전은 PRESENT상태
		BarrierDesc.Transition.StateAfter
			= D3D12_RESOURCE_STATE_RENDER_TARGET; //지금부터 렌더타겟 상태

		_cmdList->ResourceBarrier(1, &BarrierDesc); //배리어지정실행

		//묘화명령
		_cmdList->SetPipelineState(_pipelinestate);

		auto rtvH = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
		rtvH.ptr += bbIdx * _dev->GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		_cmdList->OMSetRenderTargets(1, &rtvH, true, nullptr);

		//화면 클리어
		float clearColor[] = { 1.0f, 1.0f, 0.0f, 1.0f }; //화면의 색깔
		_cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);
		//뷰포트와 시저의 도형의 설정
		_cmdList->RSSetViewports(1, &viewport);
		_cmdList->RSSetScissorRects(1, &scissorrect);
		//루트시그니처 세트
		_cmdList->SetGraphicsRootSignature(rootsignature);

		//프리미티브 토폴로지 메소드 지정
		_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		_cmdList->IASetVertexBuffers(0, 1, &vbView);
		_cmdList->IASetIndexBuffer(&ibView);

		//_cmdList->DrawInstanced(4, 1, 0, 0);
		_cmdList->DrawIndexedInstanced(6, 1, 0, 0, 0);
		//앞뒤를 바꾼다
		BarrierDesc.Transition.StateBefore
			= D3D12_RESOURCE_STATE_RENDER_TARGET; // 직전은 PRESENT상태
		BarrierDesc.Transition.StateAfter
			= D3D12_RESOURCE_STATE_PRESENT; //지금부터 렌더타겟 상태




		_cmdList->ResourceBarrier(1, &BarrierDesc); //배리어지정실행

		// 명령의 클로즈
		_cmdList->Close();
		//커맨드 리스트의 실행
		ID3D12CommandList* cmdlists[] = { _cmdList };
		_cmdQueue->ExecuteCommandLists(1, cmdlists);
		_cmdQueue->Signal(_fence, ++_fenceVal);
		if (_fence->GetCompletedValue() != _fenceVal)
		{
			//이벤트핸들의 취득
			auto event = CreateEvent(nullptr, false, false, nullptr);
			_fence->SetEventOnCompletion(_fenceVal, event);
			//이벤트가 발생하기까지 계속 기다린다 (INFINITE)
			WaitForSingleObject(event, INFINITE);

			//이벤트핸들을 닫는다
			CloseHandle(event);
		}
		_cmdAllocator->Reset(); //큐를 클리어
		_cmdList->Reset(_cmdAllocator, nullptr); //다시 커맨드리스트를 쌓을 준비
		//플립
		_swapchain->Present(1, 0);
		//result = _cmdAllocator->Reset();
	}
	//D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&_dev));

	//더는 클래스를 사용하지 않기 때문에 등록해제합니다.
	UnregisterClass(w.lpszClassName, w.hInstance);
	getchar();  // 사용자 입력을 대기 (프로그램 종료 방지)
	return 0;  // 프로그램 종료 (0은 성공적 종료 의미)
}
