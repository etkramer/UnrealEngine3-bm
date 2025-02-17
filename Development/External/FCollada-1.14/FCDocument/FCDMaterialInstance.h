/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/
/*
	Based on the FS Import classes:
	Copyright (C) 2005-2006 Feeling Software Inc
	Copyright (C) 2005-2006 Autodesk Media Entertainment
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FCDMaterialInstance.h
	This file contains the FCDMaterialInstance and the FCDMaterialInstanceBind classes.
*/

#ifndef _FCD_MATERIAL_BIND_H_
#define	_FCD_MATERIAL_BIND_H_

#ifndef _FCD_ENTITY_INSTANCE_H_
#include "FCDocument/FCDEntityInstance.h"
#endif // _FCD_ENTITY_INSTANCE_H_
#ifndef _FU_DAE_ENUM_H_
#include "FUtils/FUDaeEnum.h"
#endif // _FU_DAE_ENUM_H_

class FCDocument;
class FCDGeometryInstance;
class FCDGeometryPolygons;

/**
	A ColladaFX per-instance binding.
*/
class FCOLLADA_EXPORT FCDMaterialInstanceBind
{
public:
	string semantic;
	string target;
};

/**
	A ColladaFX per-instance vertex input binding.
*/
class FCOLLADA_EXPORT FCDMaterialInstanceBindVertexInput
{
public:
	string semantic;
	FUDaeGeometryInput::Semantic inputSemantic;
	int32 inputSet;
};

/**
	A ColladaFX per-instance texture surface binding.
*/
class FCOLLADA_EXPORT FCDMaterialInstanceBindTextureSurface
{
public:
	string semantic;
	int32 surfaceIndex;
	int32 textureIndex;
};

typedef vector<FCDMaterialInstanceBind> FCDMaterialInstanceBindList; /**< A dynamically-sized array of per-instance binding. */
typedef vector<FCDMaterialInstanceBindVertexInput> FCDMaterialInstanceBindVertexInputList; /**< A dynamically-sized array of per-instance vertex input binding. */
typedef vector<FCDMaterialInstanceBindTextureSurface> FCDMaterialInstanceBindTextureSurfaceList; /**< A dynamically-sized array of per-instance texture surface binding. */

/**
	A COLLADA material instance.
	A material instance is used to given polygon sets with a COLLADA material entity.
	It is also used to bind data sources with the inputs of an effect.

	@ingroup FCDocument
*/
class FCOLLADA_EXPORT FCDMaterialInstance : public FCDEntityInstance
{
private:
	DeclareObjectType(FCDEntityInstance);
	FCDGeometryInstance* parent;
	fstring semantic;
	FCDMaterialInstanceBindList bindings;
	FCDMaterialInstanceBindVertexInputList vertexBindings;
	FCDMaterialInstanceBindTextureSurfaceList texSurfBindings;

public:
	FCDMaterialInstance(FCDGeometryInstance* parent);
	virtual ~FCDMaterialInstance();

	// Accessors
	virtual Type GetType() const { return MATERIAL; }

	inline const fstring& GetSemantic() const { return semantic; }
	inline void SetSemantic(const fchar* _semantic) { semantic = _semantic; SetDirtyFlag(); }
	inline void SetSemantic(const fstring& _semantic) { semantic = _semantic; SetDirtyFlag(); }

	inline FCDMaterial* GetMaterial() { return (FCDMaterial*) GetEntity(); }
	inline const FCDMaterial* GetMaterial() const { return (FCDMaterial*) GetEntity(); }
	inline void SetMaterial(FCDMaterial* _material) { SetEntity((FCDEntity*) _material); }

	inline FCDMaterialInstanceBindList& GetBindings() { return bindings; }
	inline const FCDMaterialInstanceBindList& GetBindings() const { return bindings; }
	inline size_t GetBindingCount() const { return bindings.size(); }
	inline FCDMaterialInstanceBind* GetBinding(size_t index) { FUAssert(index < bindings.size(), return NULL); return &bindings.at(index); }
	inline const FCDMaterialInstanceBind* GetBinding(size_t index) const { FUAssert(index < bindings.size(), return NULL); return &bindings.at(index); }

	// NOTE: this function uses the parent geometry instance and searches for the polygon set:
	// The application should therefore buffer the retrieved pointer.
	FCDGeometryPolygons* GetPolygons();

	FCDMaterialInstanceBind* AddBinding();
	FCDMaterialInstanceBind* AddBinding(const char* semantic, const char* target);
	inline FCDMaterialInstanceBind* AddBinding(const string& semantic, const char* target) { return AddBinding(semantic.c_str(), target); }
	inline FCDMaterialInstanceBind* AddBinding(const char* semantic, const string& target) { return AddBinding(semantic, target.c_str()); }
	inline FCDMaterialInstanceBind* AddBinding(const string& semantic, const string& target) { return AddBinding(semantic.c_str(), target.c_str()); }
	void ReleaseBinding(size_t index);

	/** Vertex input binding functions. */
	
	inline FCDMaterialInstanceBindVertexInputList& GetVertexInputBindings() { return vertexBindings; }
	inline const FCDMaterialInstanceBindVertexInputList& GetVertexInputBindings() const { return vertexBindings; }
	inline size_t GetVertexInputBindingCount() const { return vertexBindings.size(); }
	inline FCDMaterialInstanceBindVertexInput* GetVertexInputBinding(size_t index) { FUAssert(index < vertexBindings.size(), return NULL); return &vertexBindings.at(index); }
	inline const FCDMaterialInstanceBindVertexInput* GetVertexInputBinding(size_t index) const { FUAssert(index < vertexBindings.size(), return NULL); return &vertexBindings.at(index); }

	FCDMaterialInstanceBindVertexInput* AddVertexInputBinding();
	FCDMaterialInstanceBindVertexInput* AddVertexInputBinding(const char* semantic, FUDaeGeometryInput::Semantic inputSemantic, int32 inputSet);

	/** Texture surface binding functions. */

	inline FCDMaterialInstanceBindTextureSurfaceList& GetTextureSurfaceBindings() { return texSurfBindings; }
	inline const FCDMaterialInstanceBindTextureSurfaceList& GetTextureSurfaceBindings() const { return texSurfBindings; }
	inline size_t GetTextureSurfaceBindingCount() const { return texSurfBindings.size(); }
	inline FCDMaterialInstanceBindTextureSurface* GetTextureSurfaceBinding(size_t index) { FUAssert(index < texSurfBindings.size(), return NULL); return &texSurfBindings.at(index); }
	inline const FCDMaterialInstanceBindTextureSurface* GetTextureSurfaceBinding(size_t index) const { FUAssert(index < texSurfBindings.size(), return NULL); return &texSurfBindings.at(index); }

	FCDMaterialInstanceBindTextureSurface* AddTextureSurfaceBinding();
	FCDMaterialInstanceBindTextureSurface* AddTextureSurfaceBinding(const char* semantic, int32 surfIndex, int32 texSurfIndex );

	/** Creates a flattened version of the instantiated material. This is the
		preferred way to generate viewer materials from a COLLADA document.
		@return The flattened version of the instantiated material. You
			will need to delete this pointer manually. This pointer will
			be NULL when there is no material attached to this instance. */
	FCDMaterial* FlattenMaterial();

	/** Clones the material instance.
		@param clone The material instance to become the clone.
		@return The cloned material instance. */
	virtual FCDEntityInstance* Clone(FCDEntityInstance* clone = NULL) const;

	// Read in the material instantiation from the COLLADA document
	virtual FUStatus LoadFromXML(xmlNode* instanceNode);
	FUStatus LoadFromId(const string& materialId); // COLLADA 1.3 backward compatibility

	// Write out the instantiation information to the XML node tree
	xmlNode* WriteToXML(xmlNode* parentNode) const;
};

#endif // _FCD_MATERIAL_BIND_H_
