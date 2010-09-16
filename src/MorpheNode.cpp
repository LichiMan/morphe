// -----------------------------------------------------------------------------
// MorpheNode.cpp - C++ File
// -----------------------------------------------------------------------------


//
// Includes
//
#include "MorpheNode.h"
// -----------------------------------------------------------------------------


MTypeId MorpheNode::id(MORPHE_ID);
// -----------------------------------------------------------------------------


//
// Attributes
//
MObject MorpheNode::aWeight;
MObject MorpheNode::aInputTargetItem;
MObject MorpheNode::aInputTargetName;
MObject MorpheNode::aInputTargetWeights;
MObject MorpheNode::aInputTargetGeometry;
MObject MorpheNode::aInputTargetPoints;
MObject MorpheNode::aInputTargetComponents;
// -----------------------------------------------------------------------------


//
// Constructor
//
MorpheNode::MorpheNode() {}
// -----------------------------------------------------------------------------


//
// Destructor
//
MorpheNode::~MorpheNode() {}
// -----------------------------------------------------------------------------


//
// Description:
//    This method gets weights for each target item.
//
// Return Values:
//    MS::kSuccess
//    MS::kFailure
//
MStatus MorpheNode::GetWeights(MDataBlock &data, MFloatArray &weights)
{
   MArrayDataHandle hArrWeight = data.inputArrayValue(aWeight);
   unsigned int uWeightCount = hArrWeight.elementCount();
   if (uWeightCount == 0)
      return MS::kSuccess;

   unsigned int i;
   unsigned uWtIdx;
   for(i = 0; i < uWeightCount; i++)
   {
      uWtIdx = hArrWeight.elementIndex(); // Index Item
      MDataHandle hWt = hArrWeight.inputValue();

      if(weights.length() < (uWtIdx + 1))
         weights.setLength(uWtIdx + 1);
      weights[uWtIdx] = hWt.asFloat();

      hArrWeight.next();
   }
   return MS::kSuccess;
}
// -----------------------------------------------------------------------------


//
// Description:
//    This method gets the final deltas position for each vertex.
//
// Return Values:
//    MS::kSuccess
//    MS::kFailure
//
MStatus MorpheNode::GetTargetsDeltas(MDataBlock &data, MItGeometry &itGeo, float &fEnv, MFloatArray &weights, MPointArray &deltas)
{
   MStatus status;

   MPointArray targetPts;
   MPointArray deltasOrig;
   itGeo.allPositions(deltasOrig);

   MArrayDataHandle hArrInputTargetItem = data.inputArrayValue(aInputTargetItem, &status);
   if (status != MS::kSuccess)
      return MS::kSuccess;

   unsigned int targetArrayCount = hArrInputTargetItem.elementCount();
   if (targetArrayCount == 0)
      return MS::kSuccess;

   // Go through each target
   unsigned uItemIdx = 0;
   float wt;
   unsigned int i;
   unsigned int j;
   for(i = 0; i < targetArrayCount; i++)
   {
      uItemIdx = hArrInputTargetItem.elementIndex(); // Index Item

      MDataHandle hInputTargetItem = hArrInputTargetItem.inputValue(); // Get compound element Item
      MObject oInputTargetGeometry = hInputTargetItem.child(aInputTargetGeometry).asMesh();
      if(oInputTargetGeometry.isNull())
         return MS::kSuccess;

      MFnMesh fnInputTargetGeometry(oInputTargetGeometry);
      fnInputTargetGeometry.getPoints(targetPts);

      wt = weights[uItemIdx] * fEnv;

      for(j = 0; j < targetPts.length(); j++)
      {
         if(i == 0)
            deltas.set((targetPts[j] - deltasOrig[j]) * wt, j);
         else
            deltas.set(deltas[j] + (targetPts[j] - deltasOrig[j]) * wt, j);
      }
      
      targetPts.clear();
      hArrInputTargetItem.next();
   }

   return MS::kSuccess;
}
// -----------------------------------------------------------------------------


//
// Description:
//    This method performs the deformation algorithm. A status code of
//      MS::kSuccess should be returned unless there was a problem during
//      the deformation, such as insufficient memory or required input
//      data is missing or invalid.
//
// Return Values:
//    MS::kSuccess
//    MS::kFailure
//
MStatus MorpheNode::deform(MDataBlock &data, MItGeometry &itGeo, const MMatrix &localToWorldMatrix, unsigned int mIndex)
{
   MStatus status;

   // Check Node State
   MDataHandle hNodeState = data.inputValue(state, &status);
   int nNodeState = hNodeState.asShort() ;
   if (nNodeState == 1)
      return MS::kSuccess;

   // Envelope data from the base class
   // The envelope is simply a scale factor
   MDataHandle hEnvelope = data.inputValue(envelope, &status);
   float fEnv = hEnvelope.asFloat();
   if(fEnv <= 0.0) // If off... done!
      return MS::kSuccess;

   // Get Weights
   MFloatArray targetWts;
   GetWeights(data, targetWts);

   // Get Targets
   MPointArray deltas;
   deltas.setLength(itGeo.count());
   GetTargetsDeltas(data, itGeo, fEnv, targetWts, deltas);

   // Iterate through each point in the geometry
   MPoint   ptOrig;
   MPoint   ptDef;
   float    wt = 0.0f;

   for(itGeo.reset(); !itGeo.isDone(); itGeo.next())
   {
      // Get weight values for this point
      wt = weightValue(data, mIndex, itGeo.index());
      if(wt <= 0.0) // If this pt isn't affected at all, just ignore it.
         continue;

      ptOrig = itGeo.position();

      ptDef = ptOrig + (deltas[itGeo.index()] * wt);
      itGeo.setPosition(ptDef);
   }

   return MS::kSuccess;
}
// -----------------------------------------------------------------------------


//
// Description:
//    This method exists to give Maya a way to create new objects
//      of this type. 
//
// Return Value:
//    a new object of this type
//
void* MorpheNode::creator()
{
   return new MorpheNode();
}
// -----------------------------------------------------------------------------


//
// Description:
//    This method is called to create and initialize all of the attributes
//      and attribute dependencies for this node type.  This is only called 
//      once when the node type is registered with Maya.
//
// Return Values:
//    MS::kSuccess
//    MS::kFailure
//
MStatus MorpheNode::initialize()
{
   MFnNumericAttribute  nAttr;
   MFnTypedAttribute    tAttr;
   MFnCompoundAttribute cAttr;

   aWeight = nAttr.create("weight", "wt", MFnNumericData::kFloat, 0.0);
   nAttr.setArray(true);
   nAttr.setUsesArrayDataBuilder(true);
   nAttr.setStorable(true);
   nAttr.setConnectable(true);
   nAttr.setKeyable(true);
   nAttr.setSoftMin(0.0);
   nAttr.setSoftMax(1.0);

   aInputTargetName = tAttr.create("inputTargetName", "itn", MFnData::kString);
   tAttr.setStorable(true);
   tAttr.setConnectable(false);

   aInputTargetGeometry = tAttr.create("inputTargetGeometry", "itg", MFnData::kMesh);
   tAttr.setStorable(false);
   tAttr.setConnectable(true);

   aInputTargetPoints = tAttr.create("inputTargetPoints", "itp", MFnData::kPointArray);
   tAttr.setStorable(true);
   tAttr.setConnectable(true);

   aInputTargetComponents = tAttr.create("inputTargetComponents", "itc", MFnData::kComponentList);
   tAttr.setStorable(true);
   tAttr.setConnectable(true);

   aInputTargetWeights = tAttr.create("inputTargetWeights", "itw", MFnData::kIntArray);
   nAttr.setStorable(true);
   nAttr.setConnectable(false);

   aInputTargetItem = cAttr.create("inputTargetItem", "iti");
   cAttr.setArray(true);
   cAttr.setUsesArrayDataBuilder(true);
   cAttr.setStorable(true);
   cAttr.setConnectable(true);
   cAttr.addChild(aInputTargetName);
   cAttr.addChild(aInputTargetGeometry);
   cAttr.addChild(aInputTargetPoints);
   cAttr.addChild(aInputTargetComponents);
   cAttr.addChild(aInputTargetWeights);

   addAttribute(aWeight);
   addAttribute(aInputTargetItem);

   attributeAffects(aWeight, outputGeom);
   attributeAffects(aInputTargetItem, outputGeom);
   attributeAffects(aInputTargetGeometry, outputGeom);
   attributeAffects(aInputTargetPoints, outputGeom);
   attributeAffects(aInputTargetComponents, outputGeom);
   attributeAffects(aInputTargetWeights, outputGeom);

   // Make the deformer weights paintable
   MGlobal::executeCommand( "makePaintable -attrType multiFloat -sm deformer morphe weights;" );

   return MS::kSuccess;
}
// -----------------------------------------------------------------------------

