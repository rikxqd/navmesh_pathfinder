
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

#define USE_NAV_TILE

struct vector3
{
	double x;
	double y;
	double z;
};

struct nav_path_context
{
	struct vector3* wp;
	int offset;
	int size;
};

struct nav_mesh_mask
{
	int *mask;
	int size;
};

struct nav_node
{
	struct list_node list_head;
	struct element elt;

	int id;

	int* poly;
	int* border;
	int size;

	int mask;

	//多边形的中心点
	struct vector3 center;

	double G;
	double H;
	double F;

	//缓存A*寻路出来的相邻多边形和与相邻多边形共边的边
	struct nav_node* link_parent;
	int link_border;

	int reserve;
};

struct nav_border
{
	int id;
	int node[2];
	int a;
	int b;
	int opposite;
};

struct nav_border_search_node
{
	struct nav_border_search_node* next;
	int id;
	int index;
};

struct nav_border_context
{
	struct nav_border* borders;
	int border_cap;
	int border_offset;
};

struct nav_tile
{
	int mask;
	int* node;
	int offset;
	int size;
	struct vector3 center;
	struct vector3 pos[4];
};

struct nav_mesh_context
{
	//顶点
	struct vector3 * vertices;
	int len;

	//所有边(同一条边有ab和ba两条)
	struct nav_border_context border_ctx;
	struct nav_border_search_node** border_searcher;
	 
	//多边形节点
	struct nav_node* node;
	int size;

	//格子信息
	struct nav_tile* tile;
	struct vector3 lt;
	struct vector3 br;
	int width;
	int heigh;

	//多边形节点的mask
	struct nav_mesh_mask mask_ctx;

	//寻路结果缓存
	struct nav_path_context result;

	//获取相邻多边形缓存
	struct list linked;

	struct minheap* openlist;
	struct list closelist;
};

struct vertex_sort_info
{
	struct nav_mesh_context* ctx;
	int index;
	struct vector3 center;
};


struct nav_mesh_context* load_mesh(double** v,int v_cnt,int** p,int p_cnt);
void release_mesh(struct nav_mesh_context* ctx);

struct nav_node* get_node(struct nav_mesh_context* ctx,int id);
struct nav_node* get_node_with_pos(struct nav_mesh_context* mesh_ctx,double x,double y,double z);

struct nav_path_context* astar_find(struct nav_mesh_context* mesh_ctx,struct vector3* pt0,struct vector3* pt1);
bool raycast(struct nav_mesh_context* ctx,struct vector3* pt0,struct vector3* pt1,struct vector3* result);

void set_mask(struct nav_mesh_mask* ctx,int mask,int enable);
int get_mask(struct nav_mesh_mask* ctx,int mask);

#endif