// -----------------------------------------------------------------------------
// MorpheCmd.cpp - C++ File
// -----------------------------------------------------------------------------


//
// Includes
//
#include "MorpheCmd.h"
#include "MorpheNode.h"
// -----------------------------------------------------------------------------


//
// Flags
//
const char *editFlag = "-e", *editLongFlag = "-edit";
const char *queryFlag = "-q", *queryLongFlag = "-query";
// -----------------------------------------------------------------------------


//
// Description:
//    This method add weight attributes to the node deformer.
//
void MorpheCmd::AddWeight(MObject &obj, MObject &objDeformer, unsigned int &idx)
{
   MFnDependencyNode fnObj(obj);
   MFnDependencyNode fnDeformer(objDeformer);
   MPlug             plugArrWeight(fnDeformer.findPlug(MorpheNode::aWeight));
   MPlug             plugWeight = plugArrWeight.elementByLogicalIndex(idx);

   plugWeight.setValue(0.0);
   fnDeformer.setAlias(fnObj.name(), plugWeight.partialName(false, false, false, false, false, true), plugWeight);
}
// -----------------------------------------------------------------------------


//
// Description:
//    This method sets the name for the target.
//
void MorpheCmd::SetTargetName(MObject &objDeformer, unsigned int &idxTarget, MString &name)
{
   MFnDependencyNode fnDeformer(objDeformer);
   MPlug             plugArrItem(fnDeformer.findPlug(MorpheNode::aInputTargetItem));
   MPlug             plugItem = plugArrItem.elementByLogicalIndex(idxTarget);
   MPlug             plugName  = plugItem.child(MorpheNode::aInputTargetName);

   plugName.setValue(name);
   
   // DEBUG: Delete in the future.
   MGlobal::displayInfo(name);
   // DEBUG: Delete in the future.
}
// -----------------------------------------------------------------------------


//
// Description:
//    This method sets the IDs of weights involved for a specific target.
//
void MorpheCmd::SetTargetWeight(MObject &objDeformer, unsigned int &idxTarget, MIntArray &idxWeight)
{
   MFnDependencyNode fnDeformer(objDeformer);
   MPlug             plugArrItem(fnDeformer.findPlug(MorpheNode::aInputTargetItem));
   MPlug             plugItem = plugArrItem.elementByLogicalIndex(idxTarget);
   MPlug             plugWt   = plugItem.child(MorpheNode::aInputTargetWeights);

   MFnIntArrayData   fnWt;
   MObject           oWt = fnWt.create(idxWeight);
   plugWt.setValue(oWt);
   
   // DEBUG: Delete in the future.
   MFnIntArrayData   test(plugWt.asMObject());
   MString tmpStr("Weight: ");
   tmpStr = tmpStr + test[0];
   MGlobal::displayInfo(tmpStr);
   // DEBUG: Delete in the future.
}
// -----------------------------------------------------------------------------


//
// Description:
//    This method connect a target mesh to the deformer.
//
void MorpheCmd::ConnectInputs(MObject &obj, MObject &objDeformer, unsigned int &idx)
{
   MDGModifier       modifier;
   MFnDagNode        dagObj(obj);
   MDagPath          shapeObj; dagObj.getPath(shapeObj); shapeObj.extendToShape();
   MFnDependencyNode fnObj(shapeObj.node());
   MPlug             plugArrWorldMesh = fnObj.findPlug("worldMesh");
   MPlug             plugWorldMesh    = plugArrWorldMesh.elementByLogicalIndex(0); // First instance is requested for mesh shapes.

   MFnDependencyNode fnDeformer(objDeformer);
   MPlug             plugArrItem(fnDeformer.findPlug(MorpheNode::aInputTargetItem));
   MPlug             plugItem = plugArrItem.elementByLogicalIndex(idx);
   MPlug             plugGeo  = plugItem.child(MorpheNode::aInputTargetGeometry);

   modifier.connect(plugWorldMesh, plugGeo);
   modifier.doIt();
}
// -----------------------------------------------------------------------------


//
// Description:
//    This method defines command flags.
//
// Return Values:
//    MSyntax
//
MSyntax MorpheCmd::newSyntax()
{
   MSyntax syntax;

   syntax.addFlag(editFlag, editLongFlag, MSyntax::kNoArg);
   syntax.addFlag(queryFlag, queryLongFlag, MSyntax::kNoArg);

	return syntax;
}
// -----------------------------------------------------------------------------


//
// Description:
//    This is the main method command.
//
// Return Values:
//    MS::kSuccess
//    MS::kFailure
//
MStatus MorpheCmd::doIt (const MArgList &args)
{
   MStatus status;

   MArgDatabase argData(syntax(), args);

   if(argData.isFlagSet(editFlag))
   {
      MGlobal::displayInfo("edit mode");
   }

   if(argData.isFlagSet(queryFlag))
   {
      MGlobal::displayInfo("query mode");
   }

   if(!argData.isFlagSet(editFlag) && !argData.isFlagSet(queryFlag))
   {
      MObject           obj;
      MFnDependencyNode fnObj;
      MObject           base;
      MDagPath          dpBase;
      
      MSelectionList    list;
      if(MStatus::kFailure == MGlobal::getActiveSelectionList(list))
      {
         MGlobal::displayError("MGlobal::getActiveSelectionList");
         return MStatus::kFailure;
      }
      
      if(list.length() == 0)
      {
         MGlobal::displayError("Select at least one object.");
         return MStatus::kFailure;
      }

      // Create the deformer to the last object selected 
      list.getDagPath(list.length()-1, dpBase);
      MStringArray strDeformer;
      MGlobal::executeCommand("deformer -type \"morphe\" " + dpBase.partialPathName(), strDeformer, false, true);

      // Get the deformer MObject node and its MPlug weight
      MSelectionList    listDeformer;
      MObject           objDeformer;
      listDeformer.add(strDeformer[0]);
      listDeformer.getDependNode(0, objDeformer);

      // We need to iterate for all objects but base object
      list.remove(list.length()-1);
      unsigned int      idx = 0;
      MItSelectionList  iterList(list, MFn::kTransform, &status);
      if(MStatus::kFailure == status)
         return MStatus::kFailure;

      for(; !iterList.isDone(); iterList.next())
      {
         iterList.getDependNode(obj);

         // Create weights attributes for each object selected
         AddWeight(obj, objDeformer, idx);

         // Connect each object selected to deformer
         ConnectInputs(obj, objDeformer, idx);

         // Assign names
         fnObj.setObject(obj);
         SetTargetName(objDeformer, idx, fnObj.name());

         // Assign the corresponding weight
         MIntArray uArrWt;
         uArrWt.append(idx);
         SetTargetWeight(objDeformer, idx, uArrWt);

         idx++;
      }

      // Return morphe name
      clearResult();
      setResult(strDeformer[0]);
   }

   return MS::kSuccess;
}
// -----------------------------------------------------------------------------
