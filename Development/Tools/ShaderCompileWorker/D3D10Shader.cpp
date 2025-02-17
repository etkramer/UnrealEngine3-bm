#include "ShaderCompileWorker.h"
#include <d3d10.h>
#include <d3dx10.h>

const BYTE D3D10ShaderCompileWorkerInputVersion = 0;
const BYTE D3D10ShaderCompileWorkerOutputVersion = 0;
const UINT REQUIRED_D3DX_SDK_VERSION = 35;

/**
 * An implementation of the D3DX include interface to access a FShaderCompilerEnvironment.
 */
class FD3D10IncludeEnvironment : public ID3D10Include
{
public:

	vector<FInclude> Includes;

	STDMETHOD(Open)(D3D10_INCLUDE_TYPE Type,LPCSTR Name,LPCVOID ParentData,LPCVOID* Data,UINT* Bytes)
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
		return S_OK;
	}

	FD3D10IncludeEnvironment(const string& InIncludePath) :
		IncludePath(InIncludePath)
	{}

private:

	string IncludePath;
};

/**
 * This wraps D3DX10CompileFromMemory in a __try __except block to catch occasional crashes in the function.
 */
static HRESULT D3D10SafeCompileShader(
	LPCSTR SourceFileName,
	LPCSTR pSrcData,
	UINT srcDataLen,
	const D3D10_SHADER_MACRO* Defines,
	ID3D10Include* pInclude,
	LPCSTR pFunctionName,
	LPCSTR pProfile,
	DWORD Flags,
	ID3D10Blob** ppShader,
	ID3D10Blob** ppErrorMsgs
	)
{
	__try
	{
		return D3DX10CompileFromMemory(
			pSrcData,
			srcDataLen,
			SourceFileName,
			Defines,
			pInclude,
			pFunctionName,
			pProfile,
			Flags,
			0,
			NULL,
			ppShader,
			ppErrorMsgs,
			NULL
			);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		ERRORF(TEXT("D3DXCompileShader threw an exception"));
		return E_FAIL;
	}
}

bool D3D10CompileShader(BYTE InputVersion, FILE* InputFile, vector<BYTE>& OutputData)
{
	CHECKF(InputVersion == D3D10ShaderCompileWorkerInputVersion, TEXT("Wrong job version for D3D10 shader"));
	CHECKF(D3DX10_SDK_VERSION == REQUIRED_D3DX_SDK_VERSION, TEXT("Compiled with wrong DX SDK"));

	string SourceFileName;
	ParseAnsiString(InputFile, SourceFileName);

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
	FD3D10IncludeEnvironment IncludeEnvironment(IncludePath);

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

	vector<D3D10_SHADER_MACRO> Macros;
	for (UINT MacroIndex = 0; MacroIndex < NumMacros; MacroIndex++)
	{
		D3D10_SHADER_MACRO NewMacro;
		UINT NameLen;
		ParseAnsiString(InputFile, NewMacro.Name, NameLen);
		UINT DefinitionLen;
		ParseAnsiString(InputFile, NewMacro.Definition, DefinitionLen);
		Macros.push_back(NewMacro);
	}

	D3D10_SHADER_MACRO TerminatorMacro;
	TerminatorMacro.Name = NULL;
	TerminatorMacro.Definition = NULL;
	Macros.push_back(TerminatorMacro);

	ID3D10Blob* ShaderByteCode = NULL;
	ID3D10Blob* Errors = NULL;

	HRESULT hr = D3D10SafeCompileShader(
		SourceFileName.c_str(),
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

	EWorkerJobType JobType = WJT_D3D10Shader;
	UINT ByteCodeLength = SUCCEEDED(hr) ? (UINT)ShaderByteCode->GetBufferSize() : 0;
	UINT ErrorStringLength = Errors == NULL ? 0 : (UINT)Errors->GetBufferSize();

	WriteValue(OutputData, D3D10ShaderCompileWorkerOutputVersion);
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

	if (ShaderByteCode != NULL)
	{
		ShaderByteCode->Release();
	}

	if (Errors != NULL)
	{
		Errors->Release();
	}

	return true;
}
