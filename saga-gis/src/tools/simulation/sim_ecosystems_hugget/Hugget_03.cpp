
///////////////////////////////////////////////////////////
//                                                       //
//                         SAGA                          //
//                                                       //
//      System for Automated Geoscientific Analyses      //
//                                                       //
//                     Tool Library                      //
//                 sim_ecosystems_hugget                 //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//                    Hugget_03.cpp                      //
//                                                       //
//                 Copyright (C) 2003 by                 //
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
#include "Hugget_03.h"


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CHugget_03::CHugget_03(void)
{
	Set_Name		(_TL("03: Spatially Distributed Simulation of Soil Nitrogen Dynamics"));

	Set_Author		("O.Conrad (c) 2003");

	Set_Description	(_TW(
		"Spatially Distributed Simulation of Soil Nitrogen Dynamics. "
	));

	Add_Reference("Hugget, R.J.", "1993",
		"Modelling the Human Impact on Nature",
		"Oxford University Press."
	);

	//-----------------------------------------------------
	Parameters.Add_Grid(
		"", "DEM"		, _TL("Elevation"),
		_TL(""),
		PARAMETER_INPUT
	);

	Parameters.Add_Grid(
		"", "NSTORE"	, _TL("Soil Nitrogen"),
		_TL(""),
		PARAMETER_OUTPUT
	);

	//-----------------------------------------------------
	Parameters.Add_Double(
		"", "TIME_SPAN"	, _TL("Time Span [a]"),
		_TL(""),
		100., 0., true
	);

	Parameters.Add_Double(
		"", "TIME_STEP"	, _TL("Time Interval [a]"),
		_TL(""),
		0.1, 0., true
	);

	Parameters.Add_Bool(
		"", "UPDATE"	, _TL("Update View"),
		_TL(""),
		true
	);

	//-----------------------------------------------------
	Parameters.Add_Double(
		"", "NINIT"		, _TL("Initial Nitrogen Content [kg/ha]"),
		_TL(""),
		5000., 0., true
	);

	Parameters.Add_Double(
		"", "NRAIN"		, _TL("Nitrogen in Rainfall [kg/ha/a]"),
		_TL(""),
		16., 0., true
	);
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CHugget_03::On_Execute(void)
{
	bool		bUpdate;
	int			iStep, nSteps;
	double		sTime, dTime, N_Init, N_Rain;
	CSG_Grid	*pDEM, *pN, N_1, S[8];
	CSG_String	s;

	//-----------------------------------------------------
	sTime	= Parameters("TIME_SPAN")->asDouble();
	dTime	= Parameters("TIME_STEP")->asDouble();
	nSteps	= (int)(sTime / dTime);

	bUpdate	= Parameters("UPDATE"   )->asBool();

	N_Init	= Parameters("NINIT"    )->asDouble();
	N_Rain	= Parameters("NRAIN"    )->asDouble();

	pDEM	= Parameters("DEM"      )->asGrid();

	pN		= Parameters("NSTORE"   )->asGrid();
	pN->Assign(N_Init);

	DataObject_Set_Colors(pN, 11, SG_COLORS_YELLOW_GREEN);

	N_1.Create(pN, SG_DATATYPE_Float);

	Init_Slopes(pDEM, S);

	//-----------------------------------------------------
	for(iStep=0; iStep<=nSteps && Set_Progress(iStep, nSteps); iStep++)
	{
		Process_Set_Text(CSG_String::Format("%s [a]: %f (%f)", _TL("Time"), dTime * iStep, sTime));

		if( bUpdate )
		{
			DataObject_Update(pN, pN->Get_Min(), pN->Get_Max(), true);
		}

		//-------------------------------------------------
		Step(S, pN, &N_1, N_Rain, dTime);

		//-------------------------------------------------
		pN->Assign(&N_1);
	}


	//-----------------------------------------------------
	return( true );
}

//---------------------------------------------------------
bool CHugget_03::Init_Slopes(CSG_Grid *pDEM, CSG_Grid S[8])
{
	int		x, y, i, ix, iy;
	double	z;

	//-----------------------------------------------------
	for(i=0; i<8; i++)
	{
		S[i].Create(pDEM, SG_DATATYPE_Float);
	}

	//-----------------------------------------------------
	for(y=0; y<Get_NY() && Set_Progress(y); y++)
	{
		for(x=0; x<Get_NX(); x++)
		{
			if( pDEM->is_NoData(x, y) )
			{
				for(i=0; i<8; i++)
				{
					S[i].Set_NoData(x, y);
				}
			}
			else
			{
				z	= pDEM->asDouble(x, y);

				for(i=0; i<8; i++)
				{
					ix	= Get_xTo(i, x);
					iy	= Get_yTo(i, y);

					if( pDEM->is_InGrid(ix, iy) )
					{
						S[i].Set_Value(x, y, (pDEM->asDouble(ix, iy) - z) / Get_Length(i));
					}
					else
					{
						S[i].Set_Value(x, y, 0.);
					}
				}
			}
		}
	}

	//-----------------------------------------------------
	return( true );
}

//---------------------------------------------------------
bool CHugget_03::Step(CSG_Grid S[8], CSG_Grid *pN, CSG_Grid *pN_1, double N_Rain, double dTime)
{
	int		x, y, i;
	double	s, dN;

	//-----------------------------------------------------
	for(y=0; y<Get_NY() && Set_Progress(y); y++)
	{
		for(x=0; x<Get_NX(); x++)
		{
			if( !S[0].is_NoData(x, y) )
			{
				for(i=0, dN=0.; i<8; i++)
				{
					if( (s = S[i].asDouble(x, y)) != 0. )
					{
						if( s < 0. )
						{
							dN	+= s * pN->asDouble(x, y);
						}
						else
						{
							dN	+= s * pN->asDouble(Get_xTo(i, x), Get_yTo(i, y));
						}
					}
				}

				s	= pN->asDouble(x, y) + (dN + N_Rain) * dTime;

				if( s < 0. )
				{
					s	= N_Rain * dTime;
				}

				pN_1->Set_Value(x, y, s);
			}
		}
	}

	//-----------------------------------------------------
	return( true );
}

///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
