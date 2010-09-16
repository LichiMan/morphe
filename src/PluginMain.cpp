// -----------------------------------------------------------------------------
// PluginMain.cpp - C++ File
// -----------------------------------------------------------------------------


//
// Includes
//
#include "MorpheNode.h"
#include "MorpheCmd.h"
#include <maya/MFnPlugin.h>
// -----------------------------------------------------------------------------


//
// Description:
//    This method is called when the plug-in is loaded into Maya.  It 
//      registers all of the services that this plug-in provides with 
//      Maya.
//
// Arguments:
//    obj - a handle to the plug-in object (use MFnPlugin to access it)
//
MStatus initializePlugin( MObject obj )
{ 
   MStatus   status;
   MFnPlugin plugin( obj, "Frank Barton", "0.01", "Any");

   status = plugin.registerNode("morphe", MorpheNode::id, MorpheNode::creator, MorpheNode::initialize, MPxNode::kDeformerNode);
   status = plugin.registerCommand( "morphe", MorpheCmd::creator, MorpheCmd::newSyntax );

   return status;
}
// -----------------------------------------------------------------------------


//
// Description:
//    This method is called when the plug-in is unloaded from Maya. It 
//      deregisters all of the services that it was providing.
//
// Arguments:
//    obj - a handle to the plug-in object (use MFnPlugin to access it)
//
MStatus uninitializePlugin( MObject obj)
{
   MStatus   status;
   MFnPlugin plugin( obj );

   status = plugin.deregisterNode( MorpheNode::id );
   status = plugin.deregisterCommand( "morphe" );

   return status;
}
// -----------------------------------------------------------------------------