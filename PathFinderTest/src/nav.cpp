#include "nav.h"


#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))


void set_mask(struct MeshMask* ctx,int mask,int enable)
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

int get_mask(struct MeshMask* ctx,int mask)
{
	return ctx->mask[mask];
}

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

bool intersect(struct vector3* a,struct vector3* b,struct vector3* c,struct vector3* d)
{
	//�����ų�ʵ��,�����ཻ
	if (max(a->x,b->x) >= min(c->x,d->x) &&
		max(a->z,b->z) >= min(c->z,d->z) && 
		max(c->x,d->x) >= min(a->x,b->x) && 
		max(c->x,d->x) >= min(a->x,b->x))
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

bool in_node(struct MeshContext* mesh_ctx,int polyId,double x,double y,double z)
{
	struct NavNode* NavNode = &mesh_ctx->node[polyId];
	struct vector3 vt;
	vt.x = x;
	vt.y = y;
	vt.z = z;
	return in_poly(mesh_ctx,NavNode->poly,NavNode->size,&vt);
}

struct NavNode* find_node(struct MeshContext* mesh_ctx,int id)
{
	if (id < 0 || id >= mesh_ctx->size)
		return NULL;
	return &mesh_ctx->node[id];
}

struct NavNode* find_node_with_pos(struct MeshContext* ctx,double x,double y,double z)
{
	//��������
	//for (int i = 0; i < ctx->size;i++)
	//{
	//	if (in_node(ctx,i,x,y,z))
	//		return &ctx->node[i];
	//}

	//���ø��ӿ��ٲ���
	int x_index = x - ctx->lt.x;
	int z_index = z - ctx->lt.z;
	int index = x_index + z_index * ctx->width;
	struct Tile* tile = &ctx->tile[index];
	for (int i = 0;i < tile->offset;i++)
	{
		if (in_node(ctx,tile->node[i],x,y,z))
			return &ctx->node[tile->node[i]];
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

double get_cost(struct NavNode* from,struct NavNode* to)
{
	double dx = from->center.x - to->center.x;
	double dy = from->center.y - to->center.y;
	double dz = from->center.z - to->center.z;
	return sqrt(dx*dx + dy* dy + dz* dz);
}

int node_cmp(struct element * left, struct element * right) 
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

void tile_add_node(struct Tile* tile,int index)
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

//�зָ�����Ϣ
void make_tile(struct MeshContext* ctx)
{
	ctx->width = ctx->br.x - ctx->lt.x;
	ctx->heigh = ctx->br.z - ctx->lt.z;
 
	int count = ctx->width * ctx->heigh;
	ctx->tile = (struct Tile*)malloc(sizeof(struct Tile)*count);
	memset(ctx->tile,0,sizeof(struct Tile)*count);

	for (int z = 0;z < ctx->heigh;z++)
	{
		for (int x = 0;x < ctx->width;x++)
		{
			int index = x + z * ctx->width;
			struct Tile* tile = &ctx->tile[index];
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
		struct Tile* tile = &ctx->tile[i];
		
		for (int j = 0;j < 4;j++)
		{
			for (int k = 0;k < ctx->size;k++)
			{
				struct NavNode* node = &ctx->node[k];
				int cross_cnt = 0;
				for (int l = 0;l < node->size;l++)
				{
					struct Border* border = get_border_with_id(ctx,node->border[l]);
					if (intersect(&tile->pos[j],&tile->pos[(j+1)%4],&ctx->vertices[border->a],&ctx->vertices[border->b]))
						cross_cnt++;
				}
				if (cross_cnt > 0)
				{
					if (cross_cnt >= 2)
						tile_add_node(tile,k);
					else
					{
						for (int m = 0;m < 4;m++)
						{
							if (in_node(ctx,k,tile->pos[m].x,tile->pos[m].y,tile->pos[m].z))
							{
								tile_add_node(tile,k);
								break;
							}
						}
					}
				}
				else
				{
					if (in_node(ctx,k,tile->center.x,tile->center.y,tile->center.z))
						tile_add_node(tile,k);
				}

			}
		}
	}

	for (int i = 0;i < count;i++)
	{
		struct Tile* tile = &ctx->tile[i];
		if (tile->node == NULL)
			tile->mask = -1;
		else
		{
			int mask_max = 0;
			for (int j = 0;j < tile->offset;j++)
			{
				struct NavNode* node = find_node(ctx,tile->node[j]);
				if (node->mask > mask_max)
					mask_max = node->mask;
			}
			tile->mask = mask_max;
		}
	}
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

	mesh_ctx->mask_ctx.mask = (int*)malloc(sizeof(int) * 8);
	mesh_ctx->mask_ctx.size = 8;
	for(int i = 0;i < 8;i++)
		set_mask(&mesh_ctx->mask_ctx,i,0);
	set_mask(&mesh_ctx->mask_ctx,0,1);

	mesh_ctx->lt.x = mesh_ctx->lt.y = mesh_ctx->lt.z = 0;
	mesh_ctx->br.x = mesh_ctx->br.y = mesh_ctx->br.z = 0;
	//���ض���
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

	//���ض��������
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
			center.x += mesh_ctx->vertices[node->poly[j-1]].x;
			center.y += mesh_ctx->vertices[node->poly[j-1]].y;
			center.z += mesh_ctx->vertices[node->poly[j-1]].z;
		}
		node->mask = p[i][node->size+1];
		node->center.x = center.x / node->size;
		node->center.y = center.y / node->size;
		node->center.z = center.z / node->size;

		//����˳ʱ������
		vertex_sort(mesh_ctx,node);

		//ͬʱ����˳ʱ�����ʱ��Ķ���εı�border,����¼�ߵ����߶����
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

	//��¼ÿ���߷����򶥵�ı�
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

	make_tile(mesh_ctx);

	mesh_ctx->openlist = minheap_new(50 * 50, node_cmp);
	LIST_INIT((&mesh_ctx->closelist));
	LIST_INIT((&mesh_ctx->linked));

	mesh_ctx->result.size = 8;
	mesh_ctx->result.offset = 0;
	mesh_ctx->result.wp = (struct vector3*)malloc(sizeof(struct vector3)*8);
	return mesh_ctx;
}

bool raycast(struct MeshContext* ctx,struct vector3* pt0,struct vector3* pt1,struct vector3* result)
{
	struct NavNode* node = find_node_with_pos(ctx,pt0->x,pt0->y,pt0->z);

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
			struct Border* border = get_border_with_id(ctx,node->border[i]);

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
					cross_pt(pt3,pt4,pt1,pt0,result);
					return true;
				}
				else
				{
					node = find_node(ctx,next);
					if (get_mask(&ctx->mask_ctx,node->mask) == 0)
					{
						cross_pt(pt3,pt4,pt1,pt0,result);
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
	struct NavNode *n = (struct NavNode*)((int8_t*)elt - sizeof(struct list_node));
	CLEAR_NODE(n);
}

#define RESET(mesh_ctx) do \
{\
struct NavNode * n = NULL; \
	while ((n = (struct NavNode*)LIST_POP(&mesh_ctx->closelist))) {\
	\
	CLEAR_NODE(n);\
	}\
	minheap_clear(mesh_ctx->openlist, heap_clear); \
} while (false);



struct NavNode* next_border(struct MeshContext* ctx, NavNode* node,struct vector3* wp,int *link_border)
{
	struct vector3 vt0,vt1;
	*link_border = node->link_border;
	while (*link_border != -1)
	{
		struct Border* border = get_border_with_id(ctx,*link_border);
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

void result_init(struct MeshContext* mesh_ctx)
{
	mesh_ctx->result.offset = 0;
}

void result_add(struct MeshContext* mesh_ctx,struct vector3* wp)
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

void make_waypoint(struct MeshContext* mesh_ctx,struct vector3* pt0,struct vector3* pt1,struct NavNode * node)
{
	result_add(mesh_ctx,pt1);

	struct vector3* pt_wp = pt1;

	int link_border = node->link_border;

	struct Border* border = get_border_with_id(mesh_ctx,link_border);

	struct vector3 pt_left,pt_right;
	vector3_copy(&pt_left,&mesh_ctx->vertices[border->a]);
	vector3_copy(&pt_right,&mesh_ctx->vertices[border->b]);

	struct vector3 vt_left,vt_right;
	vector3_sub(&pt_left,pt_wp,&vt_left);
	vector3_sub(&pt_right,pt_wp,&vt_right);

	struct NavNode* left_node = node->link_parent;
	struct NavNode* right_node = node->link_parent;

	struct NavNode* tmp = node->link_parent;
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

struct PathContext* astar_find(struct MeshContext* mesh_ctx,struct vector3* pt0,struct vector3* pt1)
{
	result_init(mesh_ctx);

	struct NavNode* from = find_node_with_pos(mesh_ctx,pt0->x,pt0->y,pt0->z);
	struct NavNode* to = find_node_with_pos(mesh_ctx,pt1->x,pt1->y,pt1->z);

	if (!from || !to || from == to)
		return NULL;

	minheap_push(mesh_ctx->openlist,&from->elt);

	struct NavNode* current = NULL;
	for (;;)
	{
		struct element* elt = minheap_pop(mesh_ctx->openlist);
		if (!elt)
		{
			RESET((mesh_ctx));
			return NULL;
		}
		current = (struct NavNode*)((int8_t*)elt - sizeof(struct list_node));
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
			struct NavNode* linked_node;
			while ((linked_node = (struct NavNode*)LIST_POP(linked)))
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
