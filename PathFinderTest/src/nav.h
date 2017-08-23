
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

	struct vector3 center;

	double G;
	double H;
	double F;

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

	//Ѱ·�������
	struct nav_path_context result;

	//��ȡ���ڶ���λ���
	struct list linked;

	//����νڵ��mask
	struct nav_mesh_mask mask_ctx;

	struct minheap* openlist;
	struct list closelist;
};

struct VertexInfo
{
	struct nav_mesh_context* ctx;
	int index;
	struct vector3 center;
};




struct nav_node* get_node(struct nav_mesh_context* ctx,int id);
struct nav_node* get_node_with_pos(struct nav_mesh_context* mesh_ctx,double x,double y,double z);
struct nav_mesh_context* load_mesh(double** v,int v_cnt,int** p,int p_cnt);
struct nav_path_context* astar_find(struct nav_mesh_context* mesh_ctx,struct vector3* pt0,struct vector3* pt1);
bool raycast(struct nav_mesh_context* ctx,struct vector3* pt0,struct vector3* pt1,struct vector3* result);
void set_mask(struct nav_mesh_mask* ctx,int mask,int enable);
#endif