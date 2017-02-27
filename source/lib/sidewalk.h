#ifndef SIDEWALK_H__
#define SIDEWALK_H__

#include "c4d.h"
#include "lib_noise.h"


/// Options for GetHardRndAngle()
enum class RANDOMANGLE
{
	GETALL =	0,     ///< Get all kinds of random angles
	GET180 =	1      ///< Get only angles divideable by 180°
} ENUM_END_LIST(RANDOMANGLE);


/// This class builds a complete sidewalk from separate objects.
class Sidewalk
{
	MAXON_DISALLOW_COPY_AND_ASSIGN(Sidewalk);

public:

	/// This struct holds all the parameters needed to build a complete sidewalk.
	struct Parameters
	{
		// General Parameters
		Vector elementSize;
		Int32 countX;
		Int32 countZ;
		Float shift;
		Int32 elementRndSeed;
		Float elementSelectBias;
		Float elementHoleBias;

		// Plates Parameters
		Float plateGap;
		Float plateFilletRad;
		Int32 plateFilletSubd;
		Bool plateUsePhong;

		Vector plateRndRot;
		Vector plateRndPos;
		Int32 plateRndSeed;

		BaseMaterial *plateMat;
		Bool plateMatPerPlate;
		Float plateMatScale;

		// Cobblestones Parameters
		Int32 cobbleCount;
		Float cobbleElevation;

		Int32 cobbleSubdiv;
		Float cobbleCrumple;
		Int32 cobbleCrumpleSeed;
		Int32 cobbleRotSeed;

		Float cobbleGap;
		Float cobbleFilletRad;
		Int32 cobbleFilletSubd;
		Bool cobbleUsePhong;

		Vector cobbleRndRot;
		Vector cobbleRndPos;
		Int32 cobbleRndSeed;

		BaseMaterial *cobbleMat;
		Bool cobbleMatPerStone;
		Float cobbleMatScale;

		// Dirt Plane Parameters
		Bool dirtPlaneEnabled;
		Int32 dirtPlaneSubd;
		Float dirtPlaneCrumple;
		Int32 dirtPlaneCrumpleSeed;
		Float dirtPlaneElevation;

		BaseMaterial *dirtPlaneMat;
		Float dirtPlaneMatScale;

		// Curbstone Parameters
		Bool curbEnabled;
		Vector curbSize;
		Int32 curbCount;
		Int32 curbSubd;
		Float curbCrumpleVal;
		Float curbFilletRad;
		Int32 curbFilletSubd;
		Float curbSizeVar;
		Int32 curbSizeSeed;
		Float curbElevation;

		BaseMaterial *curbMat;
		Float curbMatScale;
		Bool curbMatPerStone;

		// Component group names
		String sidewalkGroupName;
		String plateGroupName;
		String cobblestoneGroupName;
		String curbstoneGroupName;

		// Component names
		String plateName;
		String cobblestoneName;
		String dirtPlaneName;
		String curbstoneName;

		/// Default constructor
		Parameters() : countX(0), countZ(0), shift(0.0), elementRndSeed(0), elementSelectBias(0.0), elementHoleBias(0.0),
		               plateGap(0.0), plateFilletRad(0.0), plateFilletSubd(0), plateUsePhong(false),
		               plateRndRot(0.0), plateRndPos(0.0), plateRndSeed(0),
		               plateMat(nullptr), plateMatPerPlate(false), plateMatScale(0.0),
		               cobbleCount(0), cobbleElevation(0.0),
		               cobbleSubdiv(0), cobbleCrumple(0.0), cobbleCrumpleSeed(0), cobbleRotSeed(0),
		               cobbleGap(0.0), cobbleFilletRad(0.0), cobbleFilletSubd(0), cobbleUsePhong(false),
		               cobbleRndSeed(0),
		               cobbleMat(nullptr), cobbleMatPerStone(false), cobbleMatScale(0.0),
		               dirtPlaneEnabled(false), dirtPlaneSubd(0), dirtPlaneCrumple(0.0), dirtPlaneCrumpleSeed(0), dirtPlaneElevation(0.0),
		               dirtPlaneMat(nullptr), dirtPlaneMatScale(0.0),
		               curbEnabled(false), curbCount(0), curbSubd(0), curbCrumpleVal(0.0), curbFilletRad(0.0), curbFilletSubd(0), curbSizeVar(0.0), curbSizeSeed(0), curbElevation(0.0),
		               curbMat(nullptr), curbMatScale(0.0), curbMatPerStone(false)
		{}
	};

public:
	/// Build a complete sidewalk
	/// @return Pointer to the parent object of the sidewalk object hierarchy; or nullptr if an error occurred. Caller owns the pointed object.
	BaseObject *Build(BaseContainer *bc, BaseDocument *doc);

private:
	/// Get all sidewalk parameters from a BaseContainer and copy them to _params
	void GetParametersFromContainer(const BaseContainer &bc, const BaseDocument &doc);
	
	// Get all object and group names from the string resource and copy them to _params
	void GetObjectNames();
	
	/// Create a single plate
	/// @return Pointer to a new plate. Caller owns the pointed object.
	BaseObject *CreateSinglePlate();
	
	/// Create a group of cobblestones (same size as a plate)
	/// @return Pointer to a new group of cobblestones. Caller owns the pointed object.
	BaseObject *CreateCobblestones(Random &rnd);
	
	/// Create the dirt plane
	/// @return Pointer to a new dirt plane. Caller owns the pointed object.
	BaseObject *CreateDirtPlane(Random &rnd);
	
	/// Create a curbstone
	/// @param[in,out] stoneSize The size for this curbstone. Assigned the final actual size value (changed by variation).
	/// @return Pointer to a new curbstone. Caller owns the pointed object.
	BaseObject *CreateSingleCurbstone(Vector &stoneSize, Random &sizeRnd, Random &crumpleRnd);
	
	/// Create a row of curbstones
	/// @return Pointer to a new row of curbstones. Caller owns the pointed object.
	BaseObject *CreateCurbstoneRow(Float totalSpace);
	
	/// Add a texture tag to op
	/// @param[in] op Pointer to the object that should receive the new texture tag
	/// @param[in] mat Pointer to the material that should be linked in the texture tag
	/// @param[in] matScale Scale value for the projection in the texture tag
	/// @return False if an error occurred and the tag could not be added; otherwise true
	static Bool AddTextureTag(BaseObject *op, BaseMaterial *mat, Float matScale);
	
	/// Add a phong tag to op
	/// @return False if an error occurred and the tag could not be added; otherwise true
	static Bool AddPhongTag(BaseObject *op, Bool angleLimit = true, Float angle = Rad(89.9));

private:
	Sidewalk::Parameters _params;
	BaseDocument *_doc;

public:
	/// Default constructor
	Sidewalk()
	{}
};


/// Make a generator object editable
/// @return The editable object. Caller owns the pointed object.
BaseObject *MakeEditable(BaseObject *op, BaseDocument *doc);

/// Return the normal vector for a vertex of a polygon object
/// Needs an initialized Neighbor class
Vector GetVertexNormal(PolygonObject *op, Neighbor *neighbor, Int32 pointIndex);

/// Crumple a geometry, using the vertex normals as displacement direction
void CrumpleGeometry(PolygonObject *op, Float strength, Random &rnd);

/// Returns 0°, 90°, 180° or 270° in radians
Float GetHardRndAngle(Random &rnd, RANDOMANGLE mode);


#endif //SIDEWALK_H__
