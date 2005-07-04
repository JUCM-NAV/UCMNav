/***********************************************************
 *
 * File:			node.cc
 * Author:			Andrew Miga
 * Created:			June 1996
 *
 * Modification history:
 *
 ***********************************************************/

#include "node.h"
#include "hypergraph.h"
#include "defines.h"

int Node::number_nodes = 0;

Node::Node( nodeColour newColour )
{ 
   TransformationManager *trans_manager = TransformationManager::Instance();
   visited = FALSE;
   colour = newColour;
   next = NULL;
   previous = NULL;
   trans_manager->CurrentGraph()->RegisterNode( this ); 
   node_number = number_nodes++;
   //visited = FALSE;
}
