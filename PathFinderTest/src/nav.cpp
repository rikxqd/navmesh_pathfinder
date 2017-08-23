#include "nav.h"


#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))


void set_mask(struct nav_mesh_mask* ctx,int mask,int enable)
{
	if (mask >= ctx->size)
	{
		int nsize = ctx->size * 2;
		int* mask_ptr = ctx->mask;
		ctx->mask = (int*)malloc(sizeof(int) * nsize);
		memcpy(ctx->mask, mask_ptr, sizeof(int) * ctx->size);
		ctx->size = nsize;
		free(mask_ptr);
	}
	ctx->mask[mask] = enable;
}

int get_mask(struct nav_mesh_mask* ctx,int mask)
{
	return ctx->mask[mask];
}

double cross(struct vector3* vt1,struct vector3* vt2)
{
	return vt1->z * vt2->x - vt1->x * vt2->z;
}

void cross_point(struct vector3* a,struct vector3* b,struct vector3* c,struct vector3* d,struct vector3* result)
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

bool intersect(struct vector3* a,struct vector3* b,struct vector3* c,struct vector3* d)
{
	//快速排斥实验,矩形相交
	if (max(a->x,b->x) >= min(c->x,d->x) &&
		max(a->z,b->z) >= min(c->z,d->z) && 
		max(c->x,d->x) >= min(a->x,b->x) && 
		max(c->z,d->z) >= min(a->z,b->z))
	{
		struct vector3 ac,dc,bc,ca,ba,da;
		vector3_sub(a,c,&ac);
		vector3_sub(d,c,&dc);
		vector3_sub(b,c,&bc);
		vector3_sub(c,a,&ca);
		vector3_sub(b,a,&ba);
		vector3_sub(d,a,&da);

		if (cross(&ac,&dc) * cross(&dc,&bc) >= 0)
		{
			if (cross(&ca,&ba) * cross(&ba,&da) >= 0)
				return true;
		}
	}
	
	return false;
}

bool in_poly(struct nav_mesh_context* mesh_ctx,int* poly,int size,struct vector3* vt3)
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

bool in_node(struct nav_mesh_context* mesh_ctx,int polyId,double x,double y,double z)
{
	struct nav_node* nav_node = &mesh_ctx->node[polyId];
	struct vector3 vt;
	vt.x = x;
	vt.y = y;
	vt.z = z;
	return in_poly(mesh_ctx,nav_node->poly,nav_node->size,&vt);
}

struct nav_node* get_node(struct nav_mesh_context* mesh_ctx,int id)
{
	if (id < 0 || id >= mesh_ctx->size)
		return NULL;
	return &mesh_ctx->node[id];
}

struct nav_node* get_node_with_pos(struct nav_mesh_context* ctx,double x,double y,double z)
{
	//遍历查找
	//for (int i = 0; i < ctx->size;i++)
	//{
	//	if (in_node(ctx,i,x,y,z))
	//		return &ctx->node[i];
	//}

	//利用格子快速查找
	int x_index = x - ctx->lt.x;
	int z_index = z - ctx->lt.z;
	int index = x_index + z_index * ctx->width;
	struct nav_tile* tile = &ctx->tile[index];
	for (int i = 0;i < tile->offset;i++)
	{
		if (in_node(ctx,tile->node[i],x,y,z))
			return &ctx->node[tile->node[i]];
	}
	return NULL;
}

struct nav_border* get_border(struct nav_mesh_context* mesh_ctx, int a, int b)
{
	struct nav_border_context * border_ctx = &mesh_ctx->border_ctx;
	int i;
	for (i = 0; i < border_ctx->border_offset; i++)
	{
		struct nav_border* border = &border_ctx->borders[i];
		if (border->a == a && border->b == b)
			return border;
	}
	return NULL;;
}

struct nav_border* get_border_with_id(struct nav_mesh_context* mesh_ctx, int id)
{
	struct nav_border_context * border_ctx = &mesh_ctx->border_ctx;
	if (id < 0 || id > border_ctx->border_offset)
		return NULL;
	return &border_ctx->borders[id];
}

void add_border(struct nav_mesh_context* mesh_ctx, int a, int b)
{
	struct nav_border_context * border_ctx = &mesh_ctx->border_ctx;
	if (border_ctx->border_offset + 1 >= border_ctx->border_cap)
	{
		int ncap = border_ctx->border_cap * 2;
		struct nav_border* oborders = border_ctx->borders;
		border_ctx->borders = (struct nav_border*)malloc(sizeof(struct nav_border) * ncap);
		memcpy(border_ctx->borders, oborders, sizeof(struct nav_border) * border_ctx->border_cap);
		border_ctx->border_cap = ncap;
		free(oborders);
	}

	struct nav_border * border = &border_ctx->borders[border_ctx->border_offset];
	border->id = border_ctx->border_offset;
	border->a = a;
	border->b = b;
	border->node[0] = -1;
	border->node[1] = -1;
	border->opposite = -1;

	border_ctx->border_offset++;
}

void border_link_node(struct nav_border* border,int id)
{
	if (border->node[0] == -1)
		border->node[0] = id;
	else if (border->node[1] == -1)
		border->node[1] = id;
	else
		assert(0);
}

struct list* get_link(struct nav_mesh_context* mesh_ctx, struct nav_node* node)
{
	if (!LIST_EMPTY((&mesh_ctx->linked)))
		LIST_POP(&mesh_ctx->linked);
	int i;
	for (i = 0; i < node->size;i++)
	{
		int border_index = node->border[i];
		struct nav_border* border = get_border_with_id(mesh_ctx, border_index);
		assert(border != NULL);

		int linked = -1;
		if (border->node[0] == node->id)
			linked = border->node[1];
		else
			linked = border->node[0];

		if (linked != -1)
		{
			struct nav_node* tmp = get_node(mesh_ctx,linked);
			assert(tmp != NULL);
			if (tmp->list_head.pre || tmp->list_head.next)
				continue;
			if (get_mask(&mesh_ctx->mask_ctx,tmp->mask))
			{
				LIST_PUSH((&mesh_ctx->linked),((struct list_node*)tmp));
				tmp->reserve = border->opposite;
			}
		}
	}

	if (LIST_EMPTY((&mesh_ctx->linked)))
		return NULL;
	
	return &mesh_ctx->linked;
}

double get_cost(struct nav_node* from,struct nav_node* to)
{
	double dx = from->center.x - to->center.x;
	double dy = from->center.y - to->center.y;
	double dz = from->center.z - to->center.z;
	return sqrt(dx*dx + dy* dy + dz* dz);
}

int node_cmp(struct element * left, struct element * right) 
{
	struct nav_node *l = (struct nav_node*)((int8_t*)left - sizeof(struct list_node));
	struct nav_node *r = (struct nav_node*)((int8_t*)right - sizeof(struct list_node));
	return l->F < r->F;
}

//顶点顺时针排序函数
int vertex_cmp(const void * left,const void * right) 
{
	struct vertex_sort_info *l = (struct vertex_sort_info*)left;
	struct vertex_sort_info *r = (struct vertex_sort_info*)right;
	
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

//每个多边形的顶点顺时针排序
void vertex_sort(struct nav_mesh_context* ctx, nav_node* node)
{
	struct vertex_sort_info* vertex = (struct vertex_sort_info*)malloc(sizeof(*vertex) * node->size);
	for (int i = 0;i < node->size;i++)
	{
		vertex[i].ctx = ctx;
		vertex[i].index = node->poly[i];
		vector3_copy(&vertex[i].center,&node->center);
	}

	qsort(vertex,node->size,sizeof(struct vertex_sort_info),vertex_cmp);

	for (int i = 0;i < node->size;i++)
		node->poly[i] = vertex[i].index;

	free(vertex);
}

void tile_add_node(struct nav_tile* tile,int index)
{
	if (tile->size == 0)
	{
		tile->size = 4;
		tile->node = (int*)malloc(sizeof(int) * tile->size);
	}
	if (tile->offset >= tile->size)
	{
		int nsize = tile->size * 2;
		int* onode = tile->node;
		tile->node = (int*)malloc(sizeof(int) * nsize);
		memcpy(tile->node,onode,sizeof(int)* tile->size);
		tile->size = nsize;
	}
	tile->node[tile->offset] = index;
	tile->offset++;
}

//切分格子信息
void make_tile(struct nav_mesh_context* ctx)
{
	ctx->width = ctx->br.x - ctx->lt.x;
	ctx->heigh = ctx->br.z - ctx->lt.z;
 
	int count = ctx->width * ctx->heigh;

	ctx->tile = (struct nav_tile*)malloc(sizeof(struct nav_tile)*count);
	memset(ctx->tile,0,sizeof(struct nav_tile)*count);

	for (int z = 0;z < ctx->heigh;z++)
	{
		for (int x = 0;x < ctx->width;x++)
		{
			int index = x + z * ctx->width;
			struct nav_tile* tile = &ctx->tile[index];
			tile->pos[0].x = ctx->lt.x + x;
			tile->pos[0].z = ctx->lt.z + z;
			tile->pos[1].x = ctx->lt.x + x+1;
			tile->pos[1].z = ctx->lt.z + z;
			tile->pos[2].x = ctx->lt.x + x+1;
			tile->pos[2].z = ctx->lt.z + z+1;
			tile->pos[3].x = ctx->lt.x + x;
			tile->pos[3].z = ctx->lt.z + z+1;
			tile->center.x = ctx->lt.x + x + 0.5;
			tile->center.z = ctx->lt.z + z + 0.5;
		}
	}

	for (int i = 0;i < count;i++)
	{
		struct nav_tile* tile = &ctx->tile[i];
		
		for (int j = 0;j < ctx->size;j++)
		{
			int cross_cnt  = 0;
			struct nav_node* node = &ctx->node[j];
			for (int k = 0;k < 4;k++)
			{
				for (int l = 0;l < node->size;l++)
				{
					struct nav_border* border = get_border_with_id(ctx,node->border[l]);
					if (intersect(&tile->pos[k],&tile->pos[(k+1)%4],&ctx->vertices[border->a],&ctx->vertices[border->b]))
						cross_cnt++;
				}
			}
			
			if (cross_cnt > 0)
				tile_add_node(tile,j);
			else
			{
				if (in_node(ctx,j,tile->center.x,tile->center.y,tile->center.z))
					tile_add_node(tile,j);
			}

		}
	}

	for (int i = 0;i < count;i++)
	{
		struct nav_tile* tile = &ctx->tile[i];
		if (tile->node == NULL)
			tile->mask = -1;
		else
		{
			int mask_max = 0;
			for (int j = 0;j < tile->offset;j++)
			{
				struct nav_node* node = get_node(ctx,tile->node[j]);
				if (node->mask > mask_max)
					mask_max = node->mask;
			}
			tile->mask = mask_max;
		}
	}
}

struct nav_mesh_context* load_mesh(double** v,int v_cnt,int** p,int p_cnt)
{
	struct nav_mesh_context* mesh_ctx = (struct nav_mesh_context*)malloc(sizeof(*mesh_ctx));
	memset(mesh_ctx,0,sizeof(*mesh_ctx));

	mesh_ctx->len = v_cnt;
	mesh_ctx->vertices = (struct vector3 *)malloc(sizeof(struct vector3) * mesh_ctx->len);
	memset(mesh_ctx->vertices,0,sizeof(struct vector3) * mesh_ctx->len);

	mesh_ctx->border_ctx.border_cap = 16;
	mesh_ctx->border_ctx.border_offset = 0;
	mesh_ctx->border_ctx.borders = (struct nav_border *)malloc(sizeof(struct nav_border) * mesh_ctx->border_ctx.border_cap);
	memset(mesh_ctx->border_ctx.borders,0,sizeof(struct nav_border) * mesh_ctx->border_ctx.border_cap);

	mesh_ctx->size = p_cnt;
	mesh_ctx->node = (struct nav_node *)malloc(sizeof(struct nav_node) * mesh_ctx->size);
	memset(mesh_ctx->node,0,sizeof(struct nav_node) * mesh_ctx->size);

	mesh_ctx->mask_ctx.size = 8;
	mesh_ctx->mask_ctx.mask = (int*)malloc(sizeof(int) * mesh_ctx->mask_ctx.size);
	for(int i = 0;i < mesh_ctx->mask_ctx.size;i++)
		set_mask(&mesh_ctx->mask_ctx,i,0);
	set_mask(&mesh_ctx->mask_ctx,0,1);

	mesh_ctx->lt.x = mesh_ctx->lt.y = mesh_ctx->lt.z = 0;
	mesh_ctx->br.x = mesh_ctx->br.y = mesh_ctx->br.z = 0;
	//加载顶点
	int i,j,k;
	for (i = 0;i < v_cnt;i++)
	{
		mesh_ctx->vertices[i].x = v[i][0];
		mesh_ctx->vertices[i].y = v[i][1];
		mesh_ctx->vertices[i].z = v[i][2];

		if (mesh_ctx->lt.x == 0)
			mesh_ctx->lt.x = mesh_ctx->vertices[i].x;
		else
		{
			if (mesh_ctx->vertices[i].x < mesh_ctx->lt.x)
				mesh_ctx->lt.x = mesh_ctx->vertices[i].x;
		}

		if (mesh_ctx->lt.z == 0)
			mesh_ctx->lt.z = mesh_ctx->vertices[i].z;
		else
		{
			if (mesh_ctx->vertices[i].z < mesh_ctx->lt.z)
				mesh_ctx->lt.z = mesh_ctx->vertices[i].z;
		}

		if (mesh_ctx->br.x == 0)
			mesh_ctx->br.x = mesh_ctx->vertices[i].x;
		else
		{
			if (mesh_ctx->vertices[i].x > mesh_ctx->br.x)
				mesh_ctx->br.x = mesh_ctx->vertices[i].x;
		}

		if (mesh_ctx->br.z == 0)
			mesh_ctx->br.z = mesh_ctx->vertices[i].z;
		else
		{
			if (mesh_ctx->vertices[i].z > mesh_ctx->br.z)
				mesh_ctx->br.z = mesh_ctx->vertices[i].z;
		}
	}

	//加载多边形索引
	for (i = 0;i < p_cnt;i++)
	{
		struct nav_node* node = &mesh_ctx->node[i];
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
			center.x += mesh_ctx->vertices[node->poly[j-1]].x;
			center.y += mesh_ctx->vertices[node->poly[j-1]].y;
			center.z += mesh_ctx->vertices[node->poly[j-1]].z;
		}
		node->mask = p[i][node->size+1];
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

			struct nav_border* border0 = get_border(mesh_ctx, a, b);
			if (border0 == NULL)
			{
				add_border(mesh_ctx, a, b);
				border0 = get_border(mesh_ctx, a, b);
			}
			border_link_node(border0,node->id);

			node->border[k] = border0->id;
			
			struct nav_border* border1 = get_border(mesh_ctx, b, a);
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
		struct nav_border* border = get_border_with_id(mesh_ctx,i);
		for (int j = 0;j < mesh_ctx->border_ctx.border_offset;j++)
		{
			struct nav_border* tmp = get_border(mesh_ctx,border->b,border->a);
			if (tmp != NULL)
				border->opposite = tmp->id;
		}
	}

	make_tile(mesh_ctx);

	mesh_ctx->result.size = 8;
	mesh_ctx->result.offset = 0;
	mesh_ctx->result.wp = (struct vector3*)malloc(sizeof(struct vector3)*mesh_ctx->result.size);

	mesh_ctx->openlist = minheap_new(50 * 50, node_cmp);
	LIST_INIT((&mesh_ctx->closelist));
	LIST_INIT((&mesh_ctx->linked));

	return mesh_ctx;
}

void release_mesh(struct nav_mesh_context* ctx)
{
	free(ctx->vertices);
	free(ctx->border_ctx.borders);
	free(ctx->node);
	free(ctx->tile);
	free(ctx->mask_ctx.mask);
	minheap_delete(ctx->openlist);
}

bool raycast(struct nav_mesh_context* ctx,struct vector3* pt0,struct vector3* pt1,struct vector3* result)
{
	struct nav_node* node = get_node_with_pos(ctx,pt0->x,pt0->y,pt0->z);

	struct vector3 vt10;
	vector3_sub(pt1,pt0,&vt10);

	while (node)
	{
		if (in_node(ctx,node->id,pt1->x,pt1->y,pt1->z))
		{
			vector3_copy(result,pt1);
			return true;
		}

		bool not_cross = true;
		for (int i = 0;i < node->size;i++)
		{
			struct nav_border* border = get_border_with_id(ctx,node->border[i]);

			struct vector3* pt3 = &ctx->vertices[border->a];
			struct vector3* pt4 = &ctx->vertices[border->b];

			struct vector3 vt30,vt40;
			vector3_sub(pt3,pt0,&vt30);
			vector3_sub(pt4,pt0,&vt40);

			double direct_a = cross(&vt30,&vt10);
			double direct_b = cross(&vt40,&vt10);

			if (direct_a < 0 && direct_b > 0)
			{
				not_cross = false;
				int next = -1;
				if (border->node[0] !=-1)
				{
					if (border->node[0] == node->id)
						next = border->node[1];
					else
						next = border->node[0];
				}
				else
					assert(border->node[1] == node->id);
				
				if (next == -1)
				{
					cross_point(pt3,pt4,pt1,pt0,result);
					return true;
				}
				else
				{
					node = get_node(ctx,next);
					if (get_mask(&ctx->mask_ctx,node->mask) == 0)
					{
						cross_point(pt3,pt4,pt1,pt0,result);
						return true;
					}
					break;
				}
			}
		}
		if (not_cross)
			return false;
	}
	return false;
}


#define CLEAR_NODE(n) do  \
{\
	n->link_parent = NULL; \
	n->link_border = -1; \
	n->F = n->G = n->H = 0; \
	n->elt.index = 0; \
} while (false);


static inline void heap_clear(struct element* elt) 
{
	struct nav_node *n = (struct nav_node*)((int8_t*)elt - sizeof(struct list_node));
	CLEAR_NODE(n);
}

#define RESET(mesh_ctx) do \
{\
struct nav_node * n = NULL; \
	while ((n = (struct nav_node*)LIST_POP(&mesh_ctx->closelist))) {\
	\
	CLEAR_NODE(n);\
	}\
	minheap_clear(mesh_ctx->openlist, heap_clear); \
} while (false);



struct nav_node* next_border(struct nav_mesh_context* ctx, nav_node* node,struct vector3* wp,int *link_border)
{
	struct vector3 vt0,vt1;
	*link_border = node->link_border;
	while (*link_border != -1)
	{
		struct nav_border* border = get_border_with_id(ctx,*link_border);
		vector3_sub(&ctx->vertices[border->a],wp,&vt0);
		vector3_sub(&ctx->vertices[border->b],wp,&vt1);
		if ((vt0.x == 0 && vt0.z == 0) || (vt1.x == 0 && vt1.z == 0))
		{
			node = node->link_parent;
			*link_border = node->link_border;
		}
		else
			break;
	}
	if (*link_border != -1)
		return node;
	
	return NULL;
}

void result_init(struct nav_mesh_context* mesh_ctx)
{
	mesh_ctx->result.offset = 0;
}

void result_add(struct nav_mesh_context* mesh_ctx,struct vector3* wp)
{
	if (mesh_ctx->result.offset >= mesh_ctx->result.size)
	{
		int nsize = mesh_ctx->result.size * 2;
		struct vector3* owp = mesh_ctx->result.wp;
		mesh_ctx->result.wp = (struct vector3*)malloc(sizeof(struct vector3)*nsize);
		memcpy(mesh_ctx->result.wp,owp,mesh_ctx->result.size * sizeof(struct vector3));
		mesh_ctx->result.size = nsize;
		free(owp);
	}

	mesh_ctx->result.wp[mesh_ctx->result.offset].x = wp->x;
	mesh_ctx->result.wp[mesh_ctx->result.offset].z = wp->z;
	mesh_ctx->result.offset++;
}

void make_waypoint(struct nav_mesh_context* mesh_ctx,struct vector3* pt0,struct vector3* pt1,struct nav_node * node)
{
	result_add(mesh_ctx,pt1);

	struct vector3* pt_wp = pt1;

	int link_border = node->link_border;

	struct nav_border* border = get_border_with_id(mesh_ctx,link_border);

	struct vector3 pt_left,pt_right;
	vector3_copy(&pt_left,&mesh_ctx->vertices[border->a]);
	vector3_copy(&pt_right,&mesh_ctx->vertices[border->b]);

	struct vector3 vt_left,vt_right;
	vector3_sub(&pt_left,pt_wp,&vt_left);
	vector3_sub(&pt_right,pt_wp,&vt_right);

	struct nav_node* left_node = node->link_parent;
	struct nav_node* right_node = node->link_parent;

	struct nav_node* tmp = node->link_parent;
	while (tmp)
	{
		int link_border = tmp->link_border;
		if (link_border == -1)
		{
			struct vector3 tmp_target;
			tmp_target.x = pt0->x - pt_wp->x;
			tmp_target.z = pt0->z - pt_wp->z;

			double forward_a = cross(&vt_left,&tmp_target);
			double forward_b = cross(&vt_right,&tmp_target);

			if (forward_a < 0 && forward_b > 0)
			{
				result_add(mesh_ctx,pt0);
				break;
			}
			else
			{
				if (forward_a > 0 && forward_b > 0)
				{
					pt_wp->x = pt_left.x;
					pt_wp->z = pt_left.z;

					result_add(mesh_ctx,pt_wp);

					left_node = next_border(mesh_ctx,left_node,pt_wp,&link_border);
					if (left_node == NULL)
					{
						result_add(mesh_ctx,pt0);
						break;
					}
					
					border = get_border_with_id(mesh_ctx,link_border);
					pt_left.x = mesh_ctx->vertices[border->a].x;
					pt_left.z = mesh_ctx->vertices[border->a].z;

					pt_right.x = mesh_ctx->vertices[border->b].x;
					pt_right.z = mesh_ctx->vertices[border->b].z;

					vt_left.x = pt_left.x - pt_wp->x;
					vt_left.z = pt_left.z - pt_wp->z;

					vt_right.x = pt_right.x - pt_wp->x;
					vt_right.z = pt_right.z - pt_wp->z;

					tmp = left_node->link_parent;
					left_node = tmp;
					right_node = tmp;
					continue;
				}
				else if (forward_a < 0 && forward_b < 0)
				{
					pt_wp->x = pt_right.x;
					pt_wp->z = pt_right.z;

					result_add(mesh_ctx,pt_wp);

					right_node = next_border(mesh_ctx,right_node,pt_wp,&link_border);
					if (right_node == NULL)
					{
						result_add(mesh_ctx,pt0);
						break;
					}
					
					border = get_border_with_id(mesh_ctx,link_border);
					pt_left.x = mesh_ctx->vertices[border->a].x;
					pt_left.z = mesh_ctx->vertices[border->a].z;

					pt_right.x = mesh_ctx->vertices[border->b].x;
					pt_right.z = mesh_ctx->vertices[border->b].z;

					vt_left.x = pt_left.x - pt_wp->x;
					vt_left.z = pt_left.z - pt_wp->z;

					vt_right.x = pt_right.x - pt_wp->x;
					vt_right.z = pt_right.z - pt_wp->z;

					tmp = right_node->link_parent;
					left_node = tmp;
					right_node = tmp;
					continue;
				}
				break;
			}

		}

		border = get_border_with_id(mesh_ctx,link_border);

		struct vector3 tmp_pt_left,tmp_pt_right;
		vector3_copy(&tmp_pt_left,&mesh_ctx->vertices[border->a]);
		vector3_copy(&tmp_pt_right,&mesh_ctx->vertices[border->b]);

		struct vector3 tmp_vt_left,tmp_vt_right;
		vector3_sub(&tmp_pt_left,pt_wp,&tmp_vt_left);
		vector3_sub(&tmp_pt_right,pt_wp,&tmp_vt_right);

		double forward_left_a = cross(&vt_left,&tmp_vt_left);
		double forward_left_b = cross(&vt_right,&tmp_vt_left);
		double forward_right_a = cross(&vt_left,&tmp_vt_right);
		double forward_right_b = cross(&vt_right,&tmp_vt_right);

		if (forward_left_a < 0 && forward_left_b > 0)
		{
			left_node = tmp->link_parent;
			vector3_copy(&pt_left,&tmp_pt_left);
			vector3_sub(&pt_left,pt_wp,&vt_left);
		}

		if (forward_right_a < 0 && forward_right_b > 0)
		{
			right_node = tmp->link_parent;
			vector3_copy(&pt_right,&tmp_pt_right);
			vector3_sub(&pt_right,pt_wp,&vt_right);
		}

		if (forward_left_a > 0 && forward_left_b > 0 && forward_right_a > 0 && forward_right_b > 0)
		{
			vector3_copy(pt_wp,&pt_left);

			left_node = next_border(mesh_ctx,left_node,pt_wp,&link_border);
			if (left_node == NULL)
			{
				result_add(mesh_ctx,pt0);
				break;
			}
			
			border = get_border_with_id(mesh_ctx,link_border);
			vector3_copy(&pt_left,&mesh_ctx->vertices[border->a]);
			vector3_copy(&pt_right,&mesh_ctx->vertices[border->b]);

			vector3_sub(&mesh_ctx->vertices[border->a],pt_wp,&vt_left);
			vector3_sub(&mesh_ctx->vertices[border->b],pt_wp,&vt_right);

			result_add(mesh_ctx,pt_wp);

			tmp = left_node->link_parent;
			left_node = tmp;
			right_node = tmp;

			continue;
		}

		if (forward_left_a < 0 && forward_left_b < 0 && forward_right_a < 0 && forward_right_b < 0)
		{
			vector3_copy(pt_wp,&pt_right);

			right_node = next_border(mesh_ctx,right_node,pt_wp,&link_border);
			if (right_node == NULL)
			{
				result_add(mesh_ctx,pt0);
				break;
			}
			
			border = get_border_with_id(mesh_ctx,link_border);
			vector3_copy(&pt_left,&mesh_ctx->vertices[border->a]);
			vector3_copy(&pt_right,&mesh_ctx->vertices[border->b]);

			vector3_sub(&mesh_ctx->vertices[border->a],pt_wp,&vt_left);
			vector3_sub(&mesh_ctx->vertices[border->b],pt_wp,&vt_right);

			result_add(mesh_ctx,pt_wp);

			tmp = right_node->link_parent;
			left_node = tmp;
			right_node = tmp;
			continue;
		}

		tmp = tmp->link_parent;
	}
}

struct nav_path_context* astar_find(struct nav_mesh_context* mesh_ctx,struct vector3* pt0,struct vector3* pt1)
{
	result_init(mesh_ctx);

	struct nav_node* from = get_node_with_pos(mesh_ctx,pt0->x,pt0->y,pt0->z);
	struct nav_node* to = get_node_with_pos(mesh_ctx,pt1->x,pt1->y,pt1->z);

	if (!from || !to || from == to)
		return NULL;

	minheap_push(mesh_ctx->openlist,&from->elt);

	struct nav_node* current = NULL;
	for (;;)
	{
		struct element* elt = minheap_pop(mesh_ctx->openlist);
		if (!elt)
		{
			RESET((mesh_ctx));
			return NULL;
		}
		current = (struct nav_node*)((int8_t*)elt - sizeof(struct list_node));
		if (current == to)
		{
			make_waypoint(mesh_ctx,pt0,pt1,current);
			RESET((mesh_ctx));
			CLEAR_NODE(current);
			return &mesh_ctx->result;
		}

		LIST_PUSH((&mesh_ctx->closelist),((struct list_node*)current));

		struct list* linked = get_link(mesh_ctx,current);
		if (linked)
		{
			struct nav_node* linked_node;
			while ((linked_node = (struct nav_node*)LIST_POP(linked)))
			{
				if (linked_node->elt.index)
				{
					double nG = current->G + get_cost(current,linked_node);
					if (nG < linked_node->G)
					{
						linked_node->G = nG;
						linked_node->F = linked_node->G + linked_node->H;
						linked_node->link_parent = current;
						linked_node->link_border = linked_node->reserve;
						MINHEAP_CHANGE(mesh_ctx->openlist, (&linked_node->elt));
					}
				}
				else
				{
					linked_node->G = current->G + get_cost(current,linked_node);
					linked_node->H = get_cost(linked_node,to);
					linked_node->F = linked_node->G + linked_node->H;
					assert(linked_node->link_border == -1);
					assert(linked_node->link_parent == NULL);
					linked_node->link_parent = current;
					linked_node->link_border = linked_node->reserve;
					minheap_push(mesh_ctx->openlist, &linked_node->elt);
				}
			}
		}
	}
}
