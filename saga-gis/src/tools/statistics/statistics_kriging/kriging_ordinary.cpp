
///////////////////////////////////////////////////////////
//                                                       //
//                         SAGA                          //
//                                                       //
//      System for Automated Geoscientific Analyses      //
//                                                       //
//                     Tool Library                      //
//                 Geostatistics_Kriging                 //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//                 Kriging_Ordinary.cpp                  //
//                                                       //
//                 Copyright (C) 2008 by                 //
//                      Olaf Conrad                      //
//                                                       //
//-------------------------------------------------------//
//                                                       //
// This file is part of 'SAGA - System for Automated     //
// Geoscientific Analyses'. SAGA is free software; you   //
// can redistribute it and/or modify it under the terms  //
// of the GNU General Public License as published by the //
// Free Software Foundation, either version 2 of the     //
// License, or (at your option) any later version.       //
//                                                       //
// SAGA is distributed in the hope that it will be       //
// useful, but WITHOUT ANY WARRANTY; without even the    //
// implied warranty of MERCHANTABILITY or FITNESS FOR A  //
// PARTICULAR PURPOSE. See the GNU General Public        //
// License for more details.                             //
//                                                       //
// You should have received a copy of the GNU General    //
// Public License along with this program; if not, see   //
// <http://www.gnu.org/licenses/>.                       //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//    e-mail:     oconrad@saga-gis.org                   //
//                                                       //
//    contact:    Olaf Conrad                            //
//                Institute of Geography                 //
//                University of Goettingen               //
//                Goldschmidtstr. 5                      //
//                37077 Goettingen                       //
//                Germany                                //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#include "kriging_ordinary.h"


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CKriging_Ordinary::CKriging_Ordinary(void)
{
	//-----------------------------------------------------
	Set_Name		(_TL("Ordinary Kriging"));

	Set_Author		("O.Conrad (c) 2008");

	Set_Description	(_TW(
		"Ordinary Kriging for grid interpolation from irregular sample points."
	));

	//-----------------------------------------------------
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CKriging_Ordinary::Get_Weights(const CSG_Points_Z &Points, CSG_Matrix &W)
{
	int	n	= Points.Get_Count();

	if( n > 0 )
	{
		W.Create(n + 1, n + 1);

		for(int i=0; i<n; i++)
		{
			W[i][i]	= 0.0;				// diagonal...
			W[i][n]	= W[n][i]	= 1.0;	// edge...

			for(int j=i+1; j<n; j++)
			{
				W[i][j]	= W[j][i]	= Get_Weight(Points.Get_X(i), Points.Get_Y(i), Points.Get_X(j), Points.Get_Y(j));
			}
		}

		W[n][n]	= 0.0;

		return( W.Set_Inverse(!m_Search.Do_Use_All(), n + 1) );
	}

	return( false );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CKriging_Ordinary::Get_Value(const TSG_Point &p, double &z, double &v)
{
	//-----------------------------------------------------
	int				i, n;
	double			**W;
	CSG_Matrix		_W;
	CSG_Points_Z	_Data, *pData;

	if( m_Search.Do_Use_All() )	// global
	{
		pData	= &m_Data;
		W		= m_W.Get_Data();
	}
	else if( m_Search.Get_Points(p, _Data) && Get_Weights(_Data, _W) )	// local
	{
		pData	= &_Data;
		W		= _W.Get_Data();
	}
	else
	{
		return( false );
	}

	//-----------------------------------------------------
	if(	(n = pData->Get_Count()) > 0 )
	{
		CSG_Vector	G(n + 1);

		for(i=0; i<n; i++)
		{
			G[i]	= Get_Weight(p, pData->Get_Point(i));
		}

		G[n]	= 1.0;

		//-------------------------------------------------
		for(i=0, z=0.0, v=0.0; i<n; i++)
		{
			double	Lambda	= 0.0;

			for(int j=0; j<=n; j++)
			{
				Lambda	+= W[i][j] * G[j];
			}

			z	+= Lambda * pData->Get_Z(i);
			v	+= Lambda * G[i];
		}

		//-------------------------------------------------
		return( true );
	}

	return( false );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
