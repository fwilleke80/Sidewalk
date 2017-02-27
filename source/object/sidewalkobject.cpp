#include "c4d.h"
#include "main.h"
#include "sidewalk.h"
#include "sidewalkdefaults.h"
#include "sidewalkobject.h"
#include "c4d_symbols.h"
#include "osidewalk.h"


Bool SidewalkObject::Init(GeListNode *node)
{
	if (!node)
		return false;
	
	BaseObject *op = static_cast<BaseObject*>(node);
	
	BaseContainer *data = op->GetDataInstance();
	if (!data)
		return false;
	
	// General
	data->SetVector(SIDEWALK_ELEMENT_SIZE, DEF_SIDEWALK_ELEMENT_SIZE);
	data->SetFloat(SIDEWALK_COUNT_X, DEF_SIDEWALK_COUNT_X);
	data->SetFloat(SIDEWALK_COUNT_Z, DEF_SIDEWALK_COUNT_Z);
	data->SetFloat(SIDEWALK_SHIFT, DEF_SIDEWALK_SHIFT);
	data->SetFloat(SIDEWALK_ELEMENT_SELBIAS, DEF_SIDEWALK_ELEMENT_SELBIAS);
	data->SetInt32(SIDEWALK_ELEMENT_SEED, DEF_SIDEWALK_ELEMENT_SEED);
	
	// Plates
	data->SetFloat(SIDEWALK_PLATES_SPACE, DEF_SIDEWALK_PLATES_SPACE);
	data->SetFloat(SIDEWALK_PLATES_FILLET_RAD, DEF_SIDEWALK_PLATES_FILLET_RAD);
	data->SetInt32(SIDEWALK_PLATES_FILLET_SUBD, DEF_SIDEWALK_PLATES_FILLET_SUBD);
	data->SetBool(SIDEWALK_PLATES_PHONG, DEF_SIDEWALK_PLATES_PHONG);
	data->SetVector(SIDEWALK_PLATES_RND_ROT, DEF_SIDEWALK_PLATES_RND_ROT);
	data->SetVector(SIDEWALK_PLATES_RND_POS, DEF_SIDEWALK_COBBLE_RND_POS);
	data->SetInt32(SIDEWALK_PLATES_RND_SEED, DEF_SIDEWALK_PLATES_RND_SEED);
	data->SetFloat(SIDEWALK_PLATES_MAT_SCALE, DEF_SIDEWALK_PLATES_MAT_SCALE);
	
	// Cobblestones
	data->SetInt32(SIDEWALK_COBBLE_COUNT, DEF_SIDEWALK_COBBLE_COUNT);
	data->SetInt32(SIDEWALK_COBBLE_SUBD, DEF_SIDEWALK_COBBLE_SUBD);
	data->SetFloat(SIDEWALK_COBBLE_CRUMPLE, DEF_SIDEWALK_COBBLE_CRUMPLE);
	data->SetFloat(SIDEWALK_COBBLE_SPACE, DEF_SIDEWALK_COBBLE_SPACE);
	data->SetFloat(SIDEWALK_COBBLE_FILLET_RAD, DEF_SIDEWALK_COBBLE_FILLET_RAD);
	data->SetInt32(SIDEWALK_COBBLE_FILLET_SUBD, DEF_SIDEWALK_COBBLE_FILLET_SUBD);
	data->SetBool(SIDEWALK_COBBLE_PHONG, DEF_SIDEWALK_COBBLE_PHONG);
	data->SetVector(SIDEWALK_COBBLE_RND_ROT, DEF_SIDEWALK_COBBLE_RND_ROT);
	data->SetVector(SIDEWALK_COBBLE_RND_POS, DEF_SIDEWALK_COBBLE_RND_POS);
	data->SetInt32(SIDEWALK_COBBLE_RND_SEED, DEF_SIDEWALK_COBBLE_RND_SEED);
	data->SetFloat(SIDEWALK_COBBLE_MAT_SCALE, DEF_SIDEWALK_COBBLE_MAT_SCALE);
	
	// Dirt Plane
	data->SetBool(SIDEWALK_USE_DIRT, DEF_SIDEWALK_USE_DIRT);
	data->SetInt32(SIDEWALK_DIRT_SUBD, DEF_SIDEWALK_DIRT_SUBD);
	data->SetFloat(SIDEWALK_DIRT_CRUMPLE, DEF_SIDEWALK_DIRT_CRUMPLE);
	data->SetFloat(SIDEWALK_DIRT_ELEVATION, DEF_SIDEWALK_DIRT_ELEVATION);
	data->SetInt32(SIDEWALK_DIRT_SEED, DEF_SIDEWALK_DIRT_SEED);
	data->SetFloat(SIDEWALK_DIRT_MAT_SCALE, DEF_SIDEWALK_DIRT_MAT_SCALE);
	
	// Curbstones
	data->SetBool(SIDEWALK_USE_CURB, DEF_SIDEWALK_USE_CURB);
	data->SetInt32(SIDEWALK_CURB_COUNT, DEF_SIDEWALK_CURB_COUNT);
	data->SetFloat(SIDEWALK_CURB_SIZE_X, DEF_SIDEWALK_CURB_SIZE_X);
	data->SetFloat(SIDEWALK_CURB_SIZE_Y, DEF_SIDEWALK_CURB_SIZE_Y);
	data->SetFloat(SIDEWALK_CURB_VARIATION, DEF_SIDEWALK_CURB_VARIATION);
	data->SetInt32(SIDEWALK_CURB_VARIATION_SEED, DEF_SIDEWALK_CURB_VARIATION_SEED);
	data->SetFloat(SIDEWALK_CURB_CRUMPLE_VAL, DEF_SIDEWALK_CURB_CRUMPLE_VAL);
	data->SetFloat(SIDEWALK_CURB_ELEVATION, DEF_SIDEWALK_CURB_ELEVATION);
	data->SetInt32(SIDEWALK_CURB_SUBD, DEF_SIDEWALK_CURB_SUBD);
	data->SetFloat(SIDEWALK_CURB_FILLET_RAD, DEF_SIDEWALK_CURB_FILLET_RAD);
	data->SetInt32(SIDEWALK_CURB_FILLET_SUBD, DEF_SIDEWALK_CURB_FILLET_SUBD);
	data->SetFloat(SIDEWALK_CURB_MAT_SCALE, DEF_SIDEWALK_CURB_MAT_SCALE);
	
	return SUPER::Init(node);
}


BaseObject *SidewalkObject::GetVirtualObjects(BaseObject *op, HierarchyHelp *hh)
{
	if (!op || !hh)
		return nullptr;
	
	// Caching
	Bool dirty = op->CheckCache(hh) || op->IsDirty(DIRTYFLAGS_DATA);
	if (!dirty)
		return op->GetCache(hh);

	// Get object container
	BaseContainer *bc = op->GetDataInstance();
	if (!bc)
		return nullptr;
	
	// Get document
	BaseDocument *doc = hh->GetDocument();
	if (!doc)
		return nullptr;

	// Create & return sidewalk
	Sidewalk sidewalk;
	return sidewalk.Build(bc, doc);
}


Bool RegisterSidewalkObject()
{
	return RegisterObjectPlugin(ID_OSIDEWALK, GeLoadString(IDS_OSIDEWALK), OBJECT_GENERATOR, SidewalkObject::Alloc, "oSidewalk", AutoBitmap("osidewalk.tif"), 0);
}
