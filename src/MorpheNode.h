// -----------------------------------------------------------------------------
// MorpheNode.h - C++ File
// -----------------------------------------------------------------------------

#ifndef MORPHE_NODE_H
#define MORPHE_NODE_H
#define MORPHE_ID          0x32000001


//
// Includes
//
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MFloatArray.h>
#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnMesh.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MGlobal.h>
#include <maya/MItGeometry.h>
#include <maya/MMatrix.h>
#include <maya/MPlug.h>
#include <maya/MPointArray.h>
#include <maya/MPxDeformerNode.h>
#include <maya/MTypeId.h>
// -----------------------------------------------------------------------------


//
// MorpheNode - Class Definition
//
class MorpheNode : public MPxDeformerNode
{
   public:
                        MorpheNode();
      virtual           ~MorpheNode(); 
   
      static  MStatus   GetWeights(MDataBlock &data, MFloatArray &weights);
      static  MStatus   GetTargetsDeltas(MDataBlock &data, MItGeometry &itGeo, float &fEnv, MFloatArray &weights, MPointArray &deltas);
      virtual MStatus   deform(MDataBlock &data, MItGeometry &itGeo, const MMatrix &localToWorldMatrix, unsigned int mIndex);
   
      static  void*     creator();
      static  MStatus   initialize();

   public:

      // The typeid is a unique 32bit indentifier that describes this node.
      // It is used to save and retrieve nodes of this type from the binary
      // file format.  If it is not unique, it will cause file IO problems.
      //
      static MTypeId id;
   
      // Input Attributes
      static MObject aWeight;
   
      static MObject aMorpheItem;
      static MObject aMorpheName;
      static MObject aMorpheGeometry;
      static MObject aMorphePoints;
      static MObject aMorpheComponents;
      static MObject aMorpheWeights;
};
// -----------------------------------------------------------------------------

#endif
