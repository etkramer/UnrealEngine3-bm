/*
	Copyright (C) 2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#include "StdAfx.h"
#include "FCDocument/FCDSceneNode.h"
#include "FCDocument/FCDSceneNodeIterator.h"

//
// FCDSceneNodeIterator
//

FCDSceneNodeIterator::FCDSceneNodeIterator(FCDSceneNode* root)
{
	queue.push_back(root);

	// Fill in the queue completely,
	// in case the user using this iterator wants to modify stuff.
	for (size_t index = 0; index < queue.size(); ++index)
	{
		FCDSceneNode* it = queue[index];
		queue.insert(queue.end(), it->GetChildren().begin(), it->GetChildren().end());
	}

	iterator = 0;
}

FCDSceneNodeIterator::~FCDSceneNodeIterator()
{
	queue.clear();
}

FCDSceneNode* FCDSceneNodeIterator::GetNode()
{
	return iterator < queue.size() ? queue[iterator] : NULL;
}

FCDSceneNode* FCDSceneNodeIterator::Next()
{
	++iterator;
	return GetNode();
}
