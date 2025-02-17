#pragma once

// ----------------------------------------------------------------------------
// CComEnumWithCountImpl
// (ATL-based template to create a COM enumeration that includes a GetCount
// method.)

template <class Base, const IID* piid, class T, class Copy>
class ATL_NO_VTABLE CComEnumWithCountImpl : public Base
{
public:
    CComEnumWithCountImpl()
    {
        m_begin = m_end = m_iter = NULL;
        m_dwFlags = 0;
        m_pUnk = NULL;
    }

    ~CComEnumWithCountImpl()
    {
        if (m_dwFlags & BitOwn)
        {
            for (T* p = m_begin; p != m_end; p++)
                Copy::destroy(p);
            delete [] m_begin;
        }
        if (m_pUnk)
            m_pUnk->Release();
    }

    STDMETHOD(Next)(ULONG celt, T* rgelt, ULONG* pceltFetched)
    {
        if (rgelt == NULL || (celt != 1 && pceltFetched == NULL))
            return E_POINTER;
        if (m_begin == NULL || m_end == NULL || m_iter == NULL)
            return E_FAIL;
        ULONG nRem = (ULONG)(m_end - m_iter);
        HRESULT hRes = S_OK;
        if (nRem < celt)
            hRes = S_FALSE;
        ULONG nMin = min(celt, nRem);
        if (pceltFetched != NULL)
            *pceltFetched = nMin;
        while(nMin--)
            Copy::copy(rgelt++, m_iter++);
        return hRes;
    }
 
    STDMETHOD(Skip)(ULONG celt)
    {
        m_iter += celt;
        if (m_iter < m_end)
            return S_OK;
        m_iter = m_end;
        return S_FALSE;
    }

    STDMETHOD(Reset)(void)
    {
        m_iter = m_begin;
        return S_OK;
    }

    STDMETHOD(Clone)(Base** ppEnum)
    {
        typedef CComObject<CComEnumWithCount<Base, piid, T, Copy> > _class;
        HRESULT hRes = E_POINTER;
        if (ppEnum != NULL)
        {
            _class* p = NULL;
            ATLTRY(p = new _class)
            if (p == NULL)
            {
                *ppEnum = NULL;
                hRes = E_OUTOFMEMORY;
            }
            else
            {
                // If the data is a copy then we need to keep "this" object around
                hRes = p->Init(m_begin, m_end, (m_dwFlags & BitCopy) ? this : m_pUnk);
                if (FAILED(hRes))
                    delete p;
                else
                {
                    p->m_iter = m_iter;
                    hRes = p->_InternalQueryInterface(*piid, (void**)ppEnum);
                    if (FAILED(hRes))
                    delete p;
                }
            }
        }
        return hRes;
    }

    STDMETHOD(GetCount)(ULONG* pcelt)
    {
        if (NULL == pcelt)
            return E_POINTER;
        *pcelt = (ULONG)(m_end - m_begin);
        return S_OK;
    }

    HRESULT Init(T* begin, T* end, IUnknown* pUnk, CComEnumFlags flags = AtlFlagNoCopy)
    {
        if (flags == AtlFlagCopy)
        {
            _ASSERTE(m_begin == NULL); //Init called twice?
            ATLTRY(m_begin = new T[end-begin])
            m_iter = m_begin;
            if (m_begin == NULL)
                return E_OUTOFMEMORY;
            for (T* i=begin; i != end; i++)
            {
                Copy::init(m_iter);
                Copy::copy(m_iter++, i);
            }
            m_end = m_begin + (end-begin);
        }
        else
        {
            m_begin = begin;
            m_end = end;
        }
        m_pUnk = pUnk;
        if (m_pUnk)
            m_pUnk->AddRef();
        m_iter = m_begin;
        m_dwFlags = flags;
        return S_OK;
    }

public:
    IUnknown* m_pUnk;
    T* m_begin;
    T* m_end;
    T* m_iter;
    DWORD m_dwFlags;
protected:
    enum FlagBits { BitCopy = 1, BitOwn = 2 };
};

// ----------------------------------------------------------------------------
// Class to declare a COM enumeration as a COM interface.

template <class Base, const IID* piid, class T, class Copy, class ThreadModel = CComObjectThreadModel>
class ATL_NO_VTABLE CComEnumWithCount :
    public CComEnumWithCountImpl<Base, piid, T, Copy>,
    public CComObjectRootEx< ThreadModel >
{
public:
    typedef CComEnumWithCount<Base, piid, T, Copy > _CComEnum;
    typedef CComEnumWithCountImpl<Base, piid, T, Copy > _CComEnumBase;
    BEGIN_COM_MAP(_CComEnum)
        COM_INTERFACE_ENTRY_IID(*piid, _CComEnumBase)
    END_COM_MAP()
};

// ----------------------------------------------------------------------------
// Class to hold a COM pointer in a apartment safe way.
template <class T>
class CApartmentSafePointer
{
private:
    DWORD dwInterfaceCookie;
    CComPtr<IGlobalInterfaceTable> m_spGIT;

    // Private and not implemented copy constructor and assignement operator.
    // These methods are not implemented on pourpose to avoid their usage.
    CApartmentSafePointer(const CApartmentSafePointer&);
    const CApartmentSafePointer& operator=(const CApartmentSafePointer&);

    void ClearCookie()
    {
        if (m_spGIT && (0!=dwInterfaceCookie))
        {
            m_spGIT->RevokeInterfaceFromGlobal(dwInterfaceCookie);
            dwInterfaceCookie = 0;
        }
    }
public:
    CApartmentSafePointer(T* pInterface = NULL) :
      dwInterfaceCookie(0)
    {
        HRESULT hr = CoCreateInstance(CLSID_StdGlobalInterfaceTable,
                                      NULL,
                                      CLSCTX_INPROC_SERVER,
                                      IID_IGlobalInterfaceTable,
                                      (void **)&m_spGIT);
        if (FAILED(hr) || (NULL==pInterface))
            return;

        m_spGIT->RegisterInterfaceInGlobal(pInterface, __uuidof(T), &dwInterfaceCookie);
    }

    ~CApartmentSafePointer()
    {
        ClearCookie();
    }

    const CApartmentSafePointer& operator=(T* pInterface)
    {
        ClearCookie();
        if ( !m_spGIT )
        {
            // It is possible that the creation of the global interface table failed during
            // the construction of the object if at the time COM was not initilizated (this
            // is the case of global variables). In this case try to create it now.
            CoCreateInstance(CLSID_StdGlobalInterfaceTable,
                             NULL,
                             CLSCTX_INPROC_SERVER,
                             IID_IGlobalInterfaceTable,
                             (void **)&m_spGIT);
        }
        if (m_spGIT && (NULL!=pInterface))
        {
            m_spGIT->RegisterInterfaceInGlobal(pInterface, __uuidof(T), &dwInterfaceCookie);
        }
        return *this;
    }

    void Clear()
    {
        ClearCookie();
        m_spGIT.Release();
    }

    operator bool()
    {
        return (0!=dwInterfaceCookie);
    }

    HRESULT GetInterface(T** ppInterface)
    {
        if (NULL == ppInterface)
            return E_POINTER;
        *ppInterface = NULL;

        if ( !m_spGIT || (0==dwInterfaceCookie) )
            return E_UNEXPECTED;

        return m_spGIT->GetInterfaceFromGlobal(dwInterfaceCookie, __uuidof(T), (void**)ppInterface);
    }
};

