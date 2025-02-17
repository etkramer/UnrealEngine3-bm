/*
	Copyright (C) 2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#include "StdAfx.h"
#include "FCDocument/FCDocument.h"
#include "FCDocument/FCDAnimated.h"
#include "FCDocument/FCDEffectPass.h"
#include "FCDocument/FCDEffectProfile.h"
#include "FCDocument/FCDEffectTechnique.h"
#include "FCDocument/FCDEffectParameter.h"
#include "FCDocument/FCDEffectParameterSurface.h"
#include "FCDocument/FCDImage.h"
#include "FUtils/FUDaeParser.h"
#include "FUtils/FUDaeWriter.h"
using namespace FUDaeParser;
using namespace FUDaeWriter;

ImplementObjectType(FCDEffectParameterSurface);

// surface type parameter
FCDEffectParameterSurface::FCDEffectParameterSurface(FCDocument* document) : FCDEffectParameter(document)
{
	initMethod = NULL;
	size.x = size.y = size.z = 0.f;
	formatHint = NULL;
	viewportRatio = 1.f;
	mipLevelCount = 0;
	generateMipmaps = false;
	type = "2D";
	format = "A8R8G8B8";
}

FCDEffectParameterSurface::~FCDEffectParameterSurface()
{
	SAFE_DELETE(initMethod);
	SAFE_DELETE(formatHint);
	names.clear();
}

void FCDEffectParameterSurface::SetInitMethod(FCDEffectParameterSurfaceInit* method)
{
	SAFE_DELETE(initMethod);
	initMethod = method;
	SetDirtyFlag();
}

// Retrieves the index that matches the given image.
size_t FCDEffectParameterSurface::FindImage(const FCDImage* image) const
{
	FCDImageTrackList::const_iterator it = images.find(image);
	if (it != images.end())
	{
		return it - images.begin();
	}
	else return (size_t) -1;
}

// Adds an image to the list.
size_t FCDEffectParameterSurface::AddImage(FCDImage* image)
{
	size_t index = FindImage(image);
	if (index == (size_t) -1)
	{
		index = images.size();
		images.push_back(image);
	}
	SetDirtyFlag();
	return index;
}

// Removes an image from the list.
void FCDEffectParameterSurface::RemoveImage(FCDImage* image)
{
	size_t index = FindImage(image);
	if (index != (size_t) -1)
	{
		images.erase(index);

		if (initMethod != NULL && initMethod->GetInitType() == FCDEffectParameterSurfaceInitFactory::CUBE)
		{
			// Shift down all the indexes found within the cube map initialization.
			FCDEffectParameterSurfaceInitCube* cube = (FCDEffectParameterSurfaceInitCube*) initMethod;
			for (UInt16List::iterator itI = cube->order.begin(); itI != cube->order.end(); ++itI)
			{
				if ((*itI) == index) (*itI) = 0;
				else if ((*itI) > index) --(*itI);
			}
		}

		SetDirtyFlag();
	}
}

// compare value
bool FCDEffectParameterSurface::IsValueEqual( FCDEffectParameter *parameter )
{
	if( !FCDEffectParameter::IsValueEqual( parameter ) ) return false;
	FCDEffectParameterSurface *param = (FCDEffectParameterSurface*)parameter;
	
	// compage images
	size_t imgCount = this->GetImageCount();
	if( imgCount != param->GetImageCount() ) return false;
	
	for( size_t i = 0; i < imgCount; i++ )
	{
		if( this->GetImage( i ) != param->GetImage( i ) ) return false;
	}

	// compare initialisation methods
	if( initMethod != NULL && param->GetInitMethod() != NULL )
	{
		if( initMethod->GetInitType() != param->GetInitMethod()->GetInitType() ) return false;
	}
	else
	{
		if( initMethod != param->GetInitMethod() ) return false;
	}

	if( size != param->GetSize() ) return false;
	if( mipLevelCount != param->GetMipLevelCount() ) return false;
	if( generateMipmaps != param->IsGenerateMipMaps() ) return false;
	if( viewportRatio != param->GetViewportRatio() ) return false;

	// if you want to add more tests, feel free!

	return true;
}

// Clone
FCDEffectParameter* FCDEffectParameterSurface::Clone(FCDEffectParameter* _clone) const
{
	FCDEffectParameterSurface* clone = NULL;
	if (_clone == NULL) _clone = clone = new FCDEffectParameterSurface(const_cast<FCDocument*>(GetDocument()));
	else if (_clone->GetObjectType() == FCDEffectParameterSurface::GetClassType()) clone = (FCDEffectParameterSurface*) _clone;

	if (_clone != NULL) FCDEffectParameter::Clone(_clone);
	if (clone != NULL)
	{
		clone->images.reserve(images.size());
		for(FCDImageTrackList::const_iterator itI = images.begin(); itI != images.end(); ++itI)
		{
			clone->AddImage(*itI);
		}

		if (initMethod != NULL)
		{
			clone->initMethod = FCDEffectParameterSurfaceInitFactory::Create(initMethod->GetInitType());
			initMethod->Clone(clone->initMethod);
		}

		clone->size = size;
		clone->viewportRatio = viewportRatio;
		clone->mipLevelCount = mipLevelCount;
		clone->generateMipmaps = generateMipmaps;

		clone->format = format;
		if (formatHint != NULL)
		{
			clone->formatHint = new FCDFormatHint();
			clone->formatHint->channels = formatHint->channels;
			clone->formatHint->range = formatHint->range;
			clone->formatHint->precision = formatHint->precision;
			clone->formatHint->options = formatHint->options;
		}
	}
	return _clone;
}

// Flattening: overwrite the target parameter with this parameter
void FCDEffectParameterSurface::Overwrite(FCDEffectParameter* target)
{
	if (target->GetType() == SURFACE)
	{
		FCDEffectParameterSurface* s = (FCDEffectParameterSurface*) target;
		s->images.clear();
		for(uint32 i=0; i<images.size(); i++)
			s->images.push_back(images[i]);

//		s->initMethod->initType = initMethod->GetInitType();
		s->size = size;
		s->viewportRatio = viewportRatio;
		s->mipLevelCount = mipLevelCount;
		s->generateMipmaps = generateMipmaps;
		SetDirtyFlag();
	}
}

FUStatus FCDEffectParameterSurface::LoadFromXML(xmlNode* parameterNode)
{
	FUStatus status = FCDEffectParameter::LoadFromXML(parameterNode);
	xmlNode* surfaceNode = FindChildByType(parameterNode, DAE_FXCMN_SURFACE_ELEMENT);

	bool initialized = false;
	xmlNode* valueNode = NULL;
	//The surface can now contain many init_from elements (1.4.1)
	xmlNodeList valueNodes;
	FindChildrenByType(surfaceNode, DAE_INITFROM_ELEMENT, valueNodes);
	for (xmlNodeList::iterator it = valueNodes.begin(); it != valueNodes.end(); ++it)
	{
		initialized = true;
		if(!initMethod)
			initMethod = new FCDEffectParameterSurfaceInitFrom();

		FCDEffectParameterSurfaceInitFrom* ptrInit = (FCDEffectParameterSurfaceInitFrom*)initMethod;
		//StringList names;
		FUStringConversion::ToStringList(ReadNodeContentDirect(*it), names);
		
		if (names.size() == 0 || names[0].empty())
		{
			names.clear();
			status.Warning(FS("<init_from> element is empty in surface parameter: ") + TO_FSTRING(GetReference()), parameterNode->line);
		}

		if(names.size() == 1) //might be 1.4.1, so allow the attributes mip, slice, face
		{
			if(HasNodeProperty(*it, DAE_MIP_ATTRIBUTE))
			{
				string mip = ReadNodeProperty(*it, DAE_MIP_ATTRIBUTE);
				ptrInit->mip.push_back(mip);
			}
			if(HasNodeProperty(*it, DAE_SLICE_ATTRIBUTE))
			{
				string slice = ReadNodeProperty(*it, DAE_SLICE_ATTRIBUTE);
				ptrInit->slice.push_back(slice);
			}
			if(HasNodeProperty(*it, DAE_FACE_ATTRIBUTE))
			{
				string face = ReadNodeProperty(*it, DAE_FACE_ATTRIBUTE);
				ptrInit->face.push_back(face);
			}
		}
	}

	//Check if it's initialized AS NULL
	if(!initialized)
	{
		valueNode = FindChildByType(surfaceNode, DAE_INITASNULL_ELEMENT);
		if(valueNode)
		{
			initialized = true;
			initMethod = FCDEffectParameterSurfaceInitFactory::Create(FCDEffectParameterSurfaceInitFactory::AS_NULL);
		}
	}
	//Check if it's initialized AS TARGET
	if(!initialized)
	{
		valueNode = FindChildByType(surfaceNode, DAE_INITASTARGET_ELEMENT);
		if(valueNode)
		{
			initialized = true;
			initMethod = FCDEffectParameterSurfaceInitFactory::Create(FCDEffectParameterSurfaceInitFactory::AS_TARGET);
		}
	}
	//Check if it's initialized AS CUBE
	if(!initialized)
	{
		valueNode = FindChildByType(surfaceNode, DAE_INITCUBE_ELEMENT);
		if(valueNode)
		{
			initialized = true;
			initMethod = FCDEffectParameterSurfaceInitFactory::Create(FCDEffectParameterSurfaceInitFactory::CUBE);
			FCDEffectParameterSurfaceInitCube* ptrInit = (FCDEffectParameterSurfaceInitCube*) initMethod;

			//Check if it's an ALL reference
			xmlNode* refNode = FindChildByType(valueNode, DAE_ALL_ELEMENT);
			if(refNode)
			{
				ptrInit->cubeType = FCDEffectParameterSurfaceInitCube::ALL;
				string name = ReadNodeProperty(refNode, DAE_REF_ATTRIBUTE);
				if (name.empty())
				{
					status.Warning(FS("Empty image name for surface parameter: ") + TO_FSTRING(GetReference()), surfaceNode->line);
				}
				else names.push_back(name);
			}

			//Check if it's a PRIMARY reference
			if(!refNode)
			{
				refNode = FindChildByType(valueNode, DAE_PRIMARY_ELEMENT);
				if(refNode)
				{
					ptrInit->cubeType = FCDEffectParameterSurfaceInitCube::PRIMARY;
					string name = ReadNodeProperty(refNode, DAE_REF_ATTRIBUTE);
					if (name.empty())
					{
						status.Warning(FS("Empty image name for surface parameter: ") + TO_FSTRING(GetReference()), surfaceNode->line);
					}
					else names.push_back(name);
	
					xmlNode* orderNode = FindChildByType(refNode, DAE_ORDER_ELEMENT);
					if(orderNode)
					{
						//FIXME: complete when the spec has more info
					}
				}
			}

			//Check if it's a FACE reference
			if(!refNode)
			{
				xmlNodeList faceNodes;
				FindChildrenByType(valueNode, DAE_FACE_ELEMENT, faceNodes);
				if(faceNodes.size()==6)
				{
					ptrInit->cubeType = FCDEffectParameterSurfaceInitCube::FACE;
					for(uint8 ii=0; ii<faceNodes.size(); ii++)
					{
						string name = ReadNodeProperty(faceNodes[ii], DAE_REF_ATTRIBUTE);
						if (name.empty())
						{
							status.Warning(FS("Empty image name for surface parameter: ") + TO_FSTRING(GetReference()), surfaceNode->line);
						}
						else names.push_back(name);
					}
				}
			}
		}
	}

	//Check if it's initialized AS VOLUME
	if(!initialized)
	{
		valueNode = FindChildByType(surfaceNode, DAE_INITVOLUME_ELEMENT);
		if(valueNode)
		{
			initialized = true;
			initMethod = FCDEffectParameterSurfaceInitFactory::Create(FCDEffectParameterSurfaceInitFactory::VOLUME);
			FCDEffectParameterSurfaceInitVolume* ptrInit = (FCDEffectParameterSurfaceInitVolume*) initMethod;

			//Check if it's an ALL reference
			xmlNode* refNode = FindChildByType(valueNode, DAE_ALL_ELEMENT);
			if(refNode)
			{
				ptrInit->volumeType = FCDEffectParameterSurfaceInitVolume::ALL;
				string name = ReadNodeProperty(refNode, DAE_REF_ATTRIBUTE);
				if (name.empty())
				{
					status.Warning(FS("Empty image name for surface parameter: ") + TO_FSTRING(GetReference()), surfaceNode->line);
				}
				else names.push_back(name);
			}

			//Check if it's a PRIMARY reference
			if(!refNode)
			{
				refNode = FindChildByType(valueNode, DAE_PRIMARY_ELEMENT);
				if(refNode)
				{
					ptrInit->volumeType = FCDEffectParameterSurfaceInitVolume::PRIMARY;
					string name = ReadNodeProperty(refNode, DAE_REF_ATTRIBUTE);
					if (name.empty())
					{
						status.Warning(FS("Empty image name for surface parameter: ") + TO_FSTRING(GetReference()), surfaceNode->line);
					}
					else names.push_back(name);
				}
			}
		}
	}

	//Check if it's initialized as PLANAR
	if(!initialized)
	{
		valueNode = FindChildByType(surfaceNode, DAE_INITPLANAR_ELEMENT);
		if(valueNode)
		{
			initialized = true;
			initMethod = FCDEffectParameterSurfaceInitFactory::Create(FCDEffectParameterSurfaceInitFactory::PLANAR);

			//Check if it's an ALL reference
			xmlNode* refNode = FindChildByType(valueNode, DAE_ALL_ELEMENT);
			if(refNode)
			{
				string name = ReadNodeProperty(refNode, DAE_REF_ATTRIBUTE);
				if (name.empty())
				{
					status.Warning(FS("Empty image name for surface parameter: ") + TO_FSTRING(GetReference()), surfaceNode->line);
				}
				else names.push_back(name);
			}
		}
	}
	
	// It is acceptable for a surface not to have an initialization option
	//but we should flag a warning
	if(!initialized)
	{
		WARNING_OUT1("Warning: surface %s not initialized", GetReference().c_str());
	}
	
	xmlNode* sizeNode = FindChildByType(parameterNode, DAE_SIZE_ELEMENT);
	size = FUStringConversion::ToPoint(ReadNodeContentDirect(sizeNode));
	xmlNode* viewportRatioNode = FindChildByType(parameterNode, DAE_VIEWPORT_RATIO);
	viewportRatio = FUStringConversion::ToFloat(ReadNodeContentDirect(viewportRatioNode));
	xmlNode* mipLevelsNode = FindChildByType(parameterNode, DAE_MIP_LEVELS);
	mipLevelCount = (uint16) FUStringConversion::ToInt32(ReadNodeContentDirect(mipLevelsNode));
	xmlNode* mipmapGenerateNode = FindChildByType(parameterNode, DAE_MIPMAP_GENERATE);
	generateMipmaps = FUStringConversion::ToBoolean(ReadNodeContentDirect(mipmapGenerateNode));

	xmlNode* formatNode = FindChildByType(parameterNode, DAE_FORMAT_ELEMENT);
	if(formatNode)
		format = FUStringConversion::ToString(ReadNodeContentDirect(formatNode));
	
	xmlNode* formatHintNode = FindChildByType(parameterNode, DAE_FORMAT_HINT_ELEMENT);
	if(formatHintNode)
	{
		formatHint = new FCDFormatHint();
		
		xmlNode* hintChild = FindChildByType(formatHintNode, DAE_CHANNEL_ELEMENT);
		if(!hintChild)
		{
			WARNING_OUT1("Warning: surface %s misses channel information in its format hint.", GetReference().c_str());
			formatHint->channels = FCDFormatHint::CHANNEL_UNKNOWN;
		}
		else
		{
			string contents = FUStringConversion::ToString(ReadNodeContentDirect(hintChild));
			if (IsEquivalent(contents, DAE_FORMAT_HINT_RGB_VALUE)) formatHint->channels = FCDFormatHint::CHANNEL_RGB;
			else if (IsEquivalent(contents, DAE_FORMAT_HINT_RGBA_VALUE)) formatHint->channels = FCDFormatHint::CHANNEL_RGBA;
			else if (IsEquivalent(contents, DAE_FORMAT_HINT_L_VALUE)) formatHint->channels = FCDFormatHint::CHANNEL_L;
			else if (IsEquivalent(contents, DAE_FORMAT_HINT_LA_VALUE)) formatHint->channels = FCDFormatHint::CHANNEL_LA;
			else if (IsEquivalent(contents, DAE_FORMAT_HINT_D_VALUE)) formatHint->channels = FCDFormatHint::CHANNEL_D;
			else if (IsEquivalent(contents, DAE_FORMAT_HINT_XYZ_VALUE)) formatHint->channels = FCDFormatHint::CHANNEL_XYZ;
			else if (IsEquivalent(contents, DAE_FORMAT_HINT_XYZW_VALUE)) formatHint->channels = FCDFormatHint::CHANNEL_XYZW;
			else
			{
				WARNING_OUT2("Warning: surface %s contains an invalid channel description %s in its format hint.", GetReference().c_str(), contents.c_str());
				formatHint->channels = FCDFormatHint::CHANNEL_UNKNOWN;
			}
		}
		
		hintChild = FindChildByType(formatHintNode, DAE_RANGE_ELEMENT);
		if(!hintChild)
		{
			WARNING_OUT1("Warning: surface %s misses range information in its format hint.", GetReference().c_str());
			formatHint->range = FCDFormatHint::RANGE_UNKNOWN;
		}
		else
		{
			string contents = FUStringConversion::ToString(ReadNodeContentDirect(hintChild));
			if (IsEquivalent(contents, DAE_FORMAT_HINT_SNORM_VALUE)) formatHint->range = FCDFormatHint::RANGE_SNORM;
			else if (IsEquivalent(contents, DAE_FORMAT_HINT_UNORM_VALUE)) formatHint->range = FCDFormatHint::RANGE_UNORM;
			else if (IsEquivalent(contents, DAE_FORMAT_HINT_SINT_VALUE)) formatHint->range = FCDFormatHint::RANGE_SINT;
			else if (IsEquivalent(contents, DAE_FORMAT_HINT_UINT_VALUE)) formatHint->range = FCDFormatHint::RANGE_UINT;
			else if (IsEquivalent(contents, DAE_FORMAT_HINT_FLOAT_VALUE)) formatHint->range = FCDFormatHint::RANGE_FLOAT;
			else if (IsEquivalent(contents, DAE_FORMAT_HINT_LOW_VALUE)) formatHint->range = FCDFormatHint::RANGE_LOW;
			else
			{
				WARNING_OUT2("Warning: surface %s contains an invalid range description %s in its format hint.", GetReference().c_str(), contents.c_str());
				formatHint->range= FCDFormatHint::RANGE_UNKNOWN;
			}
		}

		hintChild = FindChildByType(formatHintNode, DAE_PRECISION_ELEMENT);
		if(hintChild)
		{
			string contents = FUStringConversion::ToString(ReadNodeContentDirect(hintChild));
			if (IsEquivalent(contents, DAE_FORMAT_HINT_LOW_VALUE)) formatHint->precision = FCDFormatHint::PRECISION_LOW;
			else if (IsEquivalent(contents, DAE_FORMAT_HINT_MID_VALUE)) formatHint->precision = FCDFormatHint::PRECISION_MID;
			else if (IsEquivalent(contents, DAE_FORMAT_HINT_HIGH_VALUE)) formatHint->precision = FCDFormatHint::PRECISION_HIGH;
			else
			{
				WARNING_OUT2("Warning: surface %s contains an invalid precision description %s in its format hint.", GetReference().c_str(), contents.c_str());
				formatHint->precision = FCDFormatHint::PRECISION_UNKNOWN;
			}
		}

		xmlNodeList optionNodes;
		FindChildrenByType(formatHintNode, DAE_OPTION_ELEMENT, optionNodes);
		for (xmlNodeList::iterator it = optionNodes.begin(); it != optionNodes.end(); ++it)
		{
			string contents = FUStringConversion::ToString(ReadNodeContentDirect(*it));
			if (IsEquivalent(contents, DAE_FORMAT_HINT_SRGB_GAMMA_VALUE)) formatHint->options.push_back(FCDFormatHint::OPT_SRGB_GAMMA);
			else if (IsEquivalent(contents, DAE_FORMAT_HINT_NORMALIZED3_VALUE)) formatHint->options.push_back(FCDFormatHint::OPT_NORMALIZED3);
			else if (IsEquivalent(contents, DAE_FORMAT_HINT_NORMALIZED4_VALUE)) formatHint->options.push_back(FCDFormatHint::OPT_NORMALIZED4);
			else if (IsEquivalent(contents, DAE_FORMAT_HINT_COMPRESSABLE_VALUE)) formatHint->options.push_back(FCDFormatHint::OPT_COMPRESSABLE);
			else 
			{
				WARNING_OUT2("Warning: surface %s contains an invalid option description %s in its format hint.", GetReference().c_str(), contents.c_str());
			}
		}
	}

	SetDirtyFlag();
	return status;
}

// Links the parameter
void FCDEffectParameterSurface::Link(FCDEffectParameterList& UNUSED(parameters))
{
	for (StringList::iterator itN = names.begin(); itN != names.end(); ++itN)
	{
		FCDImage* image = GetDocument()->FindImage(*itN);
		if (image != NULL)
		{
			AddImage(image);
		}
	}
}

xmlNode* FCDEffectParameterSurface::WriteToXML(xmlNode* parentNode) const
{
	xmlNode* parameterNode = FCDEffectParameter::WriteToXML(parentNode);
	xmlNode* surfaceNode = AddChild(parameterNode, DAE_FXCMN_SURFACE_ELEMENT);
	AddAttribute(surfaceNode, DAE_TYPE_ATTRIBUTE, type);
	
	if (initMethod != NULL)
	{
		switch (initMethod->GetInitType())
		{
			case FCDEffectParameterSurfaceInitFactory::FROM:
			{
				//Since 1.4.1, there are two possibilities here.
				//Possibility 1
				//<init_from> image1 image2...imageN </init_from>

				//Possibility 2
				//<init_from mip=... face=... slice=...> image1 </init_from>
				//<init_from mip=... face=... slice=...> image2 </init_from>

				FCDEffectParameterSurfaceInitFrom* in = (FCDEffectParameterSurfaceInitFrom*)initMethod;
				size_t size = images.size();
				size_t sizeMips = in->mip.size();
				size_t sizeSlices = in->slice.size();
				size_t sizeFaces = in->face.size();

				if( size == in->face.size() || size == in->mip.size() || size == in->slice.size())
				{
					//This is possibility 2
					for(uint32 i=0; i<size; i++)
					{
						xmlNode* childNode = AddChild(surfaceNode, DAE_INITFROM_ELEMENT);
						if(i<sizeMips) AddAttribute(childNode, DAE_MIP_ATTRIBUTE, in->mip[i]);
						if(i<sizeSlices) AddAttribute(childNode, DAE_SLICE_ATTRIBUTE, in->slice[i]);
						if(i<sizeFaces) AddAttribute(childNode, DAE_FACE_ATTRIBUTE, in->face[i]);
						AddContent(childNode, images[i]->GetDaeId());
					}
				}
				else
				{
					//This is possibility 1
					globalSBuilder.clear();
					for (FCDImageTrackList::const_iterator itV = images.begin(); itV != images.end(); ++itV) 
					{ 
						globalSBuilder.append((*itV)->GetDaeId());
						globalSBuilder.append(' ');
					}
					globalSBuilder.pop_back();
					xmlNode* childNode = AddChild(surfaceNode, DAE_INITFROM_ELEMENT);
					AddContent(childNode, globalSBuilder.ToCharPtr());
				}
				break;
			}
			case FCDEffectParameterSurfaceInitFactory::AS_NULL:
			{
				AddChild(surfaceNode, DAE_INITASNULL_ELEMENT);
				break;
			}
			case FCDEffectParameterSurfaceInitFactory::AS_TARGET:
			{
				AddChild(surfaceNode, DAE_INITASTARGET_ELEMENT);
				break;
			}
			case FCDEffectParameterSurfaceInitFactory::VOLUME:
			{
				FCDEffectParameterSurfaceInitVolume* in = (FCDEffectParameterSurfaceInitVolume*)initMethod;
				xmlNode* childNode = AddChild(surfaceNode, DAE_INITVOLUME_ELEMENT);
				if(in->volumeType == FCDEffectParameterSurfaceInitVolume::ALL)
				{
					xmlNode* typeNode = AddChild(childNode, DAE_ALL_ELEMENT);
					AddAttribute(typeNode, DAE_REF_ATTRIBUTE, images[0]->GetDaeId());
				}
				else if(in->volumeType == FCDEffectParameterSurfaceInitVolume::PRIMARY)
				{
					xmlNode* typeNode = AddChild(childNode, DAE_PRIMARY_ELEMENT);
					AddAttribute(typeNode, DAE_REF_ATTRIBUTE, images[0]->GetDaeId());
				}
				break;
			}
			case FCDEffectParameterSurfaceInitFactory::CUBE:
			{
				FCDEffectParameterSurfaceInitCube* in = (FCDEffectParameterSurfaceInitCube*)initMethod;
				xmlNode* childNode = AddChild(surfaceNode, DAE_INITCUBE_ELEMENT);
				if(in->cubeType == FCDEffectParameterSurfaceInitCube::ALL)
				{
					xmlNode* typeNode = AddChild(childNode, DAE_ALL_ELEMENT);
					AddAttribute(typeNode, DAE_REF_ATTRIBUTE, images[0]->GetDaeId());
				}
				else if(in->cubeType == FCDEffectParameterSurfaceInitCube::PRIMARY)
				{
					xmlNode* typeNode = AddChild(childNode, DAE_PRIMARY_ELEMENT);
					AddChild(typeNode, DAE_ORDER_ELEMENT); //FIXME: complete when the spec gets more info.
					AddAttribute(typeNode, DAE_REF_ATTRIBUTE, images[0]->GetDaeId());
				}
				if(in->cubeType == FCDEffectParameterSurfaceInitCube::FACE)
				{
					xmlNode* childNode = AddChild(surfaceNode, DAE_FACE_ELEMENT);
					AddAttribute(childNode, DAE_REF_ATTRIBUTE, images[0]->GetDaeId());
				}
				break;
			}
			case FCDEffectParameterSurfaceInitFactory::PLANAR:
			{
				xmlNode* childNode = AddChild(surfaceNode, DAE_INITPLANAR_ELEMENT);
				xmlNode* typeNode = AddChild(childNode, DAE_ALL_ELEMENT);
				AddAttribute(typeNode, DAE_REF_ATTRIBUTE, images[0]->GetDaeId());
				break;
			}
			default:
				break;
		}

	}
	if(!format.empty())
	{
		xmlNode* childNode = AddChild(surfaceNode, DAE_FORMAT_ELEMENT);
		AddContent(childNode, format.c_str());
	}
	if(formatHint)
	{
		xmlNode* formatHintNode = AddChild(surfaceNode, DAE_FORMAT_HINT_ELEMENT);
		
		xmlNode* channelsNode = AddChild(formatHintNode, DAE_CHANNELS_ELEMENT);
		if(formatHint->channels == FCDFormatHint::CHANNEL_RGB) AddContent(channelsNode, DAE_FORMAT_HINT_RGB_VALUE);
		else if(formatHint->channels == FCDFormatHint::CHANNEL_RGBA) AddContent(channelsNode, DAE_FORMAT_HINT_RGBA_VALUE);
		else if(formatHint->channels == FCDFormatHint::CHANNEL_L) AddContent(channelsNode, DAE_FORMAT_HINT_L_VALUE);
		else if(formatHint->channels == FCDFormatHint::CHANNEL_LA) AddContent(channelsNode, DAE_FORMAT_HINT_LA_VALUE);
		else if(formatHint->channels == FCDFormatHint::CHANNEL_D) AddContent(channelsNode, DAE_FORMAT_HINT_D_VALUE);
		else if(formatHint->channels == FCDFormatHint::CHANNEL_XYZ) AddContent(channelsNode, DAE_FORMAT_HINT_XYZ_VALUE);
		else if(formatHint->channels == FCDFormatHint::CHANNEL_XYZW) AddContent(channelsNode, DAE_FORMAT_HINT_XYZW_VALUE);
		else AddContent(channelsNode, DAEERR_UNKNOWN_ELEMENT);

		xmlNode* rangeNode = AddChild(formatHintNode, DAE_RANGE_ELEMENT);
		if(formatHint->range == FCDFormatHint::RANGE_SNORM) AddContent(rangeNode, DAE_FORMAT_HINT_SNORM_VALUE);
		else if(formatHint->range == FCDFormatHint::RANGE_UNORM) AddContent(rangeNode, DAE_FORMAT_HINT_UNORM_VALUE);
		else if(formatHint->range == FCDFormatHint::RANGE_SINT) AddContent(rangeNode, DAE_FORMAT_HINT_SINT_VALUE);
		else if(formatHint->range == FCDFormatHint::RANGE_UINT) AddContent(rangeNode, DAE_FORMAT_HINT_UINT_VALUE);
		else if(formatHint->range == FCDFormatHint::RANGE_FLOAT) AddContent(rangeNode, DAE_FORMAT_HINT_FLOAT_VALUE);
		else if(formatHint->range == FCDFormatHint::RANGE_LOW) AddContent(rangeNode, DAE_FORMAT_HINT_LOW_VALUE);
		else AddContent(rangeNode, DAEERR_UNKNOWN_ELEMENT);

		xmlNode* precisionNode = AddChild(formatHintNode, DAE_PRECISION_ELEMENT);
		if(formatHint->precision == FCDFormatHint::PRECISION_LOW) AddContent(precisionNode, DAE_FORMAT_HINT_LOW_VALUE);
		else if(formatHint->precision == FCDFormatHint::PRECISION_MID) AddContent(precisionNode, DAE_FORMAT_HINT_MID_VALUE);
		else if(formatHint->precision == FCDFormatHint::PRECISION_HIGH) AddContent(precisionNode, DAE_FORMAT_HINT_HIGH_VALUE);
		else AddContent(precisionNode, DAEERR_UNKNOWN_ELEMENT);

		for(vector<FCDFormatHint::optionValue>::iterator it = formatHint->options.begin(); it != formatHint->options.end(); ++it)
		{
			xmlNode* optionNode = AddChild(formatHintNode, DAE_OPTION_ELEMENT);
			if(*it == FCDFormatHint::OPT_SRGB_GAMMA) AddContent(optionNode, DAE_FORMAT_HINT_SRGB_GAMMA_VALUE);
			else if(*it == FCDFormatHint::OPT_NORMALIZED3) AddContent(optionNode, DAE_FORMAT_HINT_NORMALIZED3_VALUE);
			else if(*it == FCDFormatHint::OPT_NORMALIZED4) AddContent(optionNode, DAE_FORMAT_HINT_NORMALIZED4_VALUE);
			else if(*it == FCDFormatHint::OPT_COMPRESSABLE) AddContent(optionNode, DAE_FORMAT_HINT_COMPRESSABLE_VALUE);
		}
	}
	if(size.x != 0.f || size.y != 0.f || size.z != 0.f)
	{
		xmlNode* sizeNode = AddChild(surfaceNode, DAE_SIZE_ELEMENT);
		AddContent(sizeNode, FUStringConversion::ToString(size));
	}
	else if(viewportRatio != 1.f) //viewport_ratio can only be there if size is not specified
	{
		xmlNode* viewportRatioNode = AddChild(surfaceNode, DAE_VIEWPORT_RATIO);
		AddContent(viewportRatioNode, FUStringConversion::ToString(viewportRatio));
	}
	
	if(mipLevelCount != 0)
	{
		xmlNode* mipmapLevelsNode = AddChild(surfaceNode, DAE_MIP_LEVELS);
		AddContent(mipmapLevelsNode, FUStringConversion::ToString(mipLevelCount));
	}
	
	return parameterNode;
}


FCDEffectParameterSurfaceInit* FCDEffectParameterSurfaceInit::Clone(FCDEffectParameterSurfaceInit* clone) const
{
	if (clone == NULL)
	{
		clone = FCDEffectParameterSurfaceInitFactory::Create(GetInitType());
		return clone != NULL ? Clone(clone) : NULL;
	}
	else
	{
		//no member variables to copy in this class, but leave this for future use.
		return clone;
	}
}

//
// FCDEffectParameterSurfaceInitCube
//

FCDEffectParameterSurfaceInitCube::FCDEffectParameterSurfaceInitCube()
{
	cubeType = ALL;
}

FCDEffectParameterSurfaceInit* FCDEffectParameterSurfaceInitCube::Clone(FCDEffectParameterSurfaceInit* _clone) const
{
	FCDEffectParameterSurfaceInitCube* clone = NULL;
	if (_clone == NULL) clone = new FCDEffectParameterSurfaceInitCube();
	else if (_clone->GetInitType() == GetInitType()) clone = (FCDEffectParameterSurfaceInitCube*) _clone;

	if (_clone != NULL) FCDEffectParameterSurfaceInit::Clone(_clone);
	if (clone != NULL)
	{
		clone->cubeType = cubeType;
	}
	return clone;
}

//
// FCDEffectParameterSurfaceInitFrom
// 

FCDEffectParameterSurfaceInit* FCDEffectParameterSurfaceInitFrom::Clone(FCDEffectParameterSurfaceInit* _clone) const
{
	FCDEffectParameterSurfaceInitFrom* clone = NULL;
	if (_clone == NULL) clone = new FCDEffectParameterSurfaceInitFrom();
	else if (_clone->GetInitType() == GetInitType()) clone = (FCDEffectParameterSurfaceInitFrom*) _clone;

	if (_clone != NULL) FCDEffectParameterSurfaceInit::Clone(_clone);
	if (clone != NULL)
	{
		clone->face = face;
		clone->mip = mip;
		clone->slice = slice;
	}
	return clone;
}

//
// FCDEffectParameterSurfaceInitVolume
//

FCDEffectParameterSurfaceInitVolume::FCDEffectParameterSurfaceInitVolume()
{
	volumeType = ALL;
}

FCDEffectParameterSurfaceInit* FCDEffectParameterSurfaceInitVolume::Clone(FCDEffectParameterSurfaceInit* _clone) const
{
	FCDEffectParameterSurfaceInitVolume* clone = NULL;
	if (_clone == NULL) clone = new FCDEffectParameterSurfaceInitVolume();
	else if (_clone->GetInitType() == GetInitType()) clone = (FCDEffectParameterSurfaceInitVolume*) _clone;

	if (_clone != NULL) FCDEffectParameterSurfaceInit::Clone(_clone);
	if (clone != NULL)
	{
		clone->volumeType = volumeType;
	}
	return clone;
}

//
// FCDEffectParameterSurfaceInitAsNull
//

FCDEffectParameterSurfaceInit* FCDEffectParameterSurfaceInitAsNull::Clone(FCDEffectParameterSurfaceInit* _clone) const
{
	FCDEffectParameterSurfaceInitAsNull* clone = NULL;
	if (_clone == NULL) clone = new FCDEffectParameterSurfaceInitAsNull();
	else if (_clone->GetInitType() == GetInitType()) clone = (FCDEffectParameterSurfaceInitAsNull*) _clone;

	if (_clone != NULL) FCDEffectParameterSurfaceInit::Clone(_clone);
	if (clone != NULL)
	{
		// Nothing to clone.
	}
	return clone;
}

//
// FCDEffectParameterSurfaceInitAsTarget
//

FCDEffectParameterSurfaceInit* FCDEffectParameterSurfaceInitAsTarget::Clone(FCDEffectParameterSurfaceInit* _clone) const
{
	FCDEffectParameterSurfaceInitAsTarget* clone = NULL;
	if (_clone == NULL) clone = new FCDEffectParameterSurfaceInitAsTarget();
	else if (_clone->GetInitType() == GetInitType()) clone = (FCDEffectParameterSurfaceInitAsTarget*) _clone;

	if (_clone != NULL) FCDEffectParameterSurfaceInit::Clone(_clone);
	if (clone != NULL)
	{
		// Nothing to clone.
	}
	return clone;
}

//
// FCDEffectParameterSurfaceInitPlanar
//

FCDEffectParameterSurfaceInit* FCDEffectParameterSurfaceInitPlanar::Clone(FCDEffectParameterSurfaceInit* _clone) const
{
	FCDEffectParameterSurfaceInitPlanar* clone = NULL;
	if (_clone == NULL) clone = new FCDEffectParameterSurfaceInitPlanar();
	else if (_clone->GetInitType() == GetInitType()) clone = (FCDEffectParameterSurfaceInitPlanar*) _clone;

	if (_clone != NULL) FCDEffectParameterSurfaceInit::Clone(_clone);
	if (clone != NULL)
	{
		// Nothing to clone.
	}
	return clone;
}

//
// FCDEffectParameterSurfaceInitFactory
//

FCDEffectParameterSurfaceInit* FCDEffectParameterSurfaceInitFactory::Create(InitType type)
{
	FCDEffectParameterSurfaceInit* parameter = NULL;

	switch (type)
	{
	case FCDEffectParameterSurfaceInitFactory::AS_NULL:		parameter = new FCDEffectParameterSurfaceInitAsNull(); break;
	case FCDEffectParameterSurfaceInitFactory::AS_TARGET:	parameter = new FCDEffectParameterSurfaceInitAsTarget(); break;
	case FCDEffectParameterSurfaceInitFactory::CUBE:		parameter = new FCDEffectParameterSurfaceInitCube(); break;
	case FCDEffectParameterSurfaceInitFactory::FROM:		parameter = new FCDEffectParameterSurfaceInitFrom(); break;
	case FCDEffectParameterSurfaceInitFactory::PLANAR:		parameter = new FCDEffectParameterSurfaceInitPlanar(); break;
	case FCDEffectParameterSurfaceInitFactory::VOLUME:		parameter = new FCDEffectParameterSurfaceInitVolume(); break;
	default: break;
	}

	return parameter;
}
