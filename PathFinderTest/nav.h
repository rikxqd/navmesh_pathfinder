
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
#endif


struct vector3
{
	double x;
	double y;
	double z;
};

struct NavNode
{
	struct list_node list_head;
	struct element elt;
	
	int id;
	
	int* poly;
	int* border;
	int size;
	
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

struct MeshContext
{
	struct vector3 * vertices;
	int len;

	struct BorderContext border_ctx;

	struct NavNode* node;
	int size;

	struct minheap* openlist;
	struct list closelist;
	struct list linked;
};

struct VertexInfo
{
	struct MeshContext* ctx;
	int index;
	struct vector3 center;
};

struct NavNode* find_node(struct MeshContext* meshCtx,int polyId);
struct NavNode* find_node_with_pos(struct MeshContext* mesh_ctx,double x,double y,double z);
struct list* get_link(struct MeshContext* mesh_ctx, struct NavNode* node);
double get_cost(struct NavNode* from,struct NavNode* to);
struct Border* get_border(struct MeshContext* mesh_ctx, int a, int b);
struct Border* get_border_with_id(struct MeshContext* mesh_ctx, int id);
struct MeshContext* load_mesh(double** v,int v_cnt,int** p,int p_cnt);
struct NavNode* astar_poly_path(struct MeshContext* mesh_ctx,struct vector3* pt0,struct vector3* pt1,struct vector3*&result,int * size);
bool raycast(struct MeshContext* mesh_ctx,int polyId,struct vector3* pt0,struct vector3* pt1,struct vector3* result);
double cross(struct vector3* vt1,struct vector3* vt2);
void cross_pt(struct vector3* pt0,struct vector3* pt1,struct vector3* pt2,struct vector3* pt3,struct vector3* result);
bool in_node_ex(struct MeshContext* mesh_ctx,int polyId,double x,double y,double z);
void node_reset(struct NavNode* node);

void vector3_copy(struct vector3* dst,struct vector3* src);
void vector3_sub(struct vector3* a,struct vector3* b,struct vector3* result);
#endif