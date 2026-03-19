#pragma once

#include <wrl/client.h>
#include <string>
#include <vector>
#include <dxcapi.h>
#include <d3dcommon.h>

using Microsoft::WRL::ComPtr;
namespace Core
{
	class ShaderCompiler
	{
	public:
		ShaderCompiler()
		{

		}

		ID3DBlob* Compile(const WCHAR* filename, const WCHAR* targetString);

	private:


	};


}


