/****************************************************************************************** 
 *	Chili DirectX Framework Version 14.03.22											  *	
 *	D3DGraphics.cpp																		  *
 *	Copyright 2014 PlanetChili.net <http://www.planetchili.net>							  *
 *																						  *
 *	This file is part of The Chili DirectX Framework.									  *
 *																						  *
 *	The Chili DirectX Framework is free software: you can redistribute it and/or modify	  *
 *	it under the terms of the GNU General Public License as published by				  *
 *	the Free Software Foundation, either version 3 of the License, or					  *
 *	(at your option) any later version.													  *
 *																						  *
 *	The Chili DirectX Framework is distributed in the hope that it will be useful,		  *
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of						  *
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the						  *
 *	GNU General Public License for more details.										  *
 *																						  *
 *	You should have received a copy of the GNU General Public License					  *
 *	along with The Chili DirectX Framework.  If not, see <http://www.gnu.org/licenses/>.  *
 ******************************************************************************************/
#include "D3DGraphics.h"
#include "ChiliMath.h"
#include <assert.h>
#pragma comment( lib,"d3d9.lib" )

D3DGraphics::D3DGraphics( HWND hWnd )
	:
pDirect3D( NULL ),
pDevice( NULL ),
pBackBuffer( NULL ),
pSysBuffer( NULL )
{
	HRESULT result;
	
	pDirect3D = Direct3DCreate9( D3D_SDK_VERSION );
	assert( pDirect3D != NULL );

    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory( &d3dpp,sizeof( d3dpp ) );
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	d3dpp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;

    result = pDirect3D->CreateDevice( D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,hWnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE,&d3dpp,&pDevice );
	assert( !FAILED( result ) );

	result = pDevice->GetBackBuffer( 0,0,D3DBACKBUFFER_TYPE_MONO,&pBackBuffer );
	assert( !FAILED( result ) );

	pSysBuffer = new D3DCOLOR[ SCREENWIDTH * SCREENHEIGHT ];
}

D3DGraphics::~D3DGraphics()
{
	if( pDevice )
	{
		pDevice->Release();
		pDevice = NULL;
	}
	if( pDirect3D )
	{
		pDirect3D->Release();
		pDirect3D = NULL;
	}
	if( pBackBuffer )
	{
		pBackBuffer->Release();
		pBackBuffer = NULL;
	}
	if( pSysBuffer )
	{
		delete pSysBuffer;
		pSysBuffer = NULL;
	}
}

void D3DGraphics::BeginFrame()
{
	memset( pSysBuffer,FILLVALUE,sizeof(D3DCOLOR)* SCREENWIDTH * SCREENHEIGHT );
}

void D3DGraphics::EndFrame()
{
	HRESULT result;
	D3DLOCKED_RECT backRect;

	result = pBackBuffer->LockRect( &backRect,NULL,NULL );
	assert( !FAILED( result ) );

	for( int y = 0; y < SCREENHEIGHT; y++ )
	{
		memcpy( &((BYTE*)backRect.pBits)[backRect.Pitch * y],&pSysBuffer[SCREENWIDTH * y],sizeof(D3DCOLOR)* SCREENWIDTH );
	}

	result = pBackBuffer->UnlockRect( );
	assert( !FAILED( result ) );

	result = pDevice->Present( NULL,NULL,NULL,NULL );
	assert( !FAILED( result ) );
}

void D3DGraphics::PutPixel( int x,int y,D3DCOLOR c )
{	
	assert( x >= 0 );
	assert( y >= 0 );
	assert( x < SCREENWIDTH );
	assert( y < SCREENHEIGHT );
	pSysBuffer[ x + SCREENWIDTH * y ] = c;
}

D3DCOLOR D3DGraphics::GetPixel(int x, int y) const
{
	assert(x >= 0);
	assert(y >= 0);
	assert(x < SCREENWIDTH);
	assert(y < SCREENHEIGHT);
	return pSysBuffer[x + SCREENWIDTH * y];
}

typedef OutCode;

const int INSIDE = 0; // 0000
const int LEFT = 1;   // 0001
const int RIGHT = 2;  // 0010
const int BOTTOM = 4; // 0100
const int TOP = 8;    // 1000

// Compute the bit code for a point(x, y) using the clip rectangle
// bounded diagonally by (xmin, ymin), and (xmax,ymax)
// ASSUME THAT xmax, xmin, ymax and ymin are global constants.
// change from double to float
OutCode ComputeOutCode(float x, float y)
{
	const float xmin = 0.0f;
	const float xmax = (float)(D3DGraphics::SCREENWIDTH - 1);
	const float ymin = 0.0f;
	const float ymax = (float)(D3DGraphics::SCREENHEIGHT - 1);

	OutCode code;

	code = INSIDE;          // initialised as being inside of [[clip window]]

	if (x < xmin)           // to the left of clip window
		code |= LEFT;
	else if (x > xmax)      // to the right of clip window
		code |= RIGHT;
	if (y < ymin)           // below the clip window
		code |= BOTTOM;
	else if (y > ymax)      // above the clip window
		code |= TOP;

	return code;
}

// Cohen–Sutherland clipping algorithm clips a line from
// P0 = (x0, y0) to P1 = (x1, y1) against a rectangle with 
// diagonal from (xmin, ymin) to (xmax, ymax).
// changed all doubles to floats
//void CohenSutherlandLineClipAndDraw(float x0, float y0, float x1, float y1)
//void CohenSutherlandLineClipAndDraw( Vec2 p0, Vec2 p1, D3DCOLOR color)
//void DrawLineClip( Vec2 p0, Vec2 p1, D3DCOLOR color)
void D3DGraphics::DrawLineClip( Vec2 p0, Vec2 p1, D3DCOLOR color)
{
	const float xmin = 0.0f;
	const float xmax = (float)(D3DGraphics::SCREENWIDTH - 1);
	const float ymin = 0.0f;
	const float ymax = (float)(D3DGraphics::SCREENHEIGHT - 1);

	// compute outcodes for P0, P1, and whatever point lies outside the clip rectangle
	OutCode outcode0 = ComputeOutCode(p0.x, p0.y);
	OutCode outcode1 = ComputeOutCode(p1.x, p1.y);
	bool accept = false;

	while (true) {
		if (!(outcode0 | outcode1)) { // Bitwise OR is 0. Trivially accept and get out of loop
			accept = true;
			break;
		}
		else if (outcode0 & outcode1) { // Bitwise AND is not 0. Trivially reject and get out of loop
			break;
		}
		else {
			// failed both tests, so calculate the line segment to clip
			// from an outside point to an intersection with clip edge
			//double x, y;
			float x, y;

			// At least one endpoint is outside the clip rectangle; pick it.
			OutCode outcodeOut = outcode0 ? outcode0 : outcode1;

			// Now find the intersection point;
			// use formulas y = y0 + slope * (x - x0), x = x0 + (1 / slope) * (y - y0)
			if (outcodeOut & TOP) {           // point is above the clip rectangle
				x = p0.x + (p1.x - p0.x) * (ymax - p0.y) / (p1.y - p0.y);
				y = ymax;
			}
			else if (outcodeOut & BOTTOM) { // point is below the clip rectangle
				x = p0.x + (p1.x - p0.x) * (ymin - p0.y) / (p1.y - p0.y);
				y = ymin;
			}
			else if (outcodeOut & RIGHT) {  // point is to the right of clip rectangle
				y = y0 + (p1.y - p0.y) * (xmax - p0.x) / (p1.x - p0.x);
				x = xmax;
			}
			else if (outcodeOut & LEFT) {   // point is to the left of clip rectangle
				y = p0.y + (p1.y - p0.y) * (xmin - p0.x) / (p1.x - p0.x);
				x = xmin;
			}

			// Now we move outside point to intersection point to clip
			// and get ready for next pass.
			if (outcodeOut == outcode0) {
				p0.x = x;
				p0.y = y;
				outcode0 = ComputeOutCode(p0.x, p0.y);
			}
			else {
				p1.x = x;
				p1.y = y;
				outcode1 = ComputeOutCode(p1.x, p1.y);
			}
		}
	}
	if (accept) 
	{
		// Following functions are left for implementation by user based on
		// their platform (OpenGL/graphics.h etc.)
		//DrawRectangle(xmin, ymin, xmax, ymax);
		//LineSegment(x0, y0, x1, y1);
		DrawLine((int)(p0.x + 0.5), (int)(p0.y + 0.5), (int)(p1.x + 0.5, (int)(p1.y + 0.5), color);

	}
}


void D3DGraphics::DrawLine( int x1,int y1,int x2,int y2,D3DCOLOR c )
{	
	const int dx = x2 - x1;
	const int dy = y2 - y1;

	if( dy == 0 && dx == 0 )
	{
		PutPixel( x1,y1,c );
	}
	else if( abs( dy ) > abs( dx ) )
	{
		if( dy < 0 )
		{
			int temp = x1;
			x1 = x2;
			x2 = temp;
			temp = y1;
			y1 = y2;
			y2 = temp;
		}
		const float m = (float)dx / (float)dy;
		const float b = x1 - m*y1;
		for( int y = y1; y <= y2; y = y + 1 )
		{
			int x = (int)(m*y + b + 0.5f);
			PutPixel( x,y,c );
		}
	}
	else
	{
		if( dx < 0 )
		{
			int temp = x1;
			x1 = x2;
			x2 = temp;
			temp = y1;
			y1 = y2;
			y2 = temp;
		}
		const float m = (float)dy / (float)dx;
		const float b = y1 - m*x1;
		for( int x = x1; x <= x2; x = x + 1 )
		{
			int y = (int)(m*x + b + 0.5f);
			PutPixel( x,y,c );
		}
	}
}

void D3DGraphics::DrawCircle( int centerX,int centerY,int radius,D3DCOLOR color )
{
	int rSquared = sq( radius );
	int xPivot = (int)( radius * 0.70710678118f + 0.5f );
	for( int x = 0; x <= xPivot; x++ )
	{
		int y = (int)(sqrt( (float)( rSquared - sq( x ) ) ) + 0.5f);
		PutPixel( centerX + x,centerY + y,color );
		PutPixel( centerX - x,centerY + y,color );
		PutPixel( centerX + x,centerY - y,color );
		PutPixel( centerX - x,centerY - y,color );
		PutPixel( centerX + y,centerY + x,color );
		PutPixel( centerX - y,centerY + x,color );
		PutPixel( centerX + y,centerY - x,color );
		PutPixel( centerX - y,centerY - x,color );
	}
}