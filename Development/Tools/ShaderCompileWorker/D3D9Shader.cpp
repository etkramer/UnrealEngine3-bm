#include "ShaderCompileWorker.h"
#include <d3d9.h>
#include <d3dx9.h>

const BYTE D3D9ShaderCompileWorkerInputVersion = 0;
const BYTE D3D9ShaderCompileWorkerOutputVersion = 0;
const UINT REQUIRED_D3DX_SDK_VERSION = 35;

/**
* An implementation of the D3DX include interface
*/
class FD3D9IncludeEnvironment : public ID3DXInclude
{
public:
	vector<FInclude> Includes;

	STDMETHOD(Open)(D3DXINCLUDE_TYPE Type,LPCSTR Name,LPCVOID ParentData,LPCVOID* Data,UINT* Bytes)
	{
		bool bFoundInclude = false;
		for (UINT IncludeIndex = 0; IncludeIndex < Includes.size(); IncludeIndex++)
		{
			if (Includes[IncludeIndex].IncludeName == Name)
			{
				bFoundInclude = true;
				*Data = _strdup(Includes[IncludeIndex].IncludeFile.c_str());
				*Bytes = (UINT)Includes[IncludeIndex].IncludeFile.size();
				break;
			}
		}

		if (!bFoundInclude)
		{
			string IncludeFilePath = IncludePath + "\\" + Name;
			FILE* IncludeFile = NULL;
			errno_t ErrorCode = fopen_s(&IncludeFile, IncludeFilePath.c_str(), "rb");
			if (ErrorCode == 0)
			{
				CHECKF(IncludeFile != NULL, TEXT("Couldn't open include file"));
				int FileId = _fileno(IncludeFile);
				CHECKF(FileId >= 0, TEXT("Couldn't get file id for the include file"));
				long FileLength = _filelength(FileId);
				LPSTR IncludeFileContents = new CHAR[FileLength + 1];
				size_t NumRead = fread(IncludeFileContents,sizeof(CHAR),FileLength,IncludeFile);
				CHECKF(NumRead == FileLength, TEXT("Failed to read the whole include file"));
				*Data = IncludeFileContents;
				*Bytes = (UINT)NumRead;
				bFoundInclude = true;
				fclose(IncludeFile);
			}
		}

		return bFoundInclude ? S_OK : S_FALSE;
	}

	STDMETHOD(Close)(LPCVOID Data)
	{
		delete [] Data;
		return D3D_OK;
	}


	FD3D9IncludeEnvironment(const string& InIncludePath) :
	IncludePath(InIncludePath)
	{}


private:

	string IncludePath;
};

/**
 * This wraps D3DXCompileShader in a __try __except block to catch occasional crashes in the function.
 */
static HRESULT D3D9SafeCompileShader(
	LPCSTR pSrcData,
	UINT srcDataLen,
	D3DXMACRO* Macros,
	LPD3DXINCLUDE pInclude,
	LPCSTR pFunctionName,
	LPCSTR pProfile,
	DWORD Flags,
	LPD3DXBUFFER* ppShader,
	LPD3DXBUFFER* ppErrorMsgs
	)
{
	__try
	{
		return D3DXCompileShader(
			pSrcData,
			srcDataLen,			
			Macros,
			pInclude,
			pFunctionName,
			pProfile,
			Flags,
			ppShader,
			ppErrorMsgs,
			NULL
			);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		Log(TEXT("D3DXCompileShader threw an exception, trying the legacy compiler"));
		for (INT i = 0; Macros[i].Name != NULL; i++)
		{
			if (strcmp(Macros[i].Name, "COMPILER_SUPPORTS_ATTRIBUTES") == 0)
			{
				delete [] Macros[i].Definition;
				Macros[i].Definition = _strdup("0");
			}
		}

		__try
		{
			return D3DXCompileShader(
				pSrcData,
				srcDataLen,			
				Macros,
				pInclude,
				pFunctionName,
				pProfile,
				Flags | D3DXSHADER_USE_LEGACY_D3DX9_31_DLL,
				ppShader,
				ppErrorMsgs,
				NULL
				);
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			ERRORF(TEXT("D3DXCompileShader threw an exception with the legacy compiler!"));
			return E_FAIL;
		}
	}
}

/** Wraps D3DXDisassembleShader in a __try __except block to catch unpredictable crashes in the function. */
static HRESULT D3D9SafeDisassembleShader(
	CONST DWORD * pShader,
	BOOL EnableColorCode,
	LPCSTR pComments,
	LPD3DXBUFFER* ppDisassembly
	)
{
	__try
	{
		return D3DXDisassembleShader(pShader, EnableColorCode, pComments, ppDisassembly);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		ERRORF( TEXT("Caught exception from D3DXDisassembleShader"));
		return -1;
	}
}

/** Wraps GetConstantDesc in a __try __except block to catch unpredictable crashes in the function. */
static HRESULT D3D9SafeGetConstantDesc(
									   ID3DXConstantTable* ConstantTable,
									   D3DXHANDLE ConstantHandle,
									   D3DXCONSTANT_DESC* ConstantDesc,
									   UINT* NumConstants
									   )
{
	__try
	{
		return ConstantTable->GetConstantDesc(ConstantHandle,ConstantDesc,NumConstants);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		ERRORF( TEXT("Caught exception from ConstantTable->GetConstantDesc()"));
		return -1;
	}
}

/** A wrapper for D3DXCONSTANT_DESC with an embedded copy of the name, so it can be serialized. */
#pragma pack(push,4)
struct FD3D9NamedConstantDesc
{
	enum { MaxNameLength = 128 };

	char Name[MaxNameLength];
	D3DXCONSTANT_DESC ConstantDesc;
};
#pragma pack(pop)

bool D3D9CompileShader(BYTE InputVersion, FILE* InputFile, vector<BYTE>& OutputData)
{
	CHECKF(InputVersion == D3D9ShaderCompileWorkerInputVersion, TEXT("Wrong job version for D3D9 shader"));
	CHECKF(D3DX_SDK_VERSION == REQUIRED_D3DX_SDK_VERSION, TEXT("Compiled with wrong DX SDK"));

	// Read the input from UE3
	string SourceFile;
	ParseAnsiString(InputFile, SourceFile);

	string FunctionName;
	ParseAnsiString(InputFile, FunctionName);

	string ShaderProfile;
	ParseAnsiString(InputFile, ShaderProfile);

	DWORD CompileFlags;
	ReadValue(InputFile, CompileFlags);

	string IncludePath;
	ParseAnsiString(InputFile, IncludePath);
	FD3D9IncludeEnvironment IncludeEnvironment(IncludePath);

	UINT NumIncludes;
	ReadValue(InputFile, NumIncludes);

	for (UINT IncludeIndex = 0; IncludeIndex < NumIncludes; IncludeIndex++)
	{
		FInclude NewInclude;
		ParseAnsiString(InputFile, NewInclude.IncludeName);
		ParseAnsiString(InputFile, NewInclude.IncludeFile);
		IncludeEnvironment.Includes.push_back(NewInclude);
	}

	UINT NumMacros;
	ReadValue(InputFile, NumMacros);

	vector<D3DXMACRO> Macros;
	for (UINT MacroIndex = 0; MacroIndex < NumMacros; MacroIndex++)
	{
		D3DXMACRO NewMacro;
		UINT NameLen;
		ParseAnsiString(InputFile, NewMacro.Name, NameLen);
		UINT DefinitionLen;
		ParseAnsiString(InputFile, NewMacro.Definition, DefinitionLen);
		Macros.push_back(NewMacro);
	}

	D3DXMACRO TerminatorMacro;
	TerminatorMacro.Name = NULL;
	TerminatorMacro.Definition = NULL;
	Macros.push_back(TerminatorMacro);

	ID3DXBuffer* ShaderByteCode = NULL;
	ID3DXBuffer* Errors = NULL;

	// Compile the shader through D3DX
	HRESULT hr = D3D9SafeCompileShader(
		SourceFile.c_str(),
		(UINT)SourceFile.size(),
		&Macros.front(),
		&IncludeEnvironment,
		FunctionName.c_str(),
		ShaderProfile.c_str(),
		CompileFlags,
		&ShaderByteCode,
		&Errors
		);

	for (UINT MacroIndex = 0; MacroIndex < Macros.size(); MacroIndex++)
	{
		delete [] Macros[MacroIndex].Name;
		delete [] Macros[MacroIndex].Definition;
	}

	vector<FD3D9NamedConstantDesc> Constants;
	ID3DXBuffer* DisassemblyBuffer = NULL;
	if(SUCCEEDED(hr))
	{
		// Read the constant table output from the shader bytecode.
		ID3DXConstantTable* ConstantTable = NULL;
		VERIFYF(SUCCEEDED(D3DXGetShaderConstantTable((DWORD*)ShaderByteCode->GetBufferPointer(), &ConstantTable)), TEXT("Failed to read shader constant table.") );
		D3DXCONSTANTTABLE_DESC ConstantTableDesc;
		VERIFYF(SUCCEEDED(ConstantTable->GetDesc(&ConstantTableDesc)), TEXT("Failed to describe shader constant table."));

		// Read the constant descriptions out of the shader bytecode.
		for(UINT ConstantIndex = 0;ConstantIndex < ConstantTableDesc.Constants;ConstantIndex++)
		{
			// Read the constant description.
			D3DXHANDLE ConstantHandle = ConstantTable->GetConstant(NULL,ConstantIndex);
			D3DXCONSTANT_DESC ConstantDesc;
			UINT NumConstants = 1;
			VERIFYF(SUCCEEDED(D3D9SafeGetConstantDesc(ConstantTable, ConstantHandle, &ConstantDesc, &NumConstants)), TEXT("Failed to describe shader constant."));

			// Copy the constant and its name into a self-contained data structure, and add it to the constant array.
			FD3D9NamedConstantDesc NamedConstantDesc;
			strncpy_s(NamedConstantDesc.Name,FD3D9NamedConstantDesc::MaxNameLength,ConstantDesc.Name,_TRUNCATE);
			NamedConstantDesc.ConstantDesc = ConstantDesc;
			Constants.push_back(NamedConstantDesc);
		}

		// Release the constant table.
		ConstantTable->Release();

		// Disassemble the shader to determine the instruction count.
		VERIFYF(SUCCEEDED(D3D9SafeDisassembleShader((const DWORD*)ShaderByteCode->GetBufferPointer(),FALSE,NULL,&DisassemblyBuffer)), TEXT("Failed to disassembly shader bytecode."));
	}

	const EWorkerJobType JobType = WJT_D3D9Shader;
	const UINT ByteCodeLength = SUCCEEDED(hr) ? ShaderByteCode->GetBufferSize() : 0;
	const UINT ErrorStringLength = Errors == NULL ? 0 : Errors->GetBufferSize();
	const UINT ConstantArrayLength = (UINT)(Constants.size() * sizeof(FD3D9NamedConstantDesc));
	const UINT DisassemblyLength = DisassemblyBuffer ? DisassemblyBuffer->GetBufferSize() : 0;

	// Write the output for UE3
	WriteValue(OutputData, D3D9ShaderCompileWorkerOutputVersion);
	WriteValue(OutputData, JobType);
	WriteValue(OutputData, hr);
	WriteValue(OutputData, ByteCodeLength);
	if (ByteCodeLength > 0)
	{
		WriteArray(OutputData, ShaderByteCode->GetBufferPointer(), ByteCodeLength);
	}
	WriteValue(OutputData, ErrorStringLength);
	if (ErrorStringLength > 0)
	{
		WriteArray(OutputData, Errors->GetBufferPointer(), ErrorStringLength);
	}

	WriteValue(OutputData, ConstantArrayLength );
	if(ConstantArrayLength > 0)
	{
		WriteArray(OutputData, &Constants[0], ConstantArrayLength );
	}

	WriteValue(OutputData, DisassemblyLength );
	if(DisassemblyLength > 0)
	{
		WriteArray(OutputData,DisassemblyBuffer->GetBufferPointer(),DisassemblyLength);
	}

	// Cleanup
	if (ShaderByteCode != NULL)
	{
		ShaderByteCode->Release();
	}

	if (Errors != NULL)
	{
		Errors->Release();
	}

	if(DisassemblyBuffer)
	{
		DisassemblyBuffer->Release();
	}

	return true;
}
