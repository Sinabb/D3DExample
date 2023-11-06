#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <wrl/client.h>
#include <d3d11.h>

#pragma comment (lib,"d3d11.lib")

const wchar_t gClassName[]{ L"D3DWindowClass" };
const wchar_t gTitle[]{ L"Direct3D Example" };
const int wINDOW_WIDTH{ 800 };
const int WINDOW_HEIGHT{ 600 };

HWND gHwnd{};
HINSTANCE gInstance{};

using namespace Microsoft::WRL;
//interface Directx Graphics Infrastructure
ComPtr<IDXGISwapChain> gspSwapChain{};
ComPtr<ID3D11Device> gspDeviec{};
ComPtr<ID3D11DeviceContext> gspDeviceContext{};

ComPtr<ID3D11Texture2D> gspRenderTraget{};
ComPtr<ID3D11RenderTargetView> gspRenderTargetView{};

ComPtr<ID3D11Texture2D> gspDepthStencil{};
ComPtr<ID3D11DepthStencilView> gspDepthStencilView{};

void InitD3D();
void RenderFrame();
void DestoryD3D();

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nShowCmd)
{
    WNDCLASSEX wc{};
    gInstance = hInstance;

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpszClassName = gClassName;
	wc.hInstance = gInstance;
	wc.lpfnWndProc = WindowProc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;

	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, L"Failed to register",L"Error", MB_OK);
		return 0;
	}

	RECT wr{ 0,0, wINDOW_WIDTH,WINDOW_HEIGHT };
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
    gHwnd = CreateWindowEx(NULL,
        gClassName,
        gTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        wr.right - wr.left,
        wr.bottom - wr.top,
        NULL,
        NULL,
        hInstance,
        NULL);
    if (gHwnd == NULL)
    {
        MessageBox(NULL, L"Failed to Create Window", L"Error", MB_OK);
        return 0;
    }

    ShowWindow(gHwnd, nShowCmd);
    SetForegroundWindow(gHwnd);
    SetFocus(gHwnd);
    UpdateWindow(gHwnd);

    InitD3D();

    MSG msg{};
    while (true)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
            {
                break;
            }
        }
        else
        {
            RenderFrame();
            // Game
        }
    }
    DestoryD3D();

    return static_cast<int>(msg.wParam);
}

void InitD3D()
{
    DXGI_SWAP_CHAIN_DESC scd{};
    ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));
    scd.BufferCount = 1; // 프론트는 필수라 1개로 쳐서 백 버퍼 1개만 [작성 추가로 할 작업]
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = gHwnd;
    scd.SampleDesc.Count = 1; //msaa : multi samplinf anti-aliasing
    scd.Windowed = TRUE;

    D3D11CreateDeviceAndSwapChain(NULL,
        D3D_DRIVER_TYPE_HARDWARE,
        NULL,
        0,
        nullptr,
        0,
        D3D11_SDK_VERSION,
        &scd,
        gspSwapChain.ReleaseAndGetAddressOf(),
        gspDeviec.ReleaseAndGetAddressOf(),
        nullptr,
        gspDeviceContext.ReleaseAndGetAddressOf());

    gspSwapChain->GetBuffer(0, IID_PPV_ARGS(gspRenderTraget.ReleaseAndGetAddressOf()));
    gspDeviec->CreateRenderTargetView(
        gspRenderTraget.Get(),
        nullptr,
        gspRenderTargetView.ReleaseAndGetAddressOf());

    CD3D11_TEXTURE2D_DESC td(
        DXGI_FORMAT_D24_UNORM_S8_UINT,
        wINDOW_WIDTH,
        WINDOW_HEIGHT,
        1,
        1,
        D3D11_BIND_DEPTH_STENCIL
    );
    gspDeviec->CreateTexture2D(&td, nullptr, gspDepthStencil.ReleaseAndGetAddressOf());

    //Depth Stencil View
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvd(D3D11_DSV_DIMENSION_TEXTURE2D);
    gspDeviec->CreateDepthStencilView(
        gspDepthStencil.Get(), &dsvd, gspDepthStencilView.ReleaseAndGetAddressOf());

    // 파이프라인 설정
    gspDeviceContext->OMSetRenderTargets(1, gspRenderTargetView.GetAddressOf(), gspDepthStencilView.Get());

    D3D11_VIEWPORT viewport(0.0f, 0.0f, 
        static_cast<FLOAT>(wINDOW_WIDTH), static_cast<FLOAT>(WINDOW_HEIGHT));
    gspDeviceContext->RSSetViewports(1, &viewport);
}

void RenderFrame()
{
    float bg[4]{ 0.0f,0.2f,0.4f,1.0f };

    gspDeviceContext->ClearRenderTargetView(gspRenderTargetView.Get(), bg);
    gspDeviceContext->ClearDepthStencilView(gspDepthStencilView.Get(),
        D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
        1.0f, 0);

    gspSwapChain->Present(0, 0);
}

void DestoryD3D()
{
    gspDepthStencilView.Reset();
    gspDepthStencil.Reset();

    gspRenderTargetView.Reset();
    gspRenderTraget.Reset();

    gspSwapChain.Reset();
    gspDeviec.Reset();
    gspDeviceContext.Reset();

    DestroyWindow(gHwnd);
    UnregisterClass(gClassName, gInstance);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}
