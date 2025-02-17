// ContainerProperty.h : Declaration of the CContainerProperty

#pragma once
#include "resource.h"       // main symbols
#include <vector>

#include "UCDebuggerSDK.h"
#include "DebugProperty.h"

//////////////////////////////////////////////////////////////////////////////
// Helper class based on general ATL template class (_Copy).  This class makes
// sure the DEBUG_PROPERTY_INFO::pProperty field is properly disposed of.

class CPropertyInfoCopy
{
    public:
        static void init(DEBUG_PROPERTY_INFO *p)
        {
            _Copy<DEBUG_PROPERTY_INFO>::init(p);
        }

        static HRESULT copy(DEBUG_PROPERTY_INFO *pTo, const DEBUG_PROPERTY_INFO *pFrom)
        {
            HRESULT hr = _Copy<DEBUG_PROPERTY_INFO>::copy(pTo,const_cast<DEBUG_PROPERTY_INFO *>(pFrom));
            if (SUCCEEDED(hr))
            {
                if (pTo->pProperty != NULL)
                {
                    pTo->pProperty->AddRef();
                }
            }
            return(hr);
        }

        static void destroy(DEBUG_PROPERTY_INFO *p)
        {
            if (NULL == p)
                return;
            if (p->pProperty != NULL)
            {
                p->pProperty->Release();
            }
            _Copy<DEBUG_PROPERTY_INFO>::destroy(p);
        }
};

//////////////////////////////////////////////////////////////////////////////
// CEnumDebugPropertyInfo
typedef CComEnumWithCount<
    IEnumDebugPropertyInfo2,
    &IID_IEnumDebugPropertyInfo2,
    DEBUG_PROPERTY_INFO,
    CPropertyInfoCopy
> CEnumDebugPropertyInfo;

typedef std::vector<CDebugProperty *> PropertyList;



#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error Single-threaded COM objects are not properly supported on Windows CE platforms that do not include full DCOM support.  Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support them anyway. You must also change the threading model in your rgs file from 'Apartment'/'Single' to 'Free'.
#endif



//////////////////////////////////////////////////////////////////////////////
// CContainerProperty
// (A container property is an IDebugProperty2 with children.)

class CContainerProperty : 
    public CDebugProperty
{
public:
    CContainerProperty()
    {
    }

    // Enum the children of this property
    STDMETHOD(EnumChildren)(
        DEBUGPROP_INFO_FLAGS dwFields,
        DWORD dwRadix,
        REFGUID guidFilter,
        DBG_ATTRIB_FLAGS dwAttribFilter,
        LPCOLESTR pszNameFilter,
        DWORD dwTimeout,
        IEnumDebugPropertyInfo2** ppEnum);

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    HRESULT FinalConstruct()
    {
        return S_OK;
    }
    
    void FinalRelease() 
    {
        PropertyList::iterator propIter = m_childrenList.begin();
        while (propIter != m_childrenList.end()) {
            (*propIter)->Release();
            ++propIter;
        }
        m_childrenList.clear();
    }

public:
    void AddChild(CDebugProperty *pChild);

protected:
    PropertyList m_childrenList;
};
