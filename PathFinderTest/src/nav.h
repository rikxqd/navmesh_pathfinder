
#ifndef NAV_H
#define NAV_H
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "minheap.h"
#include "list.h"

#ifndef _MSC_VER
#include <stdbool.h>
#else
#define inline __inline
#define false 0
#define NULL 0
#endif

struct vector3
{
	double x;
	double y;
	double z;
};

struct PathContext
{
	struct vector3* wp;
	int offset;
	int size;
};

struct MeshMask
{
	int *mask;
	int size;
};

struct NavNode
{
	struct list_node list_head;
	struct element elt;

	int id;

	int* poly;
	int* border;
	int size;

	int mask;

	struct vector3 center;

	double G;
	double H;
	double F;

	struct NavNode* link_parent;
	int link_border;

	int reserve;
};

struct Border
{
	int id;
	int node[2];
	int a;
	int b;
	int opposite;
};

struct BorderContext
{
	struct Border* borders;
	int border_cap;
	int border_offset;
};

struct Tile
{
	int mask;
	int* node;
	int offset;
	int size;
	struct vector3 center;
	struct vector3 pos[4];
};

struct MeshContext
{
	//顶点
	struct vector3 * vertices;
	int len;

	//所有边(同一条边有ab和ba两条)
	struct BorderContext border_ctx;

	//多边形节点
	struct NavNode* node;
	int size;

	//格子信息
	struct Tile* tile;
	struct vector3 lt;
	struct vector3 br;
	int count;
	int width;
	int heigh;

	//寻路结果缓存
	struct PathContext result;

	//获取相邻多边形缓存
	struct list linked;

	//多边形节点的mask
	struct MeshMask mask_ctx;

	struct minheap* openlist;
	struct list closelist;
};

struct VertexInfo
{
	struct MeshContext* ctx;
	int index;
	struct vector3 center;
};




struct NavNode* find_node(struct MeshContext* ctx,int id);
struct NavNode* find_node_with_pos(struct MeshContext* mesh_ctx,double x,double y,double z);
struct MeshContext* load_mesh(double** v,int v_cnt,int** p,int p_cnt);
struct PathContext* astar_find(struct MeshContext* mesh_ctx,struct vector3* pt0,struct vector3* pt1);
bool raycast(struct MeshContext* ctx,struct vector3* pt0,struct vector3* pt1,struct vector3* result);
void set_mask(struct MeshMask* ctx,int mask,int enable);
#endif