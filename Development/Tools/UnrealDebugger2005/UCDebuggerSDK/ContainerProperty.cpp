// ContainerProperty.cpp : Implementation of CContainerProperty

#include "stdafx.h"
#include "ContainerProperty.h"


//////////////////////////////////////////////////////////////////////////////
// IDebugProperty2 interface

    // Enum the children of this property
HRESULT CContainerProperty::EnumChildren(
        DEBUGPROP_INFO_FLAGS dwFields,
        DWORD dwRadix,
        REFGUID guidFilter,
        DBG_ATTRIB_FLAGS dwAttribFilter,
        LPCOLESTR pszNameFilter,
        DWORD dwTimeout,
        IEnumDebugPropertyInfo2** ppEnum)
{
    HRESULT retval = E_POINTER;
    if (ppEnum != NULL)
    {
        retval = S_FALSE;
        if (!m_childrenList.empty()) {
            CComObject<CEnumDebugPropertyInfo> *pEnumPropertyInfo;
            retval = CComObject<CEnumDebugPropertyInfo>::CreateInstance(&pEnumPropertyInfo);
            if ( SUCCEEDED(retval) && (pEnumPropertyInfo != NULL) ) {
                pEnumPropertyInfo->AddRef();
                DEBUG_PROPERTY_INFO *pInfoList = new DEBUG_PROPERTY_INFO[m_childrenList.size()];
                if (NULL == pInfoList) {
                    retval = E_OUTOFMEMORY;
                } else {
                    int i = 0;
                    PropertyList::iterator propIter = m_childrenList.begin();
                    while (propIter != m_childrenList.end()) {
                        (*propIter)->GetPropertyInfo(dwFields,dwRadix,dwTimeout,NULL,0,&pInfoList[i]);
                        ++propIter;
                        ++i;
                    }
                    pEnumPropertyInfo->Init(pInfoList,pInfoList + m_childrenList.size(),NULL,AtlFlagCopy);
                    retval = pEnumPropertyInfo->QueryInterface(__uuidof(IEnumDebugPropertyInfo2),(LPVOID *)ppEnum);
                    pEnumPropertyInfo->Release();
                    // Make sure any property interfaces are released before
                    // deleting the array.
                    for (size_t infoIndex = 0; infoIndex < m_childrenList.size(); infoIndex++) {
                        CPropertyInfoCopy::destroy(&pInfoList[infoIndex]);
                    }
                    delete[] pInfoList;
                }
            }
        }
    }
    return retval;
}

//////////////////////////////////////////////////////////////////////////////
// CContainerProperty methods.

void CContainerProperty::AddChild(CDebugProperty *pChild)
{
    if (pChild != NULL) {
        m_childrenList.push_back(pChild);
        pChild->AddRef();
        pChild->SetParent(this);
        m_dwAttrib |= DBG_ATTRIB_OBJ_IS_EXPANDABLE;
    }
}
