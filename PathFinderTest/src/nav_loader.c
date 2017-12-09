#include "nav.h"

struct nav_border_searcher
{
	struct nav_border_searcher* next;
	int index;
	int id;
};


static int 
node_cmp(struct element * left, struct element * right) {
	struct nav_node *l = cast_node(left);
	struct nav_node *r = cast_node(right);
	return l->F < r->F;
}

int 
vertex_cmp(const void * left,const void * right) {
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

struct nav_border* 
add_border(struct nav_mesh_context* mesh_ctx, int a, int b) {
	struct nav_border_context * border_ctx = &mesh_ctx->border_ctx;
	if (border_ctx->border_offset + 1 >= border_ctx->border_cap) {
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
	border->center.x = (mesh_ctx->vertices[a].x + mesh_ctx->vertices[b].x);
	border->center.y = (mesh_ctx->vertices[a].y + mesh_ctx->vertices[b].y);
	border->center.z = (mesh_ctx->vertices[a].z + mesh_ctx->vertices[b].z);

	border_ctx->border_offset++;

	return border;
}

struct nav_border* 
search_border(struct nav_mesh_context* ctx,struct nav_border_searcher** searcher,int begin,int end) {
	if (begin >= ctx->len || end >= ctx->len)
		return NULL;

	struct nav_border_searcher* node = searcher[begin];
	while(node != NULL) {
		if (node->index == end)
			return get_border(ctx,node->id);
		node = node->next;
	}

	node =  (struct nav_border_searcher*)malloc(sizeof(*node));
	struct nav_border* border = add_border(ctx,begin,end);
	node->id = border->id;
	node->index = end;
	node->next = searcher[begin];
	searcher[begin] = node;
	return border;
}

void 
border_link_node(struct nav_border* border,int id) {
	if (border->node[0] == -1)
		border->node[0] = id;
	else if (border->node[1] == -1)
		border->node[1] = id;
	else
		assert(0);
}

void 
release_border_searcher(struct nav_mesh_context* ctx,struct nav_border_searcher** searcher) {
	int i;
	for(i = 0;i < ctx->len;i++) {
		struct nav_border_searcher* node = searcher[i];
		while(node != NULL) {
			struct nav_border_searcher* tmp = node;
			node = node->next;
			free(tmp);
		}
	}
	free(searcher);
}

void 
vertex_sort(struct nav_mesh_context* ctx,struct nav_node* node) {
	struct vertex_sort_info* vertex = (struct vertex_sort_info*)malloc(sizeof(*vertex) * node->size);
	int i;
	for (i = 0;i < node->size;i++) {
		vertex[i].ctx = ctx;
		vertex[i].index = node->poly[i];
		vector3_copy(&vertex[i].center,&node->center);
	}

	qsort(vertex,node->size,sizeof(struct vertex_sort_info),vertex_cmp);

	for (i = 0;i < node->size;i++)
		node->poly[i] = vertex[i].index;

	free(vertex);
}

void 
load_mesh_done(struct nav_mesh_context* mesh_ctx) {
	mesh_ctx->mask_ctx.size = 8;
	mesh_ctx->mask_ctx.mask = (int*)malloc(sizeof(int) * mesh_ctx->mask_ctx.size);
	int i;
	for(i = 0;i < mesh_ctx->mask_ctx.size;i++)
		set_mask(&mesh_ctx->mask_ctx,i,1);
	set_mask(&mesh_ctx->mask_ctx,0,1);

	mesh_ctx->result.size = 8;
	mesh_ctx->result.offset = 0;
	mesh_ctx->result.wp = (struct vector3*)malloc(sizeof(struct vector3)*mesh_ctx->result.size);

	mesh_ctx->openlist = minheap_create(50 * 50, node_cmp);
	list_init(&mesh_ctx->closelist);
	list_init(&mesh_ctx->linked);
}

struct nav_mesh_context* 
load_mesh(double** v,int v_cnt,int** p,int p_cnt) {
	struct nav_mesh_context* mesh_ctx = (struct nav_mesh_context*)malloc(sizeof(*mesh_ctx));
	memset(mesh_ctx,0,sizeof(*mesh_ctx));

	mesh_ctx->len = v_cnt;
	mesh_ctx->vertices = (struct vector3 *)malloc(sizeof(struct vector3) * mesh_ctx->len);
	memset(mesh_ctx->vertices,0,sizeof(struct vector3) * mesh_ctx->len);

	mesh_ctx->border_ctx.border_cap = 64;
	mesh_ctx->border_ctx.border_offset = 0;
	mesh_ctx->border_ctx.borders = (struct nav_border *)malloc(sizeof(struct nav_border) * mesh_ctx->border_ctx.border_cap);
	memset(mesh_ctx->border_ctx.borders,0,sizeof(struct nav_border) * mesh_ctx->border_ctx.border_cap);

	struct nav_border_searcher** border_searcher = (struct nav_border_searcher**)malloc(sizeof(*border_searcher) * mesh_ctx->len);
	memset(border_searcher,0,sizeof(*border_searcher) * mesh_ctx->len);

	mesh_ctx->size = p_cnt;
	mesh_ctx->node = (struct nav_node *)malloc(sizeof(struct nav_node) * mesh_ctx->size);
	memset(mesh_ctx->node,0,sizeof(struct nav_node) * mesh_ctx->size);

	mesh_ctx->lt.x = mesh_ctx->lt.y = mesh_ctx->lt.z = 0;
	mesh_ctx->br.x = mesh_ctx->br.y = mesh_ctx->br.z = 0;
	//¼ÓÔØ¶¥µã,ÕÒ³öµØÍ¼µÄ×óÉÏºÍÓÒÏÂµÄÁ½¸öÏîµã
	int i,j,k;
	for (i = 0;i < v_cnt;i++) {
		mesh_ctx->vertices[i].x = v[i][0];
		mesh_ctx->vertices[i].y = v[i][1];
		mesh_ctx->vertices[i].z = v[i][2];

		if (mesh_ctx->lt.x == 0)
			mesh_ctx->lt.x = mesh_ctx->vertices[i].x;
		else {
			if (mesh_ctx->vertices[i].x < mesh_ctx->lt.x)
				mesh_ctx->lt.x = mesh_ctx->vertices[i].x;
		}

		if (mesh_ctx->lt.z == 0)
			mesh_ctx->lt.z = mesh_ctx->vertices[i].z;
		else {
			if (mesh_ctx->vertices[i].z < mesh_ctx->lt.z)
				mesh_ctx->lt.z = mesh_ctx->vertices[i].z;
		}

		if (mesh_ctx->br.x == 0)
			mesh_ctx->br.x = mesh_ctx->vertices[i].x;
		else {
			if (mesh_ctx->vertices[i].x > mesh_ctx->br.x)
				mesh_ctx->br.x = mesh_ctx->vertices[i].x;
		}

		if (mesh_ctx->br.z == 0)
			mesh_ctx->br.z = mesh_ctx->vertices[i].z;
		else {
			if (mesh_ctx->vertices[i].z > mesh_ctx->br.z)
				mesh_ctx->br.z = mesh_ctx->vertices[i].z;
		}
	}

	mesh_ctx->width = mesh_ctx->br.x - mesh_ctx->lt.x;
	mesh_ctx->heigh = mesh_ctx->br.z - mesh_ctx->lt.z;

	//¼ÓÔØ¶à±ßÐÎË÷Òý
	for (i = 0;i < p_cnt;i++) {
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

		for (j = 1;j <= node->size;j++) {
			node->poly[j-1] = p[i][j];
			center.x += mesh_ctx->vertices[node->poly[j-1]].x;
			center.y += mesh_ctx->vertices[node->poly[j-1]].y;
			center.z += mesh_ctx->vertices[node->poly[j-1]].z;
		}
		node->mask = p[i][node->size+1];
		node->center.x = center.x / node->size;
		node->center.y = center.y / node->size;
		node->center.z = center.z / node->size;

		//¶¥µãË³Ê±ÕëÅÅÐò
		vertex_sort(mesh_ctx,node);

		//Í¬Ê±Éú³ÉË³Ê±ÕëºÍÄæÊ±ÕëµÄ¶à±ßÐÎµÄ±ßborder,²¢¼ÇÂ¼±ßµÄÁ½±ß¶à±ßÐÎ
		for (k = 0; k < node->size;k++) {
			int k0 = k;
			int k1 = k + 1 >= node->size ? 0 : k + 1;
			
			int a = node->poly[k0];
			int b = node->poly[k1];

			struct nav_border* border = search_border(mesh_ctx,border_searcher, a, b);
			border_link_node(border,node->id);
			
			int border_id = border->id;
			node->border[k] = border_id;

			struct nav_border* border_opposite = search_border(mesh_ctx,border_searcher, b, a);
			border_link_node(border_opposite, node->id);
			border_opposite->opposite = border_id;
			
			border = get_border(mesh_ctx,border_id);
			border->opposite = border_opposite->id;
		}
	}

	release_border_searcher(mesh_ctx,border_searcher);

	load_mesh_done(mesh_ctx);

	return mesh_ctx;
}


void 
release_mesh(struct nav_mesh_context* ctx) {
	free(ctx->vertices);
	free(ctx->border_ctx.borders);
	int i;
	for(i = 0;i < ctx->size;i++) {
		if (ctx->node[i].border != NULL)
			free(ctx->node[i].border);
		if (ctx->node[i].poly != NULL)
			free(ctx->node[i].poly);
	}
	free(ctx->node);
	free(ctx->mask_ctx.mask);
	free(ctx->result.wp);
	minheap_release(ctx->openlist);
	if (ctx->tile != NULL) {
		release_tile(ctx,ctx->tile);
	}
	free(ctx);
}
