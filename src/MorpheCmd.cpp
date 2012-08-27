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
   MPlug             plugArrItem(fnDeformer.findPlug(MorpheNode::aMorpheItem));
   MPlug             plugItem = plugArrItem.elementByLogicalIndex(idxTarget);
   MPlug             plugName  = plugItem.child(MorpheNode::aMorpheName);

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
   MPlug             plugArrItem(fnDeformer.findPlug(MorpheNode::aMorpheItem));
   MPlug             plugItem = plugArrItem.elementByLogicalIndex(idxTarget);
   MPlug             plugWt   = plugItem.child(MorpheNode::aMorpheWeights);

   MFnIntArrayData   fnWt;
   MObject           oWt = fnWt.create(idxWeight);
   plugWt.setValue(oWt);
   
   // DEBUG: Delete in the future.
   //MFnIntArrayData   test(plugWt.asMObject());
   //MString tmpStr("Weight: ");
   //tmpStr = tmpStr + test[0];
   //MGlobal::displayInfo(tmpStr);
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
   MPlug             plugArrItem(fnDeformer.findPlug(MorpheNode::aMorpheItem));
   MPlug             plugItem = plugArrItem.elementByLogicalIndex(idx);
   MPlug             plugGeo  = plugItem.child(MorpheNode::aMorpheGeometry);

   modifier.connect(plugWorldMesh, plugGeo);
   modifier.doIt();
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
void* MorpheCmd::creator()
{
   return new MorpheCmd();
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

   // Query Mode

   // Edit Mode
   syntax.addFlag(kCreateMorphesFlag, kCreateMorphesFlagLong, MSyntax::kString);
   //syntax.makeFlagMultiUse(kCreateMorphesFlag);

   // Enable Query and Edit
   syntax.enableQuery(true);
   syntax.enableEdit(true);

	return syntax;
}
// -----------------------------------------------------------------------------


//
// Description:
//    This method parse arguments.
//
// Return Values:
//    MSyntax
//
MStatus MorpheCmd::parseArgs(const MArgList &args)
{
   MStatus status;

   MArgDatabase argData(syntax(), args, &status);

   // -createMorphes [objectList]
   if(argData.isFlagSet(kCreateMorphesFlag) && argData.isEdit())
   {
      // status = argData.getFlagArgument(kCreateMorphesFlag, 0, sCreateMorphes);
      unsigned int uCmsPos = 0;
      argData.getFlagArgumentPosition(kCreateMorphesFlag, 0, uCmsPos);
      unsigned int uCmsArgPos = uCmsPos + 1;
      sCreateMorphes = args.asStringArray(uCmsArgPos, &status);
      if(status != MS::kSuccess)
      {
         MGlobal::displayError("-cms/createMorphes is missing a list of objects argument");
         return status;
      }
      else
      {
         unsigned int i = 2;
         //MGlobal::displayInfo(kk.asString());
         MString tmpStr("Weight: ");
         tmpStr = tmpStr + sCreateMorphes.length();
         tmpStr = tmpStr + "     Position: " + uCmsPos;
         MGlobal::displayInfo(tmpStr);
      }
   }
   return MS::kSuccess;
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

   status = parseArgs(args);
   if(status != MS::kSuccess)
      return status;

   MArgDatabase argData(syntax(), args);

   // Query Mode
   if(argData.isQuery())
   {
      MGlobal::displayInfo("query mode");
   }

   // Edit Mode
   if(argData.isEdit())
   {
      MGlobal::displayInfo("edit mode");
   }

   // Create Mode
   if(!argData.isQuery() && !argData.isEdit())
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
