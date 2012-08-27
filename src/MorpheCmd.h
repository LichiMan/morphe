// -----------------------------------------------------------------------------
// MorpheCmd.h - C++ File
// -----------------------------------------------------------------------------

#ifndef MORPHE_CMD_H
#define MORPHE_CMD_H


//
// Includes
//
#include <maya/MArgDatabase.h>
#include <maya/MArgList.h>
#include <maya/MDagPath.h>
#include <maya/MDGModifier.h>
#include <maya/MFnDagNode.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnIntArrayData.h>
#include <maya/MFnStringData.h>
#include <maya/MGlobal.h>
#include <maya/MItSelectionList.h>
#include <maya/MObject.h>
#include <maya/MPlug.h>
#include <maya/MPxCommand.h>
#include <maya/MSelectionList.h>
#include <maya/MSyntax.h>
// -----------------------------------------------------------------------------


//
// MorpheCmd - Class Definition
//
class MorpheCmd : public MPxCommand
{
public:

   static  void      AddWeight(MObject &obj, MObject &objDeformer, unsigned int &idx);
   static  void      SetTargetName(MObject &objDeformer, unsigned int &idxTarget, MString &name);
   static  void      SetTargetWeight(MObject &objDeformer, unsigned int &idxTarget, MIntArray &idxWeight);
   static  void      ConnectInputs(MObject &obj, MObject &objDeformer, unsigned int &idx);
   virtual MStatus   doIt(const MArgList &args);
   static  MSyntax   newSyntax();
   static  void*     creator();

private:

   MStatus           parseArgs(const MArgList &args);

   MStringArray      sCreateMorphes;
};
// -----------------------------------------------------------------------------


//
// Syntax Flags
//
#define kCreateMorphesFlag        "-cms"
#define kCreateMorphesFlagLong    "-createMorphes"
// -----------------------------------------------------------------------------

#endif
