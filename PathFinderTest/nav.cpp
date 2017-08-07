#include "nav.h"

#ifndef _MSC_VER
#include <stdbool.h>
#else
#define inline __inline
#define false 0
#define NULL 0
#endif

#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))

double cross(struct vector3* vt1,struct vector3* vt2)
{
	return vt1->z * vt2->x - vt1->x * vt2->z;
}

void cross_pt(struct vector3* a,struct vector3* b,struct vector3* c,struct vector3* d,struct vector3* result)
{
	result->x = ((b->x - a->x) * (c->x - d->x) * (c->z - a->z) - c->x * (b->x - a->x) * (c->z - d->z) + a->x * (b->z - a->z) * (c->x - d->x))/((b->z - a->z)*(c->x - d->x) - (b->x - a->x) * (c->z - d->z));
	result->z = ((b->z - a->z) * (c->z - d->z) * (c->x - a->x) - c->z * (b->z - a->z) * (c->x - d->x) + a->z * (b->x - a->x) * (c->z - d->z))/((b->x - a->x)*(c->z - d->z) - (b->z - a->z) * (c->x - d->x));
}

void vector3_copy(struct vector3* dst,struct vector3* src)
{
	dst->x = src->x;
	dst->y = src->y;
	dst->z = src->z;
}

void vector3_sub(struct vector3* a,struct vector3* b,struct vector3* result)
{
	result->x = a->x - b->x;
	result->y = a->y - b->y;
	result->z = a->z - b->z;
}

double vector3_angle(struct vector3* start,struct vector3* over)
{
	double dot = start->x * over->x + start->z * over->z;
	double tmp = dot/(sqrt(start->x*start->x+ start->z*start->z) * sqrt(over->x*over->x+ over->z*over->z));
	return acos(tmp);
}


bool in_poly(struct MeshContext* mesh_ctx,int* poly,int size,struct vector3* vt3)
{
	int forward = 0;
	for (int i = 0;i < size;i++)
	{
		struct vector3* vt1 = &mesh_ctx->vertices[poly[i]];
		struct vector3* vt2 = &mesh_ctx->vertices[poly[(i+1)%size]];

		struct vector3 vt21;
		vt21.x = vt2->x - vt1->x;
		vt21.y = 0;
		vt21.z = vt2->z - vt1->z;

		struct vector3 vt31;
		vt31.x = vt3->x - vt1->x;
		vt31.y = 0;
		vt31.z = vt3->z - vt1->z;

		double y = cross(&vt21,&vt31);

		if (forward == 0)
			forward = y > 0? 1:-1;
		else
		{
			if (forward == 1 && y < 0)
				return false;
			else if (forward == -1 && y > 0)
				return false;
		}
	}
	return true;
}

bool in_node_ex(struct MeshContext* mesh_ctx,int polyId,double x,double y,double z)
{
	struct NavNode* navNode = &mesh_ctx->node[polyId];
	struct vector3 vt;
	vt.x = x;
	vt.y = y;
	vt.z = z;
	return in_poly(mesh_ctx,navNode->poly,navNode->size,&vt);
}

bool in_node(struct MeshContext* mesh_ctx,int polyId,double x,double y,double z)
{
	int cross_cnt = 0;
	
	struct vector3 vt;
	vt.x = x;
	vt.y = y;
	vt.z = z;

	struct NavNode* navNode = &mesh_ctx->node[polyId];
	for (int i = 0; i < navNode->size; i++)
	{
		struct vector3* vt1 = &mesh_ctx->vertices[navNode->poly[i]];
		struct vector3* vt2 = &mesh_ctx->vertices[navNode->poly[(i+1)%navNode->size]];
		
		if (vt1->z == vt2->z)
			continue;

		if (vt.z < min(vt1->z,vt2->z))
			continue;

		if (vt.z >= max(vt1->z, vt2->z))
			continue;

		double x = (vt.z - vt1->z)*(vt2->x - vt1->x)/(vt2->z - vt1->z)+ vt1->x;
		if (x > vt.x)
			cross_cnt++;
	}

	if (cross_cnt%2 == 1)
		return true;

	return false;
}

struct NavNode* find_node(struct MeshContext* mesh_ctx,int polyId)
{
	if (polyId < 0 || polyId >= mesh_ctx->size)
	{
		return NULL;
	}
	return &mesh_ctx->node[polyId];
}

struct NavNode* find_node_with_pos(struct MeshContext* mesh_ctx,double x,double y,double z)
{
	for (int i = 0; i < mesh_ctx->size;i++)
	{
		if (in_node_ex(mesh_ctx,i,x,y,z))
		{
			return &mesh_ctx->node[i];
		}
	}
	return NULL;
}

struct Border* get_border(struct MeshContext* mesh_ctx, int a, int b)
{
	struct BorderContext * border_ctx = &mesh_ctx->border_ctx;
	int i;
	for (i = 0; i < border_ctx->border_offset; i++)
	{
		struct Border* border = &border_ctx->borders[i];
		if (border->a == a && border->b == b)
			return border;
	}
	return NULL;;
}

struct Border* get_border_with_id(struct MeshContext* mesh_ctx, int id)
{
	struct BorderContext * border_ctx = &mesh_ctx->border_ctx;
	if (id < 0 || id > border_ctx->border_offset)
		return NULL;
	return &border_ctx->borders[id];
}

void add_border(struct MeshContext* mesh_ctx, int a, int b)
{
	struct BorderContext * border_ctx = &mesh_ctx->border_ctx;
	if (border_ctx->border_offset + 1 >= border_ctx->border_cap)
	{
		int ncap = border_ctx->border_cap * 2;
		struct Border* oborders = border_ctx->borders;
		border_ctx->borders = (struct Border*)malloc(sizeof(struct Border) * ncap);
		memcpy(border_ctx->borders, oborders, sizeof(struct Border) * border_ctx->border_cap);
		border_ctx->border_cap = ncap;
		free(oborders);
	}

	struct Border * border = &border_ctx->borders[border_ctx->border_offset];
	border->id = border_ctx->border_offset;
	border->a = a;
	border->b = b;
	border->node[0] = -1;
	border->node[1] = -1;
	border->opposite = -1;

	border_ctx->border_offset++;
}

void border_link_node(struct Border* border,int id)
{
	if (border->node[0] == -1)
		border->node[0] = id;
	else if (border->node[1] == -1)
		border->node[1] = id;
	else
		assert(0);
}

struct list* get_link(struct MeshContext* mesh_ctx, struct NavNode* node)
{
	if (!LIST_EMPTY((&mesh_ctx->linked)))
		LIST_POP(&mesh_ctx->linked);
	int i;
	for (i = 0; i < node->size;i++)
	{
		int border_index = node->border[i];
		struct Border* border = get_border_with_id(mesh_ctx, border_index);
		assert(border != NULL);

		int linked = -1;
		if (border->node[0] == node->id)
			linked = border->node[1];
		else
			linked = border->node[0];

		if (linked != -1)
		{
			struct NavNode* tmp = find_node(mesh_ctx,linked);
			assert(tmp != NULL);
			if (tmp->list_head.pre || tmp->list_head.next)
				continue;
			LIST_PUSH((&mesh_ctx->linked),((struct list_node*)tmp));
			tmp->reserve = border->opposite;
		}
	}

	if (LIST_EMPTY((&mesh_ctx->linked)))
		return NULL;
	
	return &mesh_ctx->linked;
}

double get_cost(struct NavNode* from,struct NavNode* to)
{
	double dx = from->center.x - to->center.x;
	double dy = from->center.y - to->center.y;
	double dz = from->center.z - to->center.z;
	return sqrt(dx*dx + dy* dy + dz* dz);
}

static inline int less(struct element * left, struct element * right) 
{
	struct NavNode *l = (struct NavNode*)((int8_t*)left - sizeof(struct list_node));
	struct NavNode *r = (struct NavNode*)((int8_t*)right - sizeof(struct list_node));
	return l->F < r->F;
}

int vertex_cmp(const void * left,const void * right) 
{
	struct VertexInfo *l = (struct VertexInfo*)left;
	struct VertexInfo *r = (struct VertexInfo*)right;
	
	struct vector3 pt0,pt1;
	vector3_copy(&pt0,&l->ctx->vertices[l->index]);
	vector3_copy(&pt1,&l->ctx->vertices[r->index]);

	if (pt0.x >= 0 && pt1.x < 0)
		return 1;
	if (pt0.x == 0 && pt1.x == 0)
		return pt0.z > pt1.z;

	struct vector3 vt0,vt1;
	vector3_sub(&pt0,&l->center,&vt0);
	vector3_sub(&pt1,&r->center,&vt1);

	double det = cross(&vt0,&vt1);
	if (det < 0)
		return 0;
	if (det > 0)
		return 1;

	return (vt0.x* vt0.x +vt0.z * vt0.z) > (vt1.x* vt1.x +vt1.z * vt1.z);
}

void vertex_sort(struct MeshContext* ctx, NavNode* node)
{
	int j;
	struct VertexInfo* vertex = (struct VertexInfo*)malloc(sizeof(*vertex) * node->size);
	for (j = 0;j < node->size;j++)
	{
		vertex[j].ctx = ctx;
		vertex[j].index = node->poly[j];
	}

	for (j = 0; j < node->size;j++)
		vector3_copy(&vertex[j].center,&node->center);

	qsort(vertex,node->size,sizeof(struct VertexInfo),vertex_cmp);

	for (j = 0;j <= node->size;j++)
		node->poly[j] = vertex[j].index;

	free(vertex);
}

struct MeshContext* load_mesh(double** v,int v_cnt,int** p,int p_cnt)
{
	struct MeshContext* mesh_ctx = (struct MeshContext*)malloc(sizeof(*mesh_ctx));
	memset(mesh_ctx,0,sizeof(*mesh_ctx));

	mesh_ctx->len = v_cnt;
	mesh_ctx->vertices = (struct vector3 *)malloc(sizeof(struct vector3) * mesh_ctx->len);
	memset(mesh_ctx->vertices,0,sizeof(struct vector3) * mesh_ctx->len);

	mesh_ctx->border_ctx.border_cap = 16;
	mesh_ctx->border_ctx.border_offset = 0;
	mesh_ctx->border_ctx.borders = (struct Border *)malloc(sizeof(struct Border) * mesh_ctx->border_ctx.border_cap);
	memset(mesh_ctx->border_ctx.borders,0,sizeof(struct Border) * mesh_ctx->border_ctx.border_cap);

	mesh_ctx->size = p_cnt;
	mesh_ctx->node = (struct NavNode *)malloc(sizeof(struct NavNode) * mesh_ctx->size);
	memset(mesh_ctx->node,0,sizeof(struct NavNode) * mesh_ctx->size);

	//加载顶点
	int i,j,k;
	for (i = 0;i < v_cnt;i++)
	{
		mesh_ctx->vertices[i].x = v[i][0];
		mesh_ctx->vertices[i].y = v[i][1];
		mesh_ctx->vertices[i].z = v[i][2];
	}

	//加载多边形索引
	for (i = 0;i < p_cnt;i++)
	{
		struct NavNode* node = &mesh_ctx->node[i];
		memset(node,0,sizeof(*node));
		node->id = i;

		node->size = p[i][0];

		node->border = (int*)malloc(node->size * sizeof(int));
		node->poly =(int*)malloc(node->size * sizeof(int));

		struct vector3 center;
		center.x = center.y = center.z = 0;

		node->link_border = -1;
		node->link_parent = NULL;

		for (j = 1;j <= node->size;j++)
		{
			node->poly[j-1] = p[i][j];
			center.x += mesh_ctx->vertices[p[i][j]].x;
			center.y += mesh_ctx->vertices[p[i][j]].y;
			center.z += mesh_ctx->vertices[p[i][j]].z;
		}

		node->center.x = center.x / node->size;
		node->center.y = center.y / node->size;
		node->center.z = center.z / node->size;

		//顶点顺时针排序
		vertex_sort(mesh_ctx,node);

		//同时生成顺时针和逆时针的多边形的边border,并记录边的两边多边形
		for (k = 0; k < node->size;k++)
		{
			int k0 = k;
			int k1 = k + 1 >= node->size ? 0 : k + 1;
			
			int a = node->poly[k0];
			int b = node->poly[k1];

			struct Border* border0 = get_border(mesh_ctx, a, b);
			if (border0 == NULL)
			{
				add_border(mesh_ctx, a, b);
				border0 = get_border(mesh_ctx, a, b);
			}
			border_link_node(border0,node->id);

			node->border[k] = border0->id;
			
			struct Border* border1 = get_border(mesh_ctx, b, a);
			if (border1 == NULL)
			{
				add_border(mesh_ctx, b, a);
				border1 = get_border(mesh_ctx, b, a);
			}
			border_link_node(border1,node->id);
		}
	}

	//记录每条边反方向顶点的边
	for (int i = 0;i < mesh_ctx->border_ctx.border_offset;i++)
	{
		struct Border* border = get_border_with_id(mesh_ctx,i);
		for (int j = 0;j < mesh_ctx->border_ctx.border_offset;j++)
		{
			struct Border* tmp = get_border(mesh_ctx,border->b,border->a);
			if (tmp != NULL)
				border->opposite = tmp->id;
		}
	}

	mesh_ctx->openlist = minheap_new(50 * 50, less);
	LIST_INIT((&mesh_ctx->closelist));
	LIST_INIT((&mesh_ctx->linked));
	return mesh_ctx;
}