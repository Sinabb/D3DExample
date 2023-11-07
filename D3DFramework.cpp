#include "D3DFramework.h"

#pragma comment (lib,"d3d11.lib")

void D3DFramework::InitWindow(HINSTANCE hInstance)
{
    WNDCLASSEX wc{};

    gInstance = hInstance;

    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpszClassName = gClassName.c_str();
    wc.hInstance = gInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpfnWndProc = WindowProc;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.hbrBackground = NULL;

    if (!RegisterClassEx(&wc))
    {
        MessageBox(NULL, L"Failed to register", L"Error", MB_OK);
        return;
    }

    RECT wr{ 0,0, gScreenWidth,gScreenHeight };
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
    gHwnd = CreateWindowEx(NULL,
        gClassName.c_str(),
        gTitle.c_str(),
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
        return;
    }
    SetWindowLongPtr(gHwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    ShowWindow(gHwnd, SW_SHOW);
    SetForegroundWindow(gHwnd);
    SetFocus(gHwnd);
    UpdateWindow(gHwnd);

}

void D3DFramework::InitD3D()
{
    DXGI_SWAP_CHAIN_DESC scd{};
    ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));
    scd.BufferCount = 1; // 프론트는 필수라 1개로 쳐서 백 버퍼 1개만 [작성 추가로 할 작업]
    scd.BufferDesc.Width = gScreenWidth;
    scd.BufferDesc.Height = gScreenHeight;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = gHwnd;
    scd.SampleDesc.Count = 1; //msaa : multi samplinf anti-aliasing
    scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
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
        gspDevice.ReleaseAndGetAddressOf(),
        nullptr,
        gspDeviceContext.ReleaseAndGetAddressOf());
    OnResize();
}

void D3DFramework::OnResize()
{
    ID3D11RenderTargetView* nullViews[]{ nullptr };
    gspDeviceContext->OMSetRenderTargets(_countof(nullViews), nullViews, nullptr);

    gspRenderTargetView.Reset();
    gspDepthStencilView.Reset();
    gspRenderTraget.Reset();
    gspDepthStencil.Reset();
    gspDeviceContext->Flush();

    gspSwapChain->ResizeBuffers(0, gScreenWidth, gScreenHeight, DXGI_FORMAT_UNKNOWN, 0);

    gspSwapChain->GetBuffer(0, IID_PPV_ARGS(gspRenderTraget.ReleaseAndGetAddressOf()));
    gspDevice->CreateRenderTargetView(
        gspRenderTraget.Get(),
        nullptr,
        gspRenderTargetView.ReleaseAndGetAddressOf());

    CD3D11_TEXTURE2D_DESC td(
        DXGI_FORMAT_D24_UNORM_S8_UINT,
        gScreenWidth,
        gScreenHeight,
        1,
        1,
        D3D11_BIND_DEPTH_STENCIL
    );
    gspDevice->CreateTexture2D(&td, nullptr, gspDepthStencil.ReleaseAndGetAddressOf());

    //Depth Stencil View
    CD3D11_DEPTH_STENCIL_VIEW_DESC dsvd(D3D11_DSV_DIMENSION_TEXTURE2D);
    gspDevice->CreateDepthStencilView(
        gspDepthStencil.Get(), &dsvd, gspDepthStencilView.ReleaseAndGetAddressOf());;

    // 파이프라인 설정
    gspDeviceContext->OMSetRenderTargets(1, gspRenderTargetView.GetAddressOf(), gspDepthStencilView.Get());

    CD3D11_VIEWPORT viewport(0.f, 0.0f,
        static_cast<FLOAT>(gScreenWidth), static_cast<FLOAT>(gScreenHeight));

    gspDeviceContext->RSSetViewports(1, &viewport);
}

void D3DFramework::RenderFrame()
{
    float bg[4]{ 0.0f,0.2f,0.4f,1.0f };

    gspDeviceContext->ClearRenderTargetView(gspRenderTargetView.Get(), bg);
    gspDeviceContext->ClearDepthStencilView(gspDepthStencilView.Get(),
        D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
        1.0f, 0);

    gspSwapChain->Present(0, 0);
}

void D3DFramework::Initialize(HINSTANCE hinstance, int width, int height)
{
	gScreenWidth = width;
	gScreenHeight = height;

	InitWindow(hinstance);
	InitD3D();
}

void D3DFramework::DestoryD3D()
{
    gspSwapChain->SetFullscreenState(FALSE, nullptr);

    gspDepthStencilView.Reset();
    gspDepthStencil.Reset();

    gspRenderTargetView.Reset();
    gspRenderTraget.Reset();

    gspSwapChain.Reset();
    gspDevice.Reset();
    gspDeviceContext.Reset();

    DestroyWindow(gHwnd);
    UnregisterClass(gClassName.c_str(), gInstance);
}

void D3DFramework::GameLoop()
{
}

LRESULT D3DFramework::MessageHandler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{

    switch (message)
    {
    case WM_PAINT:
        if (gResizing)
        {
            RenderFrame();
        }
        else {
            PAINTSTRUCT ps;
            BeginPaint(hwnd, &ps);
            EndPaint(hwnd, &ps);
        }
        break;
    case WM_ERASEBKGND:
        return 1;
        break;

    case WM_ENTERSIZEMOVE:
        gResizing = true;
        break;
    case WM_SIZE:
        gScreenWidth = LOWORD(lParam);
        gScreenHeight = HIWORD(lParam);

        if (!gspDevice)
        {
            break;
        }

        if (wParam == SIZE_MINIMIZED)
        {
            gMinimized = true;
            gMaximized = false;
        }
        else if (wParam == SIZE_MAXIMIZED)
        {
            gMinimized = false;
            gMaximized = true;
        }
        else if (wParam == SIZE_RESTORED)
        {
            if (gMinimized)
            {
                gMinimized = false;
                OnResize();
            }
            else if (gMaximized)
            {
                gMaximized = false;
                OnResize();
            }
            else if (gResizing)
            {

            }
            else
            {
                OnResize();
            }
        }
        break;
    case WM_GETMINMAXINFO:
        reinterpret_cast<MINMAXINFO*>(lParam)->ptMinTrackSize.x = 640;
        reinterpret_cast<MINMAXINFO*>(lParam)->ptMinTrackSize.y = 480;
        break;
    case WM_EXITSIZEMOVE:
        gResizing = false;
        OnResize();
        break;
    case WM_MENUCHAR:
        return MAKELRESULT(0, MNC_CLOSE);
        break;
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

LRESULT WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    auto pFramework = reinterpret_cast<D3DFramework*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    return pFramework->MessageHandler(hwnd, message, wParam, lParam);
}
