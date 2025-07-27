// ClearScreen.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <tchar.h>
#include <iostream>
#include <d3dcompiler.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")


class Render
{
private:

    struct RenderDevice
    {
        ID3D12Device* device = nullptr;
        ID3D12CommandQueue* commandQueue = nullptr;
        IDXGISwapChain3* swapChain = nullptr;
        ID3D12Resource* renderTargets[2];
        ID3D12CommandAllocator* commandAlloc = nullptr;
        ID3D12GraphicsCommandList* commandList = nullptr;

    } renderDevice;

    class DescriptorHeap
    {
    public:
        ID3D12DescriptorHeap* m_Heap = nullptr;
        uint32_t m_DescriptorSize = 0;

        DescriptorHeap() = default;

        void Initialize(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT numDescriptors)
        {
            D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
            heapDesc.Type = type;
            heapDesc.NumDescriptors = numDescriptors;
            heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            heapDesc.NodeMask = 0;
            device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_Heap));
            m_DescriptorSize = device->GetDescriptorHandleIncrementSize(type);
		}

        D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(uint32_t index) const
        {
            D3D12_CPU_DESCRIPTOR_HANDLE handle = GetCPUDescriptorHandleForHeapStart();
            handle.ptr += index * m_DescriptorSize;
            return handle;
        }

        D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandleForHeapStart() const
        {
            return m_Heap->GetCPUDescriptorHandleForHeapStart();
        }
        D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandleForHeapStart() const
        {
            return m_Heap->GetGPUDescriptorHandleForHeapStart();
        }

        uint32_t GetDescriptorSize() const
        {
            return m_DescriptorSize;
		}

        void Destroy()
        {
            if (m_Heap)
            {
                m_Heap->Release();
                m_Heap = nullptr;
            }
		}
    };

public:
    Render() = default;

    uint32_t m_Width { };
    uint32_t m_Height { };
    uint32_t m_FrameCount { 2 };
    DescriptorHeap rtvDescriptorHeap {};

    bool Initialize(HWND hwnd, uint32_t width, uint32_t Heigh)
    {
		m_Width = width;
		m_Height = Heigh;
        IDXGIFactory4* factory = nullptr;
        CreateDXGIFactory1(IID_PPV_ARGS(&factory));


        /*
            +----------------+    +----------------+    +----------------+
            |    GPU 0       |    |    GPU 1       |    |    GPU 2       |
            |  Integrated    |    |   Dedicated    |    |   WARP/etc     |
            +----------------+    +----------------+    +----------------+
                   ↑                    ↑                      ↑
             IDXGIFactory::EnumAdapters / EnumAdapters1

            You can iterate adapters and choose manually.
            Passing nullptr to D3D11CreateDevice / D3D12CreateDevice
            means "use default adapter" (usually GPU 0).
        */

        D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&renderDevice.device));



        
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        renderDevice.device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&renderDevice.commandQueue));




        /*
            +----------------------------+
            |        SwapChain          |
            +----------------------------+
                 +----------+ +----------+
                 | Buffer 0 | | Buffer 1 |   <- Double buffering
                 +----------+ +----------+
                      ^            ^
                Back Buffer    Front Buffer
               (next frame)    (being shown)

            DXGI_SWAP_EFFECT_FLIP_DISCARD or FLIP_SEQUENTIAL (modern)
            IDXGISwapChain::Present() swaps the buffers

            You can increase buffer count (e.g., triple buffering)
            by setting BufferCount = 3+ in SwapChainDesc.
        */

        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.BufferCount = m_FrameCount;
        swapChainDesc.Width = m_Width;
        swapChainDesc.Height = m_Height;
        swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

        IDXGISwapChain1* tempSwapChain = nullptr;
        factory->CreateSwapChainForHwnd(renderDevice.commandQueue, hwnd, &swapChainDesc, nullptr, nullptr, &tempSwapChain);


        tempSwapChain->QueryInterface(IID_PPV_ARGS(&renderDevice.swapChain));
        factory->Release();


        /*
           [RTV DESCRIPTOR HEAP] (example: 3 entries for potential triple buffering)

           +--------+--------+--------+
           | RTV #0 | RTV #1 | RTV #2 |
           +--------+--------+--------+
              ^        ^         ^
              |        |         +-- Optional: can be used for post-process or extra framebuffer
              |        +------------ Used for back buffer 1
              +--------------------- Used for back buffer 0

           - CPU writes here using CreateRenderTargetView()
           - GPU reads from here via OMSetRenderTargets()

           Note:
           - Only the RTVs corresponding to the swap chain’s back buffers are required.
           - You can optionally add more RTVs for offscreen passes, etc.
       */


		// Create RTV descriptor heap
        rtvDescriptorHeap.Initialize(renderDevice.device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, m_FrameCount);

        for (uint32_t i = 0; i < m_FrameCount; ++i)
        {
            ID3D12Resource* backBuffer = nullptr;
            renderDevice.swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer));


            D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvDescriptorHeap.GetCPUDescriptorHandleForHeapStart();
            rtvHandle.ptr += i * rtvDescriptorHeap.GetDescriptorSize();
            renderDevice.device->CreateRenderTargetView(backBuffer, nullptr, rtvHandle);

            renderDevice.renderTargets[i] = backBuffer;
        }




        /*
            +--------------------+
            |  Command Allocator |
            +--------------------+
                     |
                     v
            +------------------+
            |  Command List    |
            +------------------+
                     |
                     v
            +--------------------------+
            | ClearRenderTargetView()  |
            | Draw()                   |
            | Copy()                   |
            | Dispatch()               |
			| Etc...                   |
            +--------------------------+
                     |
                     v
            +------------------+
            | Close()          |
            +------------------+
                     |
                     v
            ExecuteCommandLists()
        */

        renderDevice.device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&renderDevice.commandAlloc));
        renderDevice.device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, renderDevice.commandAlloc, nullptr, IID_PPV_ARGS(&renderDevice.commandList));

        renderDevice.commandList->Close();

        return true;
    }


    void Loop()
    {
		// get the current back buffer index
		uint32_t backBufferIndex = renderDevice.swapChain->GetCurrentBackBufferIndex();

		// Reset the command allocator and command list for the current frame
        renderDevice.commandAlloc->Reset();
        renderDevice.commandList->Reset(renderDevice.commandAlloc, nullptr);



		// Set the render target view (RTV) for the current back bufferq
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvDescriptorHeap.GetCPUHandle(backBufferIndex);
        renderDevice.commandList->OMSetRenderTargets(1, &rtvHandle, false, nullptr);

		// Clear the render target
        float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
        renderDevice.commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);


        renderDevice.commandList->Close();

		// Execute the command list
        ID3D12CommandList* ppCommandLists[] = { renderDevice.commandList };
        renderDevice.commandQueue->ExecuteCommandLists(1, ppCommandLists);

		// Present the frame
        renderDevice.swapChain->Present(1, 0);
    }


    void Cleanup()
    {
        for (uint32_t i = 0; i < 2; ++i)
            if (renderDevice.renderTargets[i])
                renderDevice.renderTargets[i]->Release();

        rtvDescriptorHeap.Destroy();

        if (renderDevice.swapChain)
            renderDevice.swapChain->Release();

        if (renderDevice.commandQueue)
            renderDevice.commandQueue->Release();

        if (renderDevice.device)
            renderDevice.device->Release();

        if (renderDevice.commandAlloc)
            renderDevice.commandAlloc->Release();

        if (renderDevice.commandList)
            renderDevice.commandList->Release();
    }


};

int main()
{
	uint32_t width = 1280;
	uint32_t height = 820;
    const wchar_t title[] = L"DX12 ClearScreen";

    Render render {};



    WNDCLASSEX wcex = { sizeof(WNDCLASSEX), CS_CLASSDC, DefWindowProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, title, nullptr };
    RegisterClassEx(&wcex);

	// Create a window
    HWND hWnd = CreateWindow(title, title, WS_OVERLAPPEDWINDOW, 100, 100, width, height, nullptr, nullptr, wcex.hInstance, nullptr);

    ShowWindow(hWnd, SW_SHOW);

    if (!render.Initialize(hWnd, width, height))
    {
        std::cerr << "Error inicializando DirectX 12" << std::endl;
        return 1;
    }

    MSG msg = { 0 };
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            render.Loop();
        }
    }


	render.Cleanup();

    return 0;
}
