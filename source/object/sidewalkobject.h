#ifndef SIDEWALKOBJECT_H__
#define SIDEWALKOBJECT_H__

#include "c4d.h"


const Int32 ID_OSIDEWALK = 1024588;


class SidewalkObject : public ObjectData
{
	INSTANCEOF(SidewalkObject, ObjectData);
	
public:
	virtual Bool Init(GeListNode *node);
	virtual BaseObject* GetVirtualObjects(BaseObject *op, HierarchyHelp *hh);
	
	static NodeData *Alloc()
	{
		return NewObjClear(SidewalkObject);
	}
};


#endif // SIDEWALKOBJECT_H__
