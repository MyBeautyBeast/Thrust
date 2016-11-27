#pragma once
#include "Vec2.h"
#include "Colors.h"
#include "D3DGraphics.h"
#include "dxflib/dl_creationadapter.h"
#include "dxflib/dl_dxf.h"
#include <vector>
#include <memory>

class PolyClosedLoader:public DL_CreationAdapter 
{
public:
	PolyClosedLoader(std::string filename)
	{
		//std::unique_ptr<DL_Dxf>pDxf = std::make_unique<DL_Dxf>();
		//std::unique_ptr<DL_Dxf> pDxf(new DL_Dxf);
		auto pDxf = std::make_unique<DL_Dxf>();
		pDxf->in(filename,this);
	}
	virtual void addVertex(const DL_VertexData& data) override
	{
		vertices.push_back({ (float)data.x, -(float)data.y });
	}
    std::vector< const Vec2 >& GetVertices()  
	{
		return vertices;
	}
private:
	std::vector< const Vec2 > vertices;
};


class PolyClosed
{
public:
	PolyClosed(std::initializer_list <Vec2> vList, D3DCOLOR color = WHITE)
		:
		vertices(vList),
		color(color)
	{}
	// line 44 vertices(PolyClosedLoader(filename).GetVertices()),
	PolyClosed(std::string filename, D3DCOLOR color = WHITE)
		:
		vertices(std::move(PolyClosedLoader(filename).GetVertices())),
		color(color)
	{}
	void Draw(Vec2 pos, float angle, D3DGraphics& gfx) const
	{
		for (auto i = vertices.begin(), end = vertices.end() - 1; i != end; i++)
		{
			gfx.DrawLine(i->Rotation(angle) + pos, (i + 1)->Rotation(angle) + pos, color);
			//gfx.DrawLine(*i + pos, *(i + 1) + pos, color);
		}
		gfx.DrawLine(vertices.back().Rotation(angle) + pos, vertices.front().Rotation(angle) + pos, color);
	}
private:
	D3DCOLOR color;
	const std::vector<const Vec2> vertices;
};