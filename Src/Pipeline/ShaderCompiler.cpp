#include "ShaderCompiler.h"
//#include <dxcapi.h>
#include <dxcapi.h>

namespace Core
{
    ID3DBlob* ShaderCompiler::Compile(const WCHAR* filename, const WCHAR* targetString)
    {
        IDxcCompiler* pCompiler = nullptr;
        IDxcLibrary* pLibrary = nullptr;

        DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&pCompiler));
        DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&pLibrary));

        IDxcBlobEncoding* pTextBlob = nullptr;
        HRESULT hr = pLibrary->CreateBlobFromFile(filename, nullptr, &pTextBlob);

        if (FAILED(hr))
        {
            OutputDebugStringA("Error\n");
            return nullptr;
        }

        IDxcIncludeHandler* pIncludeHandler = nullptr;
        pLibrary->CreateIncludeHandler(&pIncludeHandler);

        IDxcOperationResult* pResult = nullptr;

        pCompiler->Compile(
            pTextBlob,
            filename,
            L"",
            targetString,
            nullptr,
            0,
            nullptr,
            0,
            pIncludeHandler,
            &pResult
        );

        HRESULT resultCode;
        pResult->GetStatus(&resultCode);

        if (FAILED(resultCode))
        {
            IDxcBlobEncoding* pErrors;
            pResult->GetErrorBuffer(&pErrors);

            if (pErrors)
            {
                OutputDebugStringA((char*)pErrors->GetBufferPointer());
            }

            return nullptr;
        }

        IDxcBlob* pBlob = nullptr;
        pResult->GetResult(&pBlob);

        return reinterpret_cast<ID3DBlob*>(pBlob);
    }
}