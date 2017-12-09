
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
#define true 1
#endif

#define USE_NAV_TILE

#define GRATE 1
#define HRATE 2

#define get_border(ctx,id) ((id < 0 || id > ctx->border_ctx.border_offset)?NULL:&ctx->border_ctx.borders[id])

#define get_node(ctx,id) ((id < 0 || id >= ctx->size) ?NULL:&ctx->node[id])

#define get_mask(ctx,index) (ctx.mask[index])

#define max(a,b)    (((a) > (b)) ? (a) : (b))

#define min(a,b)    (((a) < (b)) ? (a) : (b))

#define cast_node(elt) ((struct nav_node*)((int8_t*)elt - sizeof(struct list_node)))

struct vector3
{
	double x;
	double y;
	double z;
};

struct nav_path
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

	//����ε����ĵ�
	struct vector3 center;

	double G;
	double H;
	double F;

	//����A*Ѱ·���������ڶ���κ������ڶ���ι��ߵı�
	struct nav_node* link_parent;
	int link_border;

	int reserve;
	struct vector3 pos;
};

struct nav_border
{
	int id;
	int node[2];
	int a;
	int b;
	int opposite;
	struct vector3 center;
};


struct nav_border_context
{
	struct nav_border* borders;
	int border_cap;
	int border_offset;
};

struct nav_tile
{
	int* node;
	int offset;
	int size;
	struct vector3 center;
	struct vector3 pos[4];
};

struct nav_mesh_context
{
	//����
	struct vector3 * vertices;
	int len;

	//���б�(ͬһ������ab��ba����)
	struct nav_border_context border_ctx;

	//����νڵ�
	struct nav_node* node;
	int size;

	//������Ϣ
	struct nav_tile* tile;
	
	struct vector3 lt;
	struct vector3 br;
	int width;
	int heigh;

	//����νڵ��mask
	struct nav_mesh_mask mask_ctx;

	//Ѱ·�������
	struct nav_path result;

	//��ȡ���ڶ���λ���
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

typedef void(*search_dumper)(void* ud, int index);

struct nav_mesh_context* load_mesh(double** v,int v_cnt,int** p,int p_cnt);
void load_mesh_done(struct nav_mesh_context* ctx);
void release_mesh(struct nav_mesh_context* ctx);

struct nav_node* get_node_with_pos(struct nav_mesh_context* mesh_ctx,double x,double y,double z);
struct nav_path* astar_find(struct nav_mesh_context* mesh_ctx, struct vector3* pt_start, struct vector3* pt_over, search_dumper dumper, void* args);
int raycast(struct nav_mesh_context* ctx, struct vector3* pt_start, struct vector3* pt_over, struct vector3* result, search_dumper dumper, void* userdata);

void set_mask(struct nav_mesh_mask* ctx,int mask,int enable);

struct vector3* around_movable(struct nav_mesh_context*, double x, double z, double y, int range, search_dumper, void*);
int point_movable(struct nav_mesh_context* ctx, double x, double z, double y);

int intersect(struct vector3* a, struct vector3* b, struct vector3* c, struct vector3* d);
int inside_node(struct nav_mesh_context* mesh_ctx, int polyId, double x, double y, double z);
double cross(struct vector3* vt1,struct vector3* vt2);
void cross_point(struct vector3* a,struct vector3* b,struct vector3* c,struct vector3* d,struct vector3* result);
void vector3_copy(struct vector3* dst,struct vector3* src);
void vector3_sub(struct vector3* a,struct vector3* b,struct vector3* result);

struct nav_tile* create_tile(struct nav_mesh_context* ctx);
void release_tile(struct nav_mesh_context* ctx,struct nav_tile* navtile);

#endif