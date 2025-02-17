//--------------------------------------------------------------------------------------
// File: Scripting.cpp
//
// Scripting code for Managed Scripting sample
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "DXUT.h"

#pragma managed

// Undef this
#ifdef GetTempFileName
#undef GetTempFileName
#endif

// User defined types
#define SCRIPT_TYPE_NAME S"ScriptClass"
#define SCRIPT_UPDATE_POSITION S"UpdatePosition"
#define SCRIPT_UPDATE_ANGLEX S"UpdateRotationX"
#define SCRIPT_UPDATE_ANGLEY S"UpdateRotationY"
#define SCRIPT_UPDATE_ANGLEZ S"UpdateRotationZ"

// We will need to have the CLR loaded
#using <mscorlib.dll>
#using <system.dll>

// These are the namespaces required
using namespace System;
using namespace System::IO;
using namespace System::Reflection;
using namespace System::CodeDom;
using namespace System::CodeDom::Compiler;
// Security namespaces
using namespace System::Security;
using namespace System::Security::Policy;
using namespace System::Security::Permissions;
// For code compiler
using namespace Microsoft::CSharp;

__sealed public __gc class ScriptEngine
{
private:
    static Assembly* pScriptAssembly = NULL; // The assembly that hold the scripts
    static Type* pScriptType = NULL; // The class that holds the scripts
    static String* pSavedFile = NULL; // The assembly saved location

    static bool canUpdatePosition = true;
    static bool canUpdateRotationX = true;
    static bool canUpdateRotationY = true;
    static bool canUpdateRotationZ = true;
    static bool hasNotifiedSecurityException = false;

    // Reset script options
    static void ResetObjects()
    {
        pScriptAssembly = NULL;
        pScriptType = NULL;
        canUpdatePosition = true;
        canUpdateRotationX = true;
        canUpdateRotationY = true;
        canUpdateRotationZ = true;
        hasNotifiedSecurityException = false;

        // Delete the saved file if possible
        if ( (pSavedFile != NULL) && (pSavedFile->Length > 0) )
        {
            try
            {
                File::Delete(pSavedFile);
            }
            catch(Exception*)
            {
                // Ignore
            }
            pSavedFile = NULL;
        }
    }

public:
    // Initialize the scripting engine's app domain
    static HRESULT Initialize()
    {
        try
        {
            // Create the security policy level for this application domain
            PolicyLevel* pSecurityLevel = PolicyLevel::CreateAppDomainLevel();

            // Create a new, empty permission set so we don't mistakenly grant some permission we don't want
            PermissionSet* pPermissions = new PermissionSet(PermissionState::None);

            // Set the permissions that you will allow, in this case we only want to allow execution of code
            pPermissions->AddPermission(new SecurityPermission(SecurityPermissionFlag::Execution));
                                                                           
            // Make sure we have the permissions currently
            pPermissions->Demand();

            // Give the policy level's root code group a new policy statement based
            // on the new permission set.
            pSecurityLevel->RootCodeGroup->PolicyStatement = new PolicyStatement(pPermissions);

            // Update the application domain's policy now
            AppDomain::CurrentDomain->SetAppDomainPolicy(pSecurityLevel);
        }
        catch(Exception*)
        {
            return E_FAIL;
        }

        return S_OK;
    }

    static HRESULT LoadScriptData(System::String* pScriptName)
    {
        // Make sure to reset the objects first
        ResetObjects();

        // Make sure a valid name was passed in
        if (pScriptName == NULL || pScriptName->Length == 0)
            return E_INVALIDARG;
        // First try to open the script file
        StreamReader* pReader = NULL;
        try
        {
            pReader = new StreamReader(pScriptName);

            // Now try to read the text
            System::String* pScriptText = pReader->ReadToEnd();
            
            // Make sure something was read
            if (pScriptText == NULL || pScriptText->Length == 0)
                return E_INVALIDARG;

            // First create the compilers
            CSharpCodeProvider* pProvider = new CSharpCodeProvider();
#ifndef VS2005 // VS2005 this method is deprecated
            ICodeCompiler* pCompiler = pProvider->CreateCompiler();
#endif
            CompilerParameters* pParams = new CompilerParameters();

            // Now try to compile this script, first make sure we compile an assembly, not exe
            pParams->GenerateExecutable = false;

            // Make sure the assembly is generated in the temp files, first get a temp file name
            pSavedFile = System::String::Concat(Path::GetTempFileName(), S".dll");
            // Next mark it as not being generated in memory
            pParams->GenerateInMemory = false;
            // Finally, update the output file
            pParams->OutputAssembly = pSavedFile;
            #if defined(DEBUG) | defined(_DEBUG)
            // If you want to include debug information, do so
            pParams->IncludeDebugInformation = true;
            #endif

            // Finally compile the code
#ifndef VS2005 // VS2005 this method is deprecated
            CompilerResults* pResult = pCompiler->CompileAssemblyFromSource(pParams, pScriptText);
#else
            CompilerResults* pResult = pProvider->CompileAssemblyFromSource(pParams, new System::String* __gc[] { pScriptText } );
#endif            
            if (pResult->Errors->Count > 0)
            {
                // Some errors occurred
                return E_FAIL;
            }
            else
            {
                // The compilation was a success, load the assembly 
                pScriptAssembly = System::Reflection::Assembly::LoadFrom(pResult->PathToAssembly);
                // Try to load the type
                pScriptType = pScriptAssembly->GetType(SCRIPT_TYPE_NAME, false, true);
                if (pScriptType == NULL)
                {
                    // Couldn't find the type, fail
                    pScriptAssembly = NULL;
                    pScriptType = NULL;
                    return E_FAIL;
                }
                
                // Finished
                return S_OK;
            }
        }
        catch (Exception* e)
        {
            System::Diagnostics::Debugger::Log(0, NULL, e->ToString());
            // Reset objects
            ResetObjects();
            // Reading the script file failed, return failure
            return E_FAIL;
        }
        __finally
        {
            // Make sure to close the file down no matter what
            if (pReader != NULL)
            {
                IDisposable* pDisposeObject = dynamic_cast<IDisposable*>(pReader);
                if (pDisposeObject != NULL)
                    pDisposeObject->Dispose();
            }
        }

    }
    static HRESULT UpdatePlayerPosition(double appTime, float __nogc* pX, float __nogc* pY, float __nogc* pZ)
    {
        // Check params
        if (pX == NULL || pY == NULL || pZ == NULL)
            return E_INVALIDARG;

        // Check options
        if (pScriptAssembly == NULL || pScriptType == NULL)
            return E_FAIL;

        // Can the position be updated?
        if (!canUpdatePosition)
            return E_FAIL;

        // Now try to call the method
        try
        {
            // Create the parameters
            System::Object* pParams __gc[] = { __box(appTime), __box(*pX), __box(*pY), __box(*pZ) };
            pScriptType->InvokeMember(SCRIPT_UPDATE_POSITION, (BindingFlags)(BindingFlags::InvokeMethod | BindingFlags::DeclaredOnly |
                BindingFlags::Public | BindingFlags::NonPublic | BindingFlags::Static), NULL, NULL, pParams);

            // The method succeeded, update the data
            *pX = *static_cast<__box float*>(pParams[1]);
            *pY = *static_cast<__box float*>(pParams[2]);
            *pZ = *static_cast<__box float*>(pParams[3]);
        }
        catch(TargetInvocationException* e)
        {
            if ( (dynamic_cast<SecurityException*>(e->InnerException) != NULL) 
                || (dynamic_cast<PolicyException*>(e->InnerException) != NULL) )
            {
                // Special case the security exceptions
                if (!hasNotifiedSecurityException)
                {
                    bool bWindowed = DXUTIsWindowed();
                    if( !bWindowed )
                        DXUTToggleFullScreen();

                    MessageBox(NULL, L"This script has attempted to do something the security system will not allow.  Please verify this script is safe to run.", 
                        L"Security Exception", MB_OK);
                    hasNotifiedSecurityException = true;

                    if( !bWindowed )
                        DXUTToggleFullScreen();
                }
            }
            canUpdatePosition = false;
            // Invoke failed
            return E_FAIL;
        }

        return S_OK;
    }
    static HRESULT UpdatePlayerRotation(System::String* pMethodName, double appTime, float __nogc* pAngle)
    {
        // Check params
        if (pAngle == NULL)
            return E_INVALIDARG;

        // Make sure a valid name was passed in
        if (pMethodName == NULL || pMethodName->Length == 0)
            return E_INVALIDARG;

        // Check options
        if (pScriptAssembly == NULL || pScriptType == NULL)
            return E_FAIL;

        // Can we call?
        if (pMethodName->Equals(SCRIPT_UPDATE_ANGLEX))
            if (!canUpdateRotationX)
                return E_FAIL;
        if (pMethodName->Equals(SCRIPT_UPDATE_ANGLEY))
            if (!canUpdateRotationY)
                return E_FAIL;
        if (pMethodName->Equals(SCRIPT_UPDATE_ANGLEZ))
            if (!canUpdateRotationZ)
                return E_FAIL;

        // Now try to call the method
        try
        {
            // Create the parameters
            System::Object* pParams __gc[] = { __box(appTime), __box(*pAngle) };
            pScriptType->InvokeMember(pMethodName, (BindingFlags)(BindingFlags::InvokeMethod | BindingFlags::DeclaredOnly |
                BindingFlags::Public | BindingFlags::NonPublic | BindingFlags::Static), NULL, NULL, pParams);

            // The method succeeded, update the data
            *pAngle = *static_cast<__box float*>(pParams[1]);
        }
        catch(Exception*)
        {
            if (pMethodName->Equals(SCRIPT_UPDATE_ANGLEX))
                canUpdateRotationX = false;
            if (pMethodName->Equals(SCRIPT_UPDATE_ANGLEY))
                canUpdateRotationY = false;
            if (pMethodName->Equals(SCRIPT_UPDATE_ANGLEZ))
                canUpdateRotationZ = false;
            // Invoke failed
            return E_FAIL;
        }

        return S_OK;
    }
};

// Initializes the scripting engine
HRESULT InitializeScriptEngine()
{
    return ScriptEngine::Initialize();
}

// Call in to load a script
HRESULT LoadScript(LPWSTR pScriptFileName)
{
    return ScriptEngine::LoadScriptData(pScriptFileName);
}

// Call in to the update player position script
HRESULT UpdatePlayerPosition(double appTime, float __nogc* pX, float __nogc* pY, float __nogc* pZ)
{
    return ScriptEngine::UpdatePlayerPosition(appTime, pX, pY, pZ);
}

// Call in to the update player rotation script
HRESULT UpdatePlayerRotationX(double appTime, D3DXMATRIX __nogc* pMatrix)
{
    // Calculate the new rotation angle
    float angle;
    if ( SUCCEEDED(ScriptEngine::UpdatePlayerRotation(SCRIPT_UPDATE_ANGLEX, appTime, &angle)) )
    {
        D3DXMATRIX mRotation;
        // Rotate the X
        D3DXMatrixRotationX(&mRotation, angle);
        D3DXMatrixMultiply(pMatrix, pMatrix, &mRotation );
    }
    else
        return E_FAIL;

    return S_OK;
}

HRESULT UpdatePlayerRotationY(double appTime, D3DXMATRIX __nogc* pMatrix)
{
    // Calculate the new rotation angle
    float angle;
    if ( SUCCEEDED(ScriptEngine::UpdatePlayerRotation(SCRIPT_UPDATE_ANGLEY, appTime, &angle)) )
    {
        D3DXMATRIX mRotation;
        // Rotate the Y
        D3DXMatrixRotationY(&mRotation, angle);
        D3DXMatrixMultiply(pMatrix, pMatrix, &mRotation );
    }
    else
        return E_FAIL;

    return S_OK;
}

HRESULT UpdatePlayerRotationZ(double appTime, D3DXMATRIX __nogc* pMatrix)
{
    // Calculate the new rotation angle
    float angle;
    if ( SUCCEEDED(ScriptEngine::UpdatePlayerRotation(SCRIPT_UPDATE_ANGLEZ, appTime, &angle)) )
    {
        D3DXMATRIX mRotation;
        // Rotate the Z
        D3DXMatrixRotationZ(&mRotation, angle);
        D3DXMatrixMultiply(pMatrix, pMatrix, &mRotation );
    }
    else
        return E_FAIL;

    return S_OK;
}