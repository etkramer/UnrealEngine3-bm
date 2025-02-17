//////////////////////////////////////////////////////////////////////////////
// RegisterPack
//
// These structures and classes simulate two sets of registers for display
// in the Registers window.
//
// The primary class is CRegisterPack which can be instantiated any number
// of times but which draws on a globally allocated collection of registers.
// This means the registers can be accessed from anywhere in the program just
// by instantiating CRegisterPack.
//
// Warning! This set of classes and structures is not thread-safe.  However,
// these are normally used only in the context of a single thread.  Since
// these classes are only used to stand in for real system registers, making
// these classes thread-safe is left as an exercise for the reader.
//////////////////////////////////////////////////////////////////////////////

#pragma once
#include <vector>

//////////////////////////////////////////////////////////////////////////////
// This structure represents a simple register with a value.
struct Register {
	CComBSTR  name;
	ULONGLONG value;
	size_t    index;	// This is set when the register is added to a group.

	Register(BSTR pName = L"", size_t i = 0, ULONGLONG v = 0): name(pName), index(i), value(v) { }
	bool IsValid() { return(name != NULL && name != L""); }
};

typedef std::vector<Register> RegisterList;


//////////////////////////////////////////////////////////////////////////////
// This class represents a named group of registers.
// The group name is used in the Registers window context menu.
class RegisterGroup {
	private:
		CComBSTR name;
		RegisterList registerList;

	public:
		RegisterGroup(BSTR pName = L"");
		void      AddRegister(Register r);
		CComBSTR& Name();
		size_t    NumRegisters();
		Register  GetRegister(size_t index);
		void      SetRegisterValue(size_t index, ULONGLONG value);
};

typedef std::vector<RegisterGroup> RegisterGroupList;


//////////////////////////////////////////////////////////////////////////////
// This class represents a collection of register groups, where each group
// has a name and contains a collection of registers.  Each register has a
// name and a value.
class CRegisterPack {
	private:
		static RegisterGroupList m_registerGroupList;

	public:
		CRegisterPack();
		RegisterGroupList& GetGroupList();
		size_t   NumGroups();
		size_t   NumRegistersInGroup(size_t groupIndex);
		BSTR     GetRegisterGroupName(size_t groupIndex);
		size_t   GetRegisterGroupIndex(BSTR pName);
		Register GetRegister(size_t groupIndex,size_t registerIndex);
		void     SetRegisterValue(size_t groupIndex, size_t registerIndex, ULONGLONG value);
		BSTR     GetRegisterValueAsString(size_t groupIndex, size_t registerIndex);

		// Static methods
		static BSTR      ValueToString(ULONGLONG value);
		static ULONGLONG StringToValue(BSTR pValue);
};
