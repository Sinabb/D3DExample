#include <sstream>
#include "D3DFramework.h"

#pragma comment (lib,"d3d11.lib")


void D3DFramework::InitWindow(HINSTANCE hInstance)
{
    WNDCLASSEX wc{};

    mInstance = hInstance;
    mTitleText = TITLE;

    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpszClassName = CLASSNAME.c_str();
    wc.hInstance = mInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpfnWndProc = WindowProc;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.hbrBackground = NULL;

    if (!RegisterClassEx(&wc))
    {
        MessageBox(NULL, L"Failed to register", L"Error", MB_OK);
        return;
    }

    RECT wr{ 0,0, mScreenWidth,mScreenHeight };
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
    mHwnd = CreateWindowEx(NULL,
        CLASSNAME.c_str(),
        mTitleText.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        wr.right - wr.left,
        wr.bottom - wr.top,
        NULL,
        NULL,
        hInstance, 
        NULL);
    if (mHwnd == NULL)
    {
        MessageBox(NULL, L"Failed to Create Window", L"Error", MB_OK);
        return;
    }
    SetWindowLongPtr(mHwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    ShowWindow(mHwnd, SW_SHOW);
    SetForegroundWindow(mHwnd);
    SetFocus(mHwnd);
    UpdateWindow(mHwnd);

}

void D3DFramework::InitD3D()
{
    DXGI_SWAP_CHAIN_DESC scd{};
    ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));
    scd.BufferCount = 1; // 프론트는 필수라 1개로 쳐서 백 버퍼 1개만 [추가할 버퍼만 세기]
    scd.BufferDesc.Width = mScreenWidth;
    scd.BufferDesc.Height = mScreenHeight;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = mHwnd;
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
        mspSwapChain.ReleaseAndGetAddressOf(),
        mspDevice.ReleaseAndGetAddressOf(),
        nullptr,
        mspDeviceContext.ReleaseAndGetAddressOf());
    OnResize();
}

void D3DFramework::CalculateFPS()
{
    static int frameCount{ 0 };
    static float timeElapsed{0.0f};

    frameCount++;

    if (mTimer.TotalTime() - timeElapsed >= 1.0f)
    {
        float fps = (float)frameCount;
        float mspf = 1000.0f / fps;

        std::wostringstream oss;
        oss.precision(6);
        oss << mTitleText << L"-" << L"FPS" << fps << L", FrameTime:" <<mspf<<L"ms";

        SetWindowText(mHwnd, oss.str().c_str());

        frameCount = 0;
        timeElapsed += 1.0f;
    }
}

void D3DFramework::OnResize()
{
    ID3D11RenderTargetView* nullViews[]{ nullptr };
    mspDeviceContext->OMSetRenderTargets(_countof(nullViews), nullViews, nullptr);

    mspRenderTargetView.Reset();
    mspDepthStencilView.Reset();
    mspRenderTraget.Reset();
    mspDepthStencil.Reset();
    mspDeviceContext->Flush();

    mspSwapChain->ResizeBuffers(0, mScreenWidth, mScreenHeight, DXGI_FORMAT_UNKNOWN, 0);

    mspSwapChain->GetBuffer(0, IID_PPV_ARGS(mspRenderTraget.ReleaseAndGetAddressOf()));
    mspDevice->CreateRenderTargetView(
        mspRenderTraget.Get(),
        nullptr,
        mspRenderTargetView.ReleaseAndGetAddressOf());

    CD3D11_TEXTURE2D_DESC td(
        DXGI_FORMAT_D24_UNORM_S8_UINT,
        mScreenWidth,
        mScreenHeight,
        1,
        1,
        D3D11_BIND_DEPTH_STENCIL
    );
    mspDevice->CreateTexture2D(&td, nullptr, mspDepthStencil.ReleaseAndGetAddressOf());

    //Depth Stencil View
    CD3D11_DEPTH_STENCIL_VIEW_DESC dsvd(D3D11_DSV_DIMENSION_TEXTURE2D);
    mspDevice->CreateDepthStencilView(
        mspDepthStencil.Get(), &dsvd, mspDepthStencilView.ReleaseAndGetAddressOf());;

    // 파이프라인 설정
    mspDeviceContext->OMSetRenderTargets(1, mspRenderTargetView.GetAddressOf(), mspDepthStencilView.Get());

    CD3D11_VIEWPORT viewport(0.f, 0.0f,
        static_cast<FLOAT>(mScreenWidth), static_cast<FLOAT>(mScreenHeight));

    mspDeviceContext->RSSetViewports(1, &viewport);
}

void D3DFramework::RenderFrame()
{
    float bg[4]{ 0.0f,0.2f,0.4f,1.0f };

    mspDeviceContext->ClearRenderTargetView(mspRenderTargetView.Get(), bg);
    mspDeviceContext->ClearDepthStencilView(mspDepthStencilView.Get(),
        D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
        1.0f, 0);

    Render();

    mspSwapChain->Present(0, 0);
}

void D3DFramework::Render()
{

}

void D3DFramework::Update(float delta)
{
}

void D3DFramework::Initialize(HINSTANCE hinstance, int width, int height)
{
    mScreenWidth = width;
    mScreenHeight = height;
    mPaused = false;

    InitWindow(hinstance);
    InitD3D();
}

void D3DFramework::Destroy()
{
    mspSwapChain->SetFullscreenState(FALSE, nullptr);

    mspDepthStencilView.Reset();
    mspDepthStencil.Reset();

    mspRenderTargetView.Reset();
    mspRenderTraget.Reset();

    mspSwapChain.Reset();
    mspDevice.Reset();
    mspDeviceContext.Reset();

    DestroyWindow(mHwnd);
    UnregisterClass(CLASSNAME.c_str(), mInstance);
}

void D3DFramework::GameLoop()
{
    mTimer.Start();
    mInput.Initialize();
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
            mTimer.Update();
            if (mPaused)
            {
                Sleep(100);
            }
            else
            {
                CalculateFPS();
                Update(mTimer.DeltaTime());
                RenderFrame();
            }
        }
    }
}

LRESULT D3DFramework::MessageHandler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{

    switch (message)
    {
    case WM_KEYDOWN:
        mInput.SetKeyDown(static_cast<unsigned int> (wParam));
        break;

    case WM_KEYUP:
        mInput.SetKeyUP(static_cast<unsigned int> (wParam));
        break;

    case WM_LBUTTONDOWN:
        mInput.SetKeyDown(VK_LBUTTON);
        break;

    case WM_RBUTTONDOWN:
        mInput.SetKeyDown(VK_RBUTTON);
        break;
    case WM_MOUSEMOVE:
        mInput.SetCursor(LOWORD(lParam), HIWORD(lParam));
        break;

    case WM_ACTIVATE:
        if (LOWORD(wParam) == WA_INACTIVE)
        {
            mPaused = true;
            mTimer.Stop();
        }
        else
        {
            mPaused = false;
            mTimer.Resume();
        }
        break;

    case WM_PAINT:
        if (mResizing)
        {
            RenderFrame();
        }
        else
        {
            PAINTSTRUCT ps;
            BeginPaint(hwnd, &ps);
            EndPaint(hwnd, &ps);
        }
        break;

    case WM_ERASEBKGND:
        return 1;
        break;

    case WM_ENTERSIZEMOVE:
        mPaused = true;
        mResizing = true;
        mTimer.Stop();
        break;

    case WM_SIZE:
        mScreenWidth = LOWORD(lParam);
        mScreenHeight = HIWORD(lParam);

        if (!mspDevice)
        {
            break;
        }

        if (wParam == SIZE_MINIMIZED)
        {
            if (!mPaused)
            {
                mTimer.Stop();
            }
            mPaused = true;
            mMinimized = true;
            mMaximized = false;
        }
        else if (wParam == SIZE_MAXIMIZED)
        {
            mTimer.Resume();
            mPaused = false;
            mMaximized = true;
            mMinimized = false;
            OnResize();
        }
        else if (wParam == SIZE_RESTORED)
        {
            if (mMinimized)
            {
                mPaused = false;
                mTimer.Resume();
                mMinimized = false;
                OnResize();
            }
            else if (mMaximized)
            {
                mPaused = false;
                mTimer.Resume();
                mMaximized = false;
                OnResize();
            }
            else if (mResizing)
            {
            }
            else
            {
                mPaused = false;
                mTimer.Resume();
                OnResize();
            }
        }
        break;
    case WM_GETMINMAXINFO:
        reinterpret_cast<MINMAXINFO*>(lParam)->ptMinTrackSize.x = 640;
        reinterpret_cast<MINMAXINFO*>(lParam)->ptMinTrackSize.y = 480;
        break;
    case WM_EXITSIZEMOVE:
        mPaused = false;
        mResizing = false;
        mTimer.Resume();
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