/***********************************************************
 *
 * File:			resp_mgr.cc
 * Author:			Andrew Miga
 * Created:			April 1999
 *
 * Modification history:
 *
 ***********************************************************/

#include "resp_mgr.h"
#include "map.h"
#include "xml_mgr.h"
#include "print_mgr.h"
#include "devices.h"

ResponsibilityManager * ResponsibilityManager::SoleResponsibilityManager = NULL;

ResponsibilityManager * ResponsibilityManager::Instance()
{
   if( SoleResponsibilityManager == NULL )
      SoleResponsibilityManager = new ResponsibilityManager();

   return( SoleResponsibilityManager );
}

void ResponsibilityManager::AddResponsibility( Responsibility *new_resp )
{
   Responsibility *cr; // add new responsibility to list in alphabetical order

   if( responsibilities.Includes( new_resp ) ) // check if already in list
      return;

   if( responsibilities.Size() == 0 )
      responsibilities.Add( new_resp );
   else {
      for( responsibilities.First(); !responsibilities.IsDone(); responsibilities.Next() ) {
	 cr = responsibilities.CurrentItem();
	 if( strcmp( new_resp->Name(), cr->Name() ) < 0 ) {
	    responsibilities.AddBefore( new_resp, cr );
	    return;
	 }
      }
      responsibilities.Add( new_resp ); // add new responsibility at end of list  
   }
}

void ResponsibilityManager::ClearResponsibilityList()
{
   while( !responsibilities.is_empty() )
      delete responsibilities.Detach();
}

void ResponsibilityManager::CleanList()
{
   Responsibility *cr;
   
   for( responsibilities.First(); !responsibilities.IsDone(); responsibilities.Next() ) {
      cr = responsibilities.CurrentItem();
      if( cr->IsNotReferenced() ) {
	 responsibilities.RemoveCurrentItem();
	 delete cr;
      }
   }
}

Responsibility * ResponsibilityManager::FindResponsibility( int parent_id )
{
   if( XmlManager::ImportSubstitution() ) { // search list of mappings from replaced responsibilities to existing responsibilities
      for( import_mappings.First(); !import_mappings.IsDone(); import_mappings.Next() ) {
	 if( import_mappings.CurrentItem().replaced_identifier == parent_id ) {
	    for( responsibilities.First(); !responsibilities.IsDone(); responsibilities.Next() ) {
	       if( responsibilities.CurrentItem()->ResponsibilityNumber() == import_mappings.CurrentItem().existing_identifier )
		  return( responsibilities.CurrentItem() );
	    }
	 }
      }
   }

   // search list of existing responsibilities to match up identifiers
   for( responsibilities.First(); !responsibilities.IsDone(); responsibilities.Next() ) {
      if( responsibilities.CurrentItem()->LoadIdentifier() == parent_id )
	 return( responsibilities.CurrentItem() );
   }

   AbortProgram( "Parent responsibility not found." );
   return( NULL ); // DA: Added August 2004   
}

void ResponsibilityManager::DeleteMappings()
{
   while( !import_mappings.is_empty() )
      import_mappings.Detach();
}

bool ResponsibilityManager::DuplicateName(const char *name, int &index )
{
   for( responsibilities.First(); !responsibilities.IsDone(); responsibilities.Next() ) {
      if( strequal( responsibilities.CurrentItem()->Name(), name ) ) {
	 index = responsibilities.CurrentItem()->ResponsibilityNumber();
	 return TRUE;
      }
   }

   return FALSE;
}

int ResponsibilityManager::MaximumLoadNumber()
{
   int max_load = 0;

   for( responsibilities.First(); !responsibilities.IsDone(); responsibilities.Next() ) {
      if( responsibilities.CurrentItem()->LoadIdentifier() > max_load )
	 max_load = responsibilities.CurrentItem()->LoadIdentifier();
   }

   return( max_load+1 );
}

bool ResponsibilityManager::UniqueResponsibilityName( const char *new_name, const char *old_name, char *name )
{
   char processed_name[32], unprocessed_name[32];
   
   unprocessed_name[31] = 0;
   strncpy( unprocessed_name, new_name, 31 );
   XmlManager::RemoveWhitespace( processed_name, unprocessed_name );
   strcpy( name, processed_name );

   if( strequal( processed_name, "" ) )
      return( FALSE );

   if( old_name ) {
      if( strequal( processed_name, old_name ) )
	 return( TRUE );
   }

   for( responsibilities.First(); !responsibilities.IsDone(); responsibilities.Next() ) {
      if( strequal( responsibilities.CurrentItem()->Name(), processed_name ) )
	 return( FALSE );
   }

   return( TRUE );
}

const char * ResponsibilityManager::VerifyUniqueResponsibilityName( const char *name )
{
   int extension = 1;
   static char new_name[32];
   char old_name[32];
   bool duplicate_name;

   strcpy( new_name, name );
   strcpy( old_name, name );
   
   forever {

      duplicate_name = FALSE;
      
      for( responsibilities.First(); !responsibilities.IsDone(); responsibilities.Next() ) {
	 if( strequal( responsibilities.CurrentItem()->Name(), new_name ) ) {
	    duplicate_name = TRUE;
	    break;
	 }
      }

      if( duplicate_name ) {
	 if( strlen( old_name ) > 29 )
	    old_name[29] = 0;
	 sprintf( new_name, "%s-%d", old_name, extension++ );
      }
      else
	 return( new_name );
   }
}

void ResponsibilityManager::DeactivateHighlighting()
{
   for( responsibilities.First(); !responsibilities.IsDone(); responsibilities.Next() )
      responsibilities.CurrentItem()->ResetHighlight();
}

void ResponsibilityManager::SaveResponsibilityList( FILE *fp, Cltn<Map *> *map_list )
{
   if( responsibilities.Size() == 0 ) return;

   CleanList();
   
   if( map_list ) {

      bool resp_exist = FALSE;
      ResetGenerate();
      
      for( map_list->First(); !map_list->IsDone(); map_list->Next() ) {
	 if( map_list->CurrentItem()->HasResponsibilities() )
	    resp_exist = TRUE;
      }
      
      if( !resp_exist ) return;
   }
   
   PrintNewXMLBlock( fp, "responsibility-specification" );

   if( !map_list ) {
      for( responsibilities.First(); !responsibilities.IsDone(); responsibilities.Next() )
	 responsibilities.CurrentItem()->SaveXML( fp );
   }
   else {
      for( responsibilities.First(); !responsibilities.IsDone(); responsibilities.Next() ) {
	 if( responsibilities.CurrentItem()->Visited() )
	    responsibilities.CurrentItem()->SaveXML( fp );
      }
   }

   PrintEndXMLBlock( fp, "responsibility-specification" );
   LinebreakXML( fp );
}

void ResponsibilityManager::SaveCSMResponsibilityList( FILE *fp)
{
	for( responsibilities.First(); !responsibilities.IsDone(); responsibilities.Next() )
	 responsibilities.CurrentItem()->SaveCSMXML( fp );

}

void ResponsibilityManager::ResetGenerate()
{
   for( responsibilities.First(); !responsibilities.IsDone(); responsibilities.Next() )
      responsibilities.CurrentItem()->ResetVisited();
}

void ResponsibilityManager::OutputDXL( std::ofstream &dxl_file ) {
	Responsibility *cr;
	DeviceDirectory *dd = DeviceDirectory::Instance();
	ServiceRequest *sr = NULL;

	for( responsibilities.First(); !responsibilities.IsDone(); responsibilities.Next() ) {
		cr = responsibilities.CurrentItem();
		Cltn<ServiceRequest *> *service_requests = cr->ServiceRequests();

		dxl_file << "responsibility( " << "\"r" << cr->ResponsibilityNumber() << "\", ";
		dxl_file << "\"" << cr->Name() << "\", ";
		if( PrintManager::TextNonempty( cr->Description() ) )
			dxl_file << "\"" << removeNewlineinString( cr->Description() )<< "\", ";
		else
			dxl_file << "\"" << "\", ";
		if( service_requests->Size() == 0 )
			dxl_file << "\"0";
		else {
			dxl_file << "\"";   
			for( service_requests->First(); !service_requests->IsDone(); service_requests->Next() ) {
				sr = service_requests->CurrentItem();
				if( ( sr->DeviceId() == PROCESSOR_DEVICE ) || ( dd->DeviceType( sr->DeviceId() ) == PROCESSOR ) )
					dxl_file << "pr:" << sr->Amount() << ";";
				else
					dxl_file << "d" <<sr->DeviceId()<<":"<< sr->Amount() << ";";     
			}
		}				             
		dxl_file << "\" )\n";  
	}
}
