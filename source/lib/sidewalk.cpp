#include "sidewalk.h"
#include "osidewalk.h"
#include "c4d_symbols.h"


BaseObject *Sidewalk::Build(BaseContainer *bc, BaseDocument *doc)
{
	// Cancel if invalid pointers
	if (!bc || !doc)
		return nullptr;
	
	// Get parameters
	_doc = doc;
	GetParametersFromContainer(*bc, *doc);
	GetObjectNames();
	
	// Calculate the total size of the sidewalk
	Vector totalSize = Vector(_params.elementSize.x * _params.countX, _params.elementSize.y, _params.elementSize.z * _params.countZ);

	// Random generators
	Random rndElementChoice; // Element selection
	rndElementChoice.Init(_params.elementRndSeed);
	
	Random holeRnd; // Missing element / hole random
	holeRnd.Init(_params.elementRndSeed * 2);

	Random plateRnd; // Plate variation
	plateRnd.Init(_params.plateRndSeed);
	
	Random cobbleRotRnd; // Cobblestone random
	cobbleRotRnd.Init(_params.cobbleRotSeed);
	
	Random cobbleCrumpleRnd; // Cobblestone Crumple variation
	cobbleCrumpleRnd.Init(_params.cobbleCrumpleSeed);
	
	Random dirtPlaneCrumpleRnd;  // Dirt Plane Crumple variation
	dirtPlaneCrumpleRnd.Init(_params.dirtPlaneCrumpleSeed);
	
	
	// Main group
	AutoAlloc<BaseObject> mainGroup(Onull);
	if (!mainGroup)
		return nullptr;
	
	// Element groups
	AutoAlloc<BaseObject> plateGroup(Onull);
	if (!plateGroup)
		return nullptr;
	
	AutoAlloc<BaseObject> cobblestoneGroup(Onull);
	if (!cobblestoneGroup)
		return nullptr;


	// Iterate sidewalk rows
	for (Int32 columnIndex = 0; columnIndex < _params.countX; ++columnIndex)
	{
		// Iterate sidewalk columns
		for (Int32 rowIndex = 0; rowIndex < _params.countZ; ++rowIndex)
		{
			// Do we create any element in this position, or just leave a hole?
			if (holeRnd.Get01() > _params.elementHoleBias)
			{
				// Do we create a plate or cobblestones?
				if (rndElementChoice.Get01() < _params.elementSelectBias)
				{
					// Create a new plate
					AutoFree<BaseObject> newPlate;
					newPlate.Set(CreateSinglePlate());
					if (!newPlate)
						return nullptr;
					
					// Position new plate (note that every 2nd row is shifted)
					Vector elementPos = Vector(_params.elementSize.x * columnIndex - _params.elementSize.x * ((Float)_params.countX - 1.0) * 0.5,
					                           0.0,
					                           _params.elementSize.z * rowIndex + _params.shift * ((columnIndex % 2 == 0) ? 1.0 : 0.0));
					
					// Compute random position variation
					elementPos += _params.plateRndPos * Vector(plateRnd.Get11(), plateRnd.Get11(), plateRnd.Get11());
					
					// Compute random rotation variation
					Vector elementRot = _params.plateRndRot * Vector(plateRnd.Get11(), plateRnd.Get11(), plateRnd.Get11());

					// Set position and rotation to plate
					newPlate->SetRelPos(elementPos);
					newPlate->SetRelRot(elementRot);
					
					// Set name
					newPlate->SetName(String::IntToString(rowIndex) + "-" + String::IntToString(columnIndex) + " (" + _params.plateName + ")");
					
					// Release plate into group
					newPlate->InsertUnderLast(plateGroup);
					newPlate.Release();
				}
				else
				{
					// Create new cobble stone group (same size as a plate)
					AutoFree<BaseObject> newCobblestones;
					newCobblestones.Set(CreateCobblestones(cobbleCrumpleRnd));
					if (!newCobblestones)
						return nullptr;
					
					// Position new coblestone group (note that every 2nd row is shifted)
					Vector elementPos = Vector(_params.elementSize.x * columnIndex - _params.elementSize.x * ((Float)_params.countX - 1.0) * 0.5,
					                           _params.cobbleElevation,
					                           _params.elementSize.z * rowIndex + _params.shift * (columnIndex % 2 == 0));
					
					newCobblestones->SetRelPos(elementPos);
					
					// Set name
					newCobblestones->SetName(String::IntToString(rowIndex) + "-" + String::IntToString(columnIndex) + " (" + _params.cobblestoneGroupName + ")");
					
					// Release new coblestones into main cobblestone group
					newCobblestones->InsertUnderLast(cobblestoneGroup);
					newCobblestones.Release();
				}
			}
		}
	}
	
	
	// If required, assign texture tag to plate group
	if (_params.plateMat && !_params.plateMatPerPlate)
	{
		if (!AddTextureTag(plateGroup, _params.plateMat, _params.plateMatScale))
			return nullptr;
	}

	// If required, assign texture tag to cobblestone group
	if (_params.cobbleMat && !_params.cobbleMatPerStone)
	{
		if (!AddTextureTag(cobblestoneGroup, _params.cobbleMat, _params.cobbleMatScale))
			return nullptr;
	}
	
	
	// Set groups' names
	mainGroup->SetName(_params.sidewalkGroupName);
	plateGroup->SetName(_params.plateGroupName);
	cobblestoneGroup->SetName(_params.cobblestoneGroupName);
	
	
	// Insert groups into mainGroup and release
	plateGroup->InsertUnderLast(mainGroup);
	plateGroup.Release();
	cobblestoneGroup->InsertUnderLast(mainGroup);
	cobblestoneGroup.Release();
	
	
	// Dirt Plane
	if (_params.dirtPlaneEnabled)
	{
		// Create Dirt Plane
		AutoFree<BaseObject> newPlane;
		newPlane.Set(CreateDirtPlane(dirtPlaneCrumpleRnd));
		if (!newPlane)
			return nullptr;
		
		// Set Name
		newPlane->SetName(_params.dirtPlaneName);
		
		// Set Position
		Vector planePos = Vector(0.0, _params.dirtPlaneElevation, totalSize.z * 0.5 - _params.elementSize.z * 0.5);
		newPlane->SetRelPos(planePos);
		
		// Release plane into mainGroup
		newPlane->InsertUnderLast(mainGroup);
		newPlane.Release();
	}
	
	// Curbstones
	if (_params.curbEnabled)
	{
		AutoFree<BaseObject> curbstoneGroup;
		curbstoneGroup.Set(CreateCurbstoneRow(totalSize.z));
		if (!curbstoneGroup)
			return nullptr;
		
		// Set Group position
		Vector groupPos = Vector(totalSize.x * 0.5 + _params.curbSize.x * 0.5, _params.curbSize.y * -0.5 + _params.elementSize.y * 0.5 + _params.curbElevation, _params.elementSize.z * -0.5);
		curbstoneGroup->SetRelPos(groupPos);
		
		// If required, assign texture tag to curbstone group
		if (_params.curbMat && !_params.curbMatPerStone)
		{
			if (!AddTextureTag(curbstoneGroup, _params.curbMat, _params.curbMatScale))
				return nullptr;
		}

		// Release curbstone group into mainGroup
		curbstoneGroup->InsertUnderLast(mainGroup);
		curbstoneGroup.Release();
	}
	
	// Release & return main group
	return mainGroup.Release();
}


BaseObject *Sidewalk::CreateSinglePlate()
{
	// Create new plate
	AutoAlloc<BaseObject> newPlate(Ocube);
	if (!newPlate)
		return nullptr;
	
	// Calculate actual size of plate (elementSize - gapSize)
	Vector gappedSize = _params.elementSize - Vector(_params.plateGap, 0.0, _params.plateGap);

	// Get plate's container
	BaseContainer *plateData = newPlate->GetDataInstance();
	if (!plateData)
		return nullptr;
	
	// Set plate parameters
	plateData->SetVector(PRIM_CUBE_LEN, gappedSize);
	plateData->SetBool(PRIM_CUBE_DOFILLET, (_params.plateFilletRad > 0.0));
	plateData->SetFloat(PRIM_CUBE_FRAD, _params.plateFilletRad);
	plateData->SetInt32(PRIM_CUBE_SUBF, _params.plateFilletSubd);
	
	// Apply Phong tag
	if (_params.plateUsePhong)
	{
		if (!AddPhongTag(newPlate))
			return nullptr;
	}
	
	// Apply material
	if (_params.plateMat && _params.plateMatPerPlate)
	{
		if (!AddTextureTag(newPlate, _params.plateMat, _params.plateMatScale))
			return nullptr;
	}
	
	// Release & return
	return newPlate.Release();
}


BaseObject *Sidewalk::CreateCobblestones(Random &rnd)
{
	// Size of a single cobblestone
	if (_params.cobbleCount == 0)
		return nullptr;
	
	Float invCobbleCount = 1.0 / _params.cobbleCount;
	Vector stoneSize = Vector(_params.elementSize.x * invCobbleCount, _params.elementSize.y, _params.elementSize.z * invCobbleCount);
	
	// Create a new Cube: That's our cobblestone prototype
	AutoAlloc<BaseObject> newPrototypeCobblestone(Ocube);
	if (!newPrototypeCobblestone)
		return nullptr;
	
	// Get new cube's container
	BaseContainer	*cobblestoneData = newPrototypeCobblestone->GetDataInstance();
	if (!cobblestoneData)
		return nullptr;
	
	// Calculate actual size
	Vector gappedStoneSize = stoneSize - Vector(_params.cobbleGap, 0.0, _params.cobbleGap);
	
	// Set basic cobblestone parameters
	cobblestoneData->SetVector(PRIM_CUBE_LEN, gappedStoneSize);
	cobblestoneData->SetInt32(PRIM_CUBE_SUBX, _params.cobbleSubdiv);
	cobblestoneData->SetInt32(PRIM_CUBE_SUBY, _params.cobbleSubdiv);
	cobblestoneData->SetInt32(PRIM_CUBE_SUBZ, _params.cobbleSubdiv);
	cobblestoneData->SetBool(PRIM_CUBE_DOFILLET, _params.cobbleFilletRad > 0.0);
	cobblestoneData->SetFloat(PRIM_CUBE_FRAD, _params.cobbleFilletRad);
	cobblestoneData->SetInt32(PRIM_CUBE_SUBF, _params.cobbleFilletSubd);
	
	// Get cobblestone polygon object: That's our 'polygonized' cobblestone prototype
	AutoFree<PolygonObject>	cobblePoly;
	cobblePoly.Set(static_cast<PolygonObject*>(MakeEditable(newPrototypeCobblestone, _doc)));
	if (!cobblePoly)
		return nullptr;
	
	// Crumple cobblestone geometry
	if (_params.cobbleCrumple > 0.0)
		CrumpleGeometry(cobblePoly, _params.cobbleCrumple, rnd);
	
	// Attach Phong Tag
	if (_params.cobbleUsePhong)
	{
		if (!AddPhongTag(cobblePoly))
			return nullptr;
	}
	
	// Apply material
	if (_params.cobbleMat && _params.cobbleMatPerStone)
	{
		if (!AddTextureTag(cobblePoly, _params.cobbleMat, _params.cobbleMatScale))
			return nullptr;
	}
	
	// Create Null Object to group Cobblestones in
	AutoAlloc<BaseObject> cobbleGroup(Onull);
	if (!cobbleGroup)
		return nullptr;
	
	// Set group name
	cobbleGroup->SetName(_params.cobblestoneGroupName);
	
	// Iterate & create all cobblestones
	for (Int32 columsIndex = 0; columsIndex < _params.cobbleCount; ++columsIndex)
	{
		for (Int32 rowIndex = 0; rowIndex < _params.cobbleCount; ++rowIndex)
		{
			// Create a single cobblestone
			AutoFree<PolygonObject> newCobblestone;
			newCobblestone.Set(static_cast<PolygonObject*>(cobblePoly->GetClone(COPYFLAGS_0, nullptr)));
			if (!newCobblestone)
				return nullptr;
			
			// Set name to new cobblestone
			newCobblestone->SetName(_params.cobblestoneName + " " + String::IntToString(columsIndex) + "-" + String::IntToString(rowIndex));
			
			// Calculate position for new stone
			Vector cobblePos = Vector(stoneSize.x * columsIndex - stoneSize.x * ((Float)_params.cobbleCount - 1.0) * 0.5,
			                          0.0,
			                          stoneSize.z * rowIndex - stoneSize.z * ((Float)_params.cobbleCount - 1.0) * 0.5);
			
			// Calculate position variation
			cobblePos += Vector(_params.cobbleRndPos.x * rnd.GetG11(),
			                    _params.cobbleRndPos.y * rnd.GetG11(),
			                    _params.cobbleRndPos.z * rnd.GetG11());
			
			// Calculate basic rotation (randomly rotating the stone by 0°, 90°, 180° or 270°)
			Vector cobbleRot = Vector(GetHardRndAngle(rnd, RANDOMANGLE::GETALL), 0.0, GetHardRndAngle(rnd, RANDOMANGLE::GET180));
			
			// Calculate rotation variation
			cobbleRot += Vector(_params.cobbleRndRot.x * rnd.GetG11(),
			                    _params.cobbleRndRot.y * rnd.GetG11(),
			                    _params.cobbleRndRot.z * rnd.GetG11());
			
			// Set position and rotation to stone
			newCobblestone->SetRelPos(cobblePos);
			newCobblestone->SetRelRot(cobbleRot);
			
			// Release to group
			newCobblestone->InsertUnderLast(cobbleGroup);
			newCobblestone.Release();
		}
	}
	
	// Release & return result
	return cobbleGroup.Release();
}


BaseObject *Sidewalk::CreateDirtPlane(Random &rnd)
{
	// Create Plane primitive
	AutoAlloc<BaseObject> newPlane(Oplane);
	if (!newPlane)
		return nullptr;
	
	// Get Plane container
	BaseContainer *planeData = newPlane->GetDataInstance();
	if (!planeData)
		return nullptr;
	
	// Plan a little extra width, in case the sidewalk also has curbstones
	// If we don't do this, there might be a visible gap between the plane and the crumpled curbstone.
	// The exact value is not important, it should just somehow close the gap
	Float extraWidth = _params.curbCrumpleVal * 5.0;
	
	// Set Plane attributes
	planeData->SetFloat(PRIM_PLANE_WIDTH, _params.elementSize.x * _params.countX + extraWidth);
	planeData->SetFloat(PRIM_PLANE_HEIGHT, _params.elementSize.z * _params.countZ);
	planeData->SetInt32(PRIM_PLANE_SUBW, _params.dirtPlaneSubd);
	planeData->SetInt32(PRIM_PLANE_SUBH, _params.dirtPlaneSubd);
	
	// Prepare Polygon Plane
	AutoFree<PolygonObject> polyPlane;
	polyPlane.Set(static_cast<PolygonObject*>(MakeEditable(newPlane, _doc)));
	if (!polyPlane)
		return nullptr;
	
	// Crumple
	if (_params.dirtPlaneCrumple > 0.0)
		CrumpleGeometry(polyPlane, _params.dirtPlaneCrumple, rnd);
	
	// Apply Phong Tag
	if (!AddPhongTag(polyPlane))
		return nullptr;
	
	// Apply material
	if (_params.dirtPlaneMat)
	{
		AddTextureTag(polyPlane, _params.dirtPlaneMat, _params.dirtPlaneMatScale);
	}
	
	// Release & return Result
	return polyPlane.Release();
}


// Create a Curbstone
BaseObject *Sidewalk::CreateSingleCurbstone(Vector &stoneSize, Random &sizeRnd, Random &crumpleRnd)
{
	// Create Cube primitive
	AutoAlloc<BaseObject> newPrototypeStone(Ocube);
	if (!newPrototypeStone)
		return nullptr;
	
	// Calculate random length variation
	stoneSize.z += stoneSize.z * sizeRnd.Get11() * _params.curbSizeVar;
	
	// Get stone's container
	BaseContainer *stoneData = newPrototypeStone->GetDataInstance();
	if (!stoneData)
		return nullptr;
	
	// Set Stone's basic attributes
	stoneData->SetVector(PRIM_CUBE_LEN, stoneSize);
	stoneData->SetInt32(PRIM_CUBE_SUBX, _params.curbSubd);
	stoneData->SetInt32(PRIM_CUBE_SUBY, _params.curbSubd);
	stoneData->SetInt32(PRIM_CUBE_SUBZ, _params.curbSubd);
	stoneData->SetBool(PRIM_CUBE_DOFILLET, _params.curbFilletRad > 0.0);
	stoneData->SetFloat(PRIM_CUBE_FRAD, _params.curbFilletRad);
	stoneData->SetInt32(PRIM_CUBE_SUBF, _params.curbFilletSubd);
	
	// Convert Stone to Polygon Object
	AutoFree<PolygonObject> stonePoly;
	stonePoly.Set(static_cast<PolygonObject*>(MakeEditable(newPrototypeStone, _doc)));
	if (!stonePoly)
		return nullptr;
	
	// Crumple Stone geometry
	if (_params.curbCrumpleVal > 0.0)
		CrumpleGeometry(stonePoly, _params.curbCrumpleVal, crumpleRnd);
		
	
	// Apply Phong Tag
	if (!AddPhongTag(stonePoly))
		return nullptr;
	
	// Return result
	return stonePoly.Release();
}


// Create row of Curbstones
BaseObject *Sidewalk::CreateCurbstoneRow(Float totalSpace)
{
	if (_params.curbCount < 1)
		return nullptr;
	
	AutoAlloc<BaseObject> stoneGroup(Onull);
	if (!stoneGroup)
		return nullptr;
	
	// Set group name
	stoneGroup->SetName(_params.curbstoneGroupName);
	
	// Random generator
	Random curbstoneCrumpleRnd;
	curbstoneCrumpleRnd.Init(_params.curbSizeSeed);
	
	Random curbstoneSizeRnd;
	curbstoneSizeRnd.Init(_params.curbSizeSeed);
	
	// Initialize remaining space
	Float remainingSpace = totalSpace;
	
	// Calculate minimum space required for one curbstone (can't be less than twice the space needed for the curbstone fillet)
	Float minimumRequiredSpace = _params.curbFilletRad * 2.0;
	
	// Create all curbstones except the last one
	for (Int32 stoneIndex = 0; (stoneIndex < _params.curbCount) && (remainingSpace > minimumRequiredSpace); ++stoneIndex)
	{
		// Calculate stone size (available space / stone count)
		Vector stoneSize = _params.curbSize;
		stoneSize.z = (Float)(totalSpace / _params.curbCount);
		
		// Create new stone
		AutoFree<BaseObject> newStone;
		newStone.Set(CreateSingleCurbstone(stoneSize, curbstoneSizeRnd, curbstoneCrumpleRnd));
		if (!newStone)
			return nullptr;
		
		// Set stone position
		Vector stonePos = Vector(0.0, 0.0, totalSpace - remainingSpace + stoneSize.z * 0.5);
		newStone->SetRelPos(stonePos);
		
		// Update remaining space
		remainingSpace -= stoneSize.z;
		
		// If required, assign texture tag to cobblestone group
		if (_params.curbMat && _params.curbMatPerStone)
		{
			if (!AddTextureTag(newStone, _params.curbMat, _params.curbMatScale))
				return nullptr;
		}
		
		// Set stone name
		newStone->SetName(_params.curbstoneName + " (" + String::IntToString(stoneIndex) + ")");
		
		// Insert into stone group
		newStone->InsertUnderLast(stoneGroup);
		newStone.Release();
	}
	
	// Create the last stone, if there's still enough space
	/*
	if (remainingLength > _params.curbFilletRad * 2.0)
	{
		// Calculate Stone size
		stoneSize.z = remainingLength;
		
		// Create new stone
		AutoFree<BaseObject> newStone;
		newStone.Set(CreateCurbstone(curbstoneCrumpleRnd));
		if (!newStone)
			return nullptr;
		
		// Set stone position
		stonePos = Vector(0.0, 0.0, totalSize.z - stoneSize.z * 0.5);
		newStone->SetRelPos(stonePos);
		
		// If required, assign texture tag to cobblestone group
		if (_params.curbMat && _params.curbMatPerStone)
			AddTextureTag(newStone, _params.curbMat, _params.curbMatScale);
		
		// Set stone name
		newStone->SetName(_params.curbstoneName + " (" + String::IntToString(stoneIndex) + ")");
		
		// Insert into stone group
		newStone->InsertUnderLast(stoneGroup);
		newStone.Release();
	}
	 */
	
	// Release & return result
	return stoneGroup.Release();
}


Bool Sidewalk::AddTextureTag(BaseObject *op, BaseMaterial *mat, Float matScale)
{
	if (op && mat)
	{
		TextureTag *matTag = TextureTag::Alloc();
		if (!matTag)
			return false;

		// Link material
		matTag->SetMaterial(mat);

		// Set projection to Cubic
		GeData dat = GeData(TEXTURETAG_PROJECTION_CUBIC);
		matTag->SetParameter(DescID(TEXTURETAG_PROJECTION), dat, DESCFLAGS_SET_0);

		// Set texture scale
		dat = GeData(matScale);
		matTag->SetParameter(DescID(TEXTURETAG_LENGTHX), dat, DESCFLAGS_SET_0);
		matTag->SetParameter(DescID(TEXTURETAG_LENGTHY), dat, DESCFLAGS_SET_0);

		// Attach texture tag to object
		op->InsertTag(matTag);
	}

	return true;
}


Bool Sidewalk::AddPhongTag(BaseObject *op, Bool angleLimit, Float angle)
{
	if (!op)
		return false;

	BaseTag *phongTag = op->MakeTag(Tphong);
	if (!phongTag)
		return false;

	BaseContainer *phongTagData = phongTag->GetDataInstance();
	if (!phongTagData)
		return false;

	phongTagData->SetBool(PHONGTAG_PHONG_ANGLELIMIT, angleLimit);
	phongTagData->SetFloat(PHONGTAG_PHONG_ANGLE, angle);

	return true;
}


void Sidewalk::GetParametersFromContainer(const BaseContainer &bc, const BaseDocument &doc)
{
	// General Parameters
	_params.elementSize = bc.GetVector(SIDEWALK_ELEMENT_SIZE);
	_params.countX = bc.GetInt32(SIDEWALK_COUNT_X);
	_params.countZ = bc.GetInt32(SIDEWALK_COUNT_Z);
	_params.shift = bc.GetFloat(SIDEWALK_SHIFT);
	_params.elementRndSeed = bc.GetInt32(SIDEWALK_ELEMENT_SEED);
	_params.elementSelectBias = 1.0 - ((bc.GetFloat(SIDEWALK_ELEMENT_SELBIAS) + 1.0) * 0.5);
	_params.elementHoleBias = bc.GetFloat(SIDEWALK_ELEMENT_HOLEBIAS);
	
	// Plates Parameters
	_params.plateGap = bc.GetFloat(SIDEWALK_PLATES_SPACE);
	_params.plateFilletRad = bc.GetFloat(SIDEWALK_PLATES_FILLET_RAD);
	_params.plateFilletSubd = bc.GetInt32(SIDEWALK_PLATES_FILLET_SUBD);
	_params.plateUsePhong = bc.GetBool(SIDEWALK_PLATES_PHONG);
	
	_params.plateRndRot = bc.GetVector(SIDEWALK_PLATES_RND_ROT);
	_params.plateRndPos = bc.GetVector(SIDEWALK_PLATES_RND_POS);
	_params.plateRndSeed = bc.GetInt32(SIDEWALK_PLATES_RND_SEED);
	
	_params.plateMat = bc.GetMaterialLink(SIDEWALK_PLATES_MAT_LINK, &doc);
	_params.plateMatPerPlate = bc.GetBool(SIDEWALK_PLATES_MAT_EACH);
	_params.plateMatScale = bc.GetFloat(SIDEWALK_PLATES_MAT_SCALE);
	
	// Cobblestones Parameters
	_params.cobbleCount = bc.GetInt32(SIDEWALK_COBBLE_COUNT);
	_params.cobbleElevation = bc.GetFloat(SIDEWALK_COBBLE_ELEVATION);
	
	_params.cobbleSubdiv = bc.GetInt32(SIDEWALK_COBBLE_SUBD);
	_params.cobbleCrumple = bc.GetFloat(SIDEWALK_COBBLE_CRUMPLE);
	_params.cobbleCrumpleSeed = 9876;
	_params.cobbleRotSeed = 4567;
	
	_params.cobbleGap = bc.GetFloat(SIDEWALK_COBBLE_SPACE);
	_params.cobbleFilletRad = bc.GetFloat(SIDEWALK_COBBLE_FILLET_RAD);
	_params.cobbleFilletSubd = bc.GetInt32(SIDEWALK_COBBLE_FILLET_SUBD);
	_params.cobbleUsePhong = bc.GetBool(SIDEWALK_COBBLE_PHONG);
	
	_params.cobbleRndRot = bc.GetVector(SIDEWALK_COBBLE_RND_ROT);
	_params.cobbleRndPos = bc.GetVector(SIDEWALK_COBBLE_RND_POS);
	_params.cobbleRndSeed = bc.GetInt32(SIDEWALK_COBBLE_RND_SEED);
	
	_params.cobbleMat = bc.GetMaterialLink(SIDEWALK_COBBLE_MAT_LINK, &doc);
	_params.cobbleMatPerStone = bc.GetBool(SIDEWALK_COBBLE_MAT_EACH);
	_params.cobbleMatScale = bc.GetFloat(SIDEWALK_COBBLE_MAT_SCALE);
	
	// Dirt Plane Parameters
	_params.dirtPlaneEnabled = bc.GetBool(SIDEWALK_USE_DIRT);
	_params.dirtPlaneCrumple = bc.GetFloat(SIDEWALK_DIRT_CRUMPLE);
	_params.dirtPlaneSubd = bc.GetInt32(SIDEWALK_DIRT_SUBD);
	_params.dirtPlaneCrumpleSeed = bc.GetInt32(SIDEWALK_DIRT_SEED);
	_params.dirtPlaneElevation = bc.GetFloat(SIDEWALK_DIRT_ELEVATION);
	
	_params.dirtPlaneMat = bc.GetMaterialLink(SIDEWALK_DIRT_MAT_LINK, &doc);
	_params.dirtPlaneMatScale = bc.GetFloat(SIDEWALK_DIRT_MAT_SCALE);
	
	// Curbstone Parameters
	_params.curbEnabled = bc.GetBool(SIDEWALK_USE_CURB);
	_params.curbSize.x = bc.GetFloat(SIDEWALK_CURB_SIZE_X);
	_params.curbSize.y = bc.GetFloat(SIDEWALK_CURB_SIZE_Y);
	_params.curbCount = bc.GetInt32(SIDEWALK_CURB_COUNT);
	_params.curbCrumpleVal = bc.GetFloat(SIDEWALK_CURB_CRUMPLE_VAL);
	_params.curbFilletRad = bc.GetFloat(SIDEWALK_CURB_FILLET_RAD);
	_params.curbFilletSubd = bc.GetInt32(SIDEWALK_CURB_FILLET_SUBD);
	_params.curbSizeVar = bc.GetFloat(SIDEWALK_CURB_VARIATION);
	_params.curbSizeSeed = bc.GetInt32(SIDEWALK_CURB_VARIATION_SEED);
	_params.curbSubd = bc.GetInt32(SIDEWALK_CURB_SUBD);
	_params.curbElevation = bc.GetFloat(SIDEWALK_CURB_ELEVATION);
	
	_params.curbMat = bc.GetMaterialLink(SIDEWALK_CURB_MAT_LINK, &doc);
	_params.curbMatScale = bc.GetFloat(SIDEWALK_CURB_MAT_SCALE);
	_params.curbMatPerStone = bc.GetBool(SIDEWALK_PLATES_MAT_EACH);
}


void Sidewalk::GetObjectNames()
{
	_params.sidewalkGroupName = GeLoadString(IDS_OSIDEWALK);
	_params.plateGroupName = GeLoadString(IDS_OBJ_PLATE_GROUP);
	_params.cobblestoneGroupName = GeLoadString(IDS_OBJ_COBBLESTONE_GROUP);
	_params.curbstoneGroupName = GeLoadString(IDS_OBJ_CURBSTONE_GROUP);
	_params.plateName = GeLoadString(IDS_OBJ_PLATE);
	_params.cobblestoneName = GeLoadString(IDS_OBJ_COBBLESTONE);
	_params.dirtPlaneName = GeLoadString(IDS_OBJ_PLATE);
	_params.curbstoneName = GeLoadString(IDS_OBJ_CURBSTONE);
}


BaseObject *MakeEditable(BaseObject *op, BaseDocument *doc)
{
	if (!op || !doc)
		return nullptr;
	
	BaseObject *result = nullptr;
	
	AutoAlloc<BaseDocument> tmpDoc;
	BaseObject *tmpOp = static_cast<BaseObject*>(op->GetClone(COPYFLAGS_0, nullptr));
	tmpDoc->InsertObject(tmpOp, nullptr, nullptr);
	
	ModelingCommandData cd;
	cd.doc = tmpDoc;
	cd.op = tmpOp;
	
	if (SendModelingCommand(MCOMMAND_MAKEEDITABLE, cd))
	{
		BaseObject *commandResult = static_cast<BaseObject*>(cd.result->GetIndex(0));
		if (!commandResult)
			return nullptr;
		
		result = static_cast<BaseObject*>(commandResult->GetClone(COPYFLAGS_NO_ANIMATION|COPYFLAGS_NO_BITS, nullptr));
		
		BaseObject::Free(commandResult);
	}
	
	return result;
}


Vector GetVertexNormal(PolygonObject *op, Neighbor *neighbor, Int32 pointIndex)
{
	// Variables
	CPolygon *pNeighborPoly = nullptr;
	Int32 *neighborFaceArr = nullptr;
	Int32 neighborFaceCount = 0;
	CPolygon *polygonArr = op->GetPolygonW();
	Vector *pointArr = op->GetPointW();
	Vector resultNormal;
	
	// Get polygons attached to point
	neighbor->GetPointPolys(pointIndex, &neighborFaceArr, &neighborFaceCount);
	if (neighborFaceCount < 1)
		return resultNormal;
	
	for(Int32 faceIndex = 0; faceIndex < neighborFaceCount; ++faceIndex)
	{
		pNeighborPoly = &polygonArr[neighborFaceArr[faceIndex]];
		
		// Compute face normal
		Vector v1 = pointArr[pNeighborPoly->b] - pointArr[pNeighborPoly->a];
		Vector v2 = pointArr[pNeighborPoly->c] - pointArr[pNeighborPoly->a];
		
		// Get cross-product
		resultNormal += Cross(v1, v2);
	}
	resultNormal /= neighborFaceCount;
	
	// Return resulting normal vector
	return resultNormal.GetNormalized();
}


void CrumpleGeometry(PolygonObject *op, Float strength, Random &rnd)
{
	if (!op)
		return;
	
	Int32 pointCount = op->GetPointCount();
	Int32 polygonCount = op->GetPolygonCount();
	
	Vector *pointArr = op->GetPointW();
	
	Neighbor nb;
	if (!nb.Init(pointCount, op->GetPolygonW(), polygonCount, nullptr)) return;
	
	for (Int32 i = 0; i < pointCount; i++)
	{
		pointArr[i] += GetVertexNormal(op, &nb, i) * strength * rnd.Get11();
	}
}


Float GetHardRndAngle(Random &rnd, RANDOMANGLE mode)
{
	Float x = 0.0;
	
	switch (mode)
	{
		case RANDOMANGLE::GETALL:
			x = Rad(Round(rnd.Get11() * 4.0) * 90.0);
			break;
			
		case RANDOMANGLE::GET180:
			x = Rad(Round(rnd.Get11()) * 180.0);
			break;
	}
	return x;
}
