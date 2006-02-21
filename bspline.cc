/***********************************************************
 *
 * File:			bspline.cc
 * Author:			Jeromy Carriere
 * Created:			February 1996
 *
 * Modification history:
 *
 ***********************************************************/

#include <math.h>
#include <stdlib.h>
#include "bspline.h"
#include "figure.h"
#include "pspresentation.h"
#include "scenario.h"

#define FABS(a)	((a)>(0.0))?(a):(-1.0*a)

int BSpline::highlight_colour = RED; // default highlight colour
int BSpline::multiple_traversal_colour = ORANGE;

extern float sampling_rate;

BSpline::BSpline( bool create )
{
   if( create )
      figures = new Cltn<Figure *>;
   else
      figures = NULL;
    
   fend_x = UNDEFINED_POINT;
   fend_y = UNDEFINED_POINT;
   changed = TRUE;
   highlight_id = 0;
}

BSpline::~BSpline()
{
   while( !lControl_points.is_empty() )
      delete lControl_points.Detach();

   delete figures;
   figures = NULL;
   fend_x = 0;
   fend_y = 0;  
}

void BSpline::Interpolate()
{
   float rgrgflData[500][3], fX, fY;
   static float *rgflKnots, (*rgrgflControl)[3];
   int i = 0, iNum = figures->Size();
   static int array_size = 0;
   
   while( !lControl_points.is_empty() )
      delete lControl_points.Detach();

   knot_vector.clear();

   for( figures->First(); !figures->IsDone(); figures->Next() ) {
      figures->CurrentItem()->GetPosition( fX, fY );
      rgrgflData[i+1][0] = fX;
      rgrgflData[i+1][1] = fY;
      rgrgflData[i+1][2] = 0; // 3rd dimension initialized to 0
      i++;
   }

   rgrgflData[0][0] = rgrgflData[1][0];
   rgrgflData[0][1] = rgrgflData[1][1];
   rgrgflData[0][2] = rgrgflData[1][2];
  
   rgrgflData[iNum+1][0] = rgrgflData[iNum][0];
   rgrgflData[iNum+1][1] = rgrgflData[iNum][1];
   rgrgflData[iNum+1][2] = rgrgflData[iNum][2];
   iNum+=2;

   if( iNum > array_size ) {
      if( array_size != 0 ) {
	 delete [] rgflKnots;
	 delete [] rgrgflControl;
      }
      
      rgflKnots = new float[iNum+4];  
      rgrgflControl = new float[iNum+2][3];
      array_size = iNum;
   }
  
   for( i = 0; i < iNum+4; i++ )
      rgflKnots[i] = i;

   Interpolate_bspline( rgrgflData, rgrgflControl,
			rgflKnots, iNum );
  
   for( i = 0; i < iNum; i++ ) 
      AddControlPoint( rgrgflControl[i][0], rgrgflControl[i][1] );
  
   changed = FALSE;
}

void BSpline::AddControlPoint( float fNew_x, float fNew_y )
{
   Point* ppNew = new Point( fNew_x, fNew_y );
   lControl_points.attachr( ppNew );
   if( knot_vector.length() == 0 ) {
      knot_vector.attachr( (double)0.0 );
      knot_vector.attachr( (double)1.0 );
      knot_vector.attachr( (double)2.0 );
   } else {
      knot_vector.attachr( knot_vector.getr() + (double)1.0 );
   }
}

int BSpline::FindControlPoint( float fCheck_x, float fCheck_y,
			       float fTolerance )
{
   int i, iIndex=-1;
   float fX, fY, fMin_dist=100000.0, fDist;

   if( fTolerance == 0.0 )
      fTolerance = 100000.0;

   for( i=0; i < lControl_points.length(); i++ ) {
      lControl_points.getnth( i )->GetPosition( fX, fY );
      fDist = (fX-fCheck_x)*(fX-fCheck_x) + (fY-fCheck_y)*(fY-fCheck_y);
    
      if( (fDist < fMin_dist) && (fDist < fTolerance) ) {
	 fMin_dist = fDist;
	 iIndex = i;
      }
   }
   return( iIndex );
}

void BSpline::DrawPath( Presentation *ppr )
{
   BSpline* pbsTemp, *pbsOld;
   float fX, fY, fLast_x=-1.0, fLast_y=-1.0, ex, ey;
   float fCp_x[4], fCp_y[4];
   
   double fKnot;
   int i, j, k, iMultiplicity, num_points = 0;
   static float (*path_points)[2] = NULL, (*old_path_points)[2];
   static int array_size = 0;
   Point *p;
   
   if( knot_vector.length() == 0 )
      return;

   if( highlight_id != 0 ) {
      if( ScenarioList::ScenarioTracingId() == abs(highlight_id) ) { // highlighting is still active
	 if( highlight_id > 0 ) // single traversal of segment
	    ppr->SetFgColour( highlight_colour );
	 else // multiple traversals of segment
	    ppr->SetFgColour( multiple_traversal_colour );
      }
      else
	 highlight_id = 0; // highlighting has been changed, reset
   }

   if( ppr->CanDrawCurves() ) {
      
      Cltn<Point *> lppPoints;
      int iSegments = GetBezierControlPoints( lppPoints );

      p = lppPoints.GetFirst();
      p->GetPosition( fX, fY );
      ppr->StartCurve( fX, fY );
	 
      for( i = 0; i < iSegments; i++ ) {
	 for( j = 0; j < 4; j++ ) {
	    p = lppPoints.RemoveFirstElement();
	    p->GetPosition( fCp_x[j], fCp_y[j] );
	    delete p;
	 }
	 ppr->DrawCurve( fCp_x, fCp_y );
      }

      ppr->EndCurve();
      if( highlight_id != 0 )
	 ppr->SetFgColour( BLACK );
      return;
   }

   float fStart = fKnot = knot_vector.getnth( DEGREE-1 ); 
   float fEnd = knot_vector.getnth( knot_vector.length()-DEGREE ); 

   for( fKnot = fStart; fKnot <= fEnd; fKnot += sampling_rate ) {
      pbsTemp = this;

      iMultiplicity = GetKnotMultiplicity( fKnot );

      for( i=0; i < (DEGREE-iMultiplicity); i++ ) {
	 pbsOld = pbsTemp;
	 pbsTemp = pbsTemp->InsertKnot( fKnot );

	 if( pbsOld != this )
	    delete pbsOld;

      }

      int max = pbsTemp->lControl_points.length();
      for( i=0; i < max; i++ ) {
	 if( (pbsTemp->knot_vector.getnth(i) == pbsTemp->knot_vector.getnth(i+1))
	     && (pbsTemp->knot_vector.getnth(i) == pbsTemp->knot_vector.getnth(i+2) ) ) {

	    pbsTemp->lControl_points.getnth( i )->GetPosition( fX, fY );

	    if( num_points >= array_size ) { // grow array
	       array_size += 1024;
	       old_path_points = path_points;
	       path_points = new float[array_size][2];
	       for( k = 0; k < num_points; k++ ) {
		  path_points[k][0] = old_path_points[k][0];
		  path_points[k][1] = old_path_points[k][1];
	       }
	       if( old_path_points ) delete [] old_path_points;
	    }

	    path_points[num_points][0] = fX;
	    path_points[num_points][1] = fY;
	    num_points++;
	    
	    fLast_x = fX;
	    fLast_y = fY;
	 }
      }
    
      fend_x = fLast_x;
      fend_y = fLast_y;
    
      // CURVE
    
      if( pbsTemp != this ) {
	 delete pbsTemp;
      }

   }

   // bug fix for short splines
   figures->GetLast()->GetPosition( ex, ey );
   
   if( ex != fend_x || ey != fend_y ) {
      path_points[num_points][0] = ex;
      path_points[num_points][1] = ey;
      num_points++;
   }      
   
   //cout << "The number of spline line segments is " << num_points << endl << flush;

   ppr->DrawConnectedLines( path_points, num_points );

   if( highlight_id != 0 )
      ppr->SetFgColour( BLACK );
}

BSpline* BSpline::InsertKnot( double fKnot )
{
   int i, iFirst_addition=0, kl = knot_vector.length();
   BSpline* pbsNew = new BSpline;
   float fRatio, fX1, fY1, fX2, fY2;
   static float *fKnots;
   static int array_size = 0;

   if( kl > array_size ) {
      if( array_size != 0 )
	 delete [] fKnots;
      fKnots = new float[kl];
      array_size = kl;
   }
   
   i=0;
   for( knot_vector.First(); !knot_vector.IsDone(); knot_vector.Next() ) {
      fKnots[i] = knot_vector.CurrentItem();
      i++;
   }

   i=0;
   for( lControl_points.First(); i < lControl_points.length()-1; lControl_points.Next() ) {
      if( (fKnot >= fKnots[i])
	  && (fKnot <= fKnots[i+DEGREE] ) ) {

	 if( (fKnots[i+DEGREE] - fKnots[i]) == 0 )
	    fRatio = 1;
	 else
	    fRatio = (fKnot-fKnots[i])/(fKnots[i+DEGREE]-fKnots[i]);

	 if( iFirst_addition == 0 ) {
	    pbsNew->lControl_points.attachr( new Point( *lControl_points.CurrentItem() ) );
	    iFirst_addition = 1;
	 }
      
	 lControl_points.CurrentItem()->GetPosition( fX1, fY1 );
	 lControl_points.NextItem()->GetPosition( fX2, fY2 );

	 pbsNew->lControl_points.attachr( new Point( (1-fRatio)*fX1 + fRatio*fX2,
						     (1-fRatio)*fY1 + fRatio*fY2 ));

      } else {
	 pbsNew->lControl_points.attachr( new Point( *lControl_points.CurrentItem() ) );
      }

      i++;
   }

   pbsNew->lControl_points.attachr( new Point( *lControl_points.getr() ) );
  
   int iKnot_not_inserted = 1;

   for( i=0; i < knot_vector.length()-1; i++ ) {
      pbsNew->knot_vector.attachr( knot_vector.getnth( i ) );

      if( (knot_vector.getnth( i ) <= fKnot)
	  && (knot_vector.getnth( i+1 ) >= fKnot) 
	  && iKnot_not_inserted ) {
	 pbsNew->knot_vector.attachr( fKnot );
	 iKnot_not_inserted = 0;
      }
   }

   pbsNew->knot_vector.attachr( knot_vector.getr() );
   return( pbsNew );
}

int BSpline::GetBezierControlPoints( Cltn<Point *> &lppPoints ) {
   int i;
   BSpline *pbsTemp,  *pbsOld;
   float fX, fY;
   float fX1, fY1;

   // for each segment

   for( i=0; i < knot_vector.length()-5; i++ ) {

      // insert the first knot in the segment
      pbsTemp = InsertKnot( knot_vector.getnth( i+2 ) );

      // retrieve the appropriate control point
      pbsTemp->lControl_points.getnth( i+2 )->GetPosition( fX1, fY1 );
      pbsOld = pbsTemp;

      // insert the knot again
      pbsTemp = pbsTemp->InsertKnot( knot_vector.getnth( i+2 ) );
      delete pbsOld;

      // get the next control point
      pbsTemp->lControl_points.getnth( i+2 )->GetPosition( fX, fY );

      lppPoints.attachr( new Point( fX, fY ) );
      lppPoints.attachr( new Point( fX1, fY1 ) );

      delete pbsTemp;

      // insert the next knot in the segment
    
      pbsTemp = InsertKnot( knot_vector.getnth( i+3 ) );
      pbsTemp->lControl_points.getnth( i+2 )->GetPosition( fX, fY );

      pbsOld = pbsTemp;
      pbsTemp = pbsTemp->InsertKnot( knot_vector.getnth( i+3 ) );
      delete pbsOld;

      lppPoints.attachr( new Point( fX, fY ) );
      pbsTemp->lControl_points.getnth( i+3 )->GetPosition( fX, fY );
      lppPoints.attachr( new Point( fX, fY ) );

      delete pbsTemp;
   }

   // return the number of segments
   return( knot_vector.length()-5 );
}

void BSpline::GetLastControlPoints( Point **pppSecond_last,
				    Point **pppLast )
{
   if( lControl_points.length() < 2 )
      this->Interpolate();
   
   *pppSecond_last = lControl_points[lControl_points.length()-2];
   *pppLast = lControl_points[lControl_points.length()-1];
}

int BSpline::GetKnotMultiplicity( double fKnot )
{
   int iCount = 0;

   for( knot_vector.First(); !knot_vector.IsDone(); knot_vector.Next() ) {
      if( (knot_vector.CurrentItem()-fKnot) == 0.0 )
	 iCount++;
   }

   return( iCount );
}

void BSpline::Tridiag( float rgflA[], float rgflB[], float rgflC[], float rgflD[], float rgflX[], int n )
{
   int j;
   float flB;
   static float *pflG = NULL, *pflD = NULL;
   static int array_size = 0;

   if( n > array_size ) {
      if( array_size != 0 ) {
	 delete [] pflG;
	 delete [] pflD;
      }
      pflG = new float[n];
      pflD = new float[n];
      array_size = n;
   }

   pflG[0] = rgflC[0]/rgflB[0];
   pflD[0] = rgflD[0]/rgflB[0];

   for( j = 1; j < n; j++ ) {

      flB = rgflB[j]-rgflA[j]*pflG[j-1];
      pflG[j] = rgflC[j]/flB;
      pflD[j] = (rgflD[j]-rgflA[j]*pflD[j-1])/flB;
   }

   rgflX[n-1] = pflD[n-1];
   for( j = n-2; j >= 0; j-- )
      rgflX[j] = pflD[j]-pflG[j]*rgflX[j+1];

}

void BSpline::Interpolate_bspline( float rgrgflData[][3], float rgrgflControl[][3], float rgflKnots[], int iNum )
{
   int i;
   static float *pflA, *pflB, *pflC, *rgflX_in, *rgflY_in, *rgflZ_in, *rgflX, *rgflY, *rgflZ;
   static int array_size = 0;
   float rgflT[5];

   if( iNum > array_size ) {
      if( array_size != 0 ) {

	 delete [] pflA;
	 delete [] pflB;
	 delete [] pflC;
  
	 delete [] rgflX_in;
	 delete [] rgflY_in;
	 delete [] rgflZ_in;
	 delete [] rgflX;
	 delete [] rgflY;
	 delete [] rgflZ;
      }
      pflA = new float[iNum];
      pflB = new float[iNum];
      pflC = new float[iNum];

      rgflX_in = new float[iNum];
      rgflY_in = new float[iNum];
      rgflZ_in = new float[iNum];
      rgflX = new float[iNum];
      rgflY = new float[iNum];
      rgflZ = new float[iNum];
      array_size = iNum;
   }

   pflA[0] = 0; pflB[0] = 1.0; pflC[0] = 0;
   pflA[iNum-1] = 0; pflB[iNum-1] = 1.0; pflC[iNum-1] = 0;

   for( i=1; i < iNum-1; i++ ) {
      int j;

      for( j=0; j <= 4; j++ ) {
	 rgflT[j] = rgflKnots[i+j+1];
      }

      pflA[i] = (rgflT[3]-rgflT[2])*(rgflT[3]-rgflT[2])/(rgflT[3]-rgflT[0]);
      pflB[i] = (rgflT[2]-rgflT[0])*(rgflT[3]-rgflT[2])/(rgflT[3]-rgflT[0])
	 + (rgflT[4]-rgflT[2])*(rgflT[2]-rgflT[1])/(rgflT[4]-rgflT[1]);
      pflC[i] = (rgflT[2]-rgflT[1])*(rgflT[2]-rgflT[1])/(rgflT[4]-rgflT[1]);
    
      pflA[i] /= (rgflT[3]-rgflT[1]);
      pflB[i] /= (rgflT[3]-rgflT[1]);
      pflC[i] /= (rgflT[3]-rgflT[1]);

   }


   for( i=0; i < iNum; i++ ) {
      rgflX_in[i] = rgrgflData[i][0];
      rgflY_in[i] = rgrgflData[i][1];
      rgflZ_in[i] = rgrgflData[i][2];
   }

   Tridiag( pflA, pflB, pflC, rgflX_in,
	    rgflX, iNum );
   Tridiag( pflA, pflB, pflC, rgflY_in,
	    rgflY, iNum );
   Tridiag( pflA, pflB, pflC, rgflZ_in,
	    rgflZ, iNum );

   for( i=0; i < iNum; i++ ) {
      rgrgflControl[i][0] = rgflX[i];
      rgrgflControl[i][1] = rgflY[i];
      rgrgflControl[i][2] = rgflZ[i];
   }
}

float BSpline::GetTangentAngle( Figure *ref_figure )
{
   Figure *prev_figure = NULL, *next_figure;
   float xp, yp, xn, yn, theta;
   
   for( figures->First(); !figures->IsDone(); figures->Next() ) {
      if( figures->CurrentItem() == ref_figure )
	 break;
      prev_figure = figures->CurrentItem();
   }

   next_figure = figures->NextItem();
   
   prev_figure->GetPosition( xp, yp );
   next_figure->GetPosition( xn, yn );
   
   if( (xn - xp) == 0.0 )
      theta = yp > yn ? PI/2.0 : -PI/2.0;
   else
      theta = (xn - xp) == 0.0 ? PI/2.0 : -1.0*atan2( yn-yp, xn-xp );
   
   return( theta );
}
