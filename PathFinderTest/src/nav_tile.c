#include "nav.h"

void 
add_node(struct nav_tile* tile,int index) {
	if (tile->size == 0) {
		tile->offset = 0;
		tile->size = 1;
		tile->node = (int*)malloc(sizeof(int) * tile->size);
	}
	if (tile->offset >= tile->size) {
		int nsize = tile->size * 2;
		int* onode = tile->node;
		tile->node = (int*)malloc(sizeof(int) * nsize);
		memcpy(tile->node,onode,sizeof(int)* tile->size);
		tile->size = nsize;
		free(onode);
	}
	tile->node[tile->offset] = index;
	tile->offset++;
}

int
intersect(struct vector3* a,struct vector3* b,struct vector3* c,struct vector3* d) {
	if (max(a->x,b->x) >= min(c->x,d->x) &&
		max(a->z,b->z) >= min(c->z,d->z) && 
		max(c->x,d->x) >= min(a->x,b->x) && 
		max(c->z,d->z) >= min(a->z,b->z)) {
		struct vector3 ac,dc,bc,ca,ba,da;
		vector3_sub(a,c,&ac);
		vector3_sub(d,c,&dc);
		vector3_sub(b,c,&bc);
		vector3_sub(c,a,&ca);
		vector3_sub(b,a,&ba);
		vector3_sub(d,a,&da);

		if (cross(&ac,&dc) * cross(&dc,&bc) >= 0) {
			if (cross(&ca,&ba) * cross(&ba,&da) >= 0)
				return true;
		}
	}
	
	return false;
}

struct nav_tile* 
create_tile(struct nav_mesh_context* ctx) {
	int count = ctx->width * ctx->heigh;
	struct nav_tile* navtile = (struct nav_tile*)malloc(sizeof(struct nav_tile)*count);
	memset(navtile,0,sizeof(struct nav_tile)*count);

	int z;
	for (z = 0;z < ctx->heigh;z++) {
		int x;
		for (x = 0;x < ctx->width;x++) {
			int index = x + z * ctx->width;
			struct nav_tile* tile = &navtile[index];
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

	int i;
	for (i = 0;i < count;i++) {
		struct nav_tile* tile = &navtile[i];
		int j;
		for (j = 0;j < ctx->size;j++) {
			int done = false;
			struct nav_node* node = &ctx->node[j];
			int k;
			for (k = 0;k < 4;k++) {
				int l;
				for (l = 0;l < node->size;l++) {
					struct nav_border* border = get_border(ctx,node->border[l]);
					if (intersect(&tile->pos[k], &tile->pos[(k + 1) % 4], &ctx->vertices[border->a], &ctx->vertices[border->b])) {
						done = true;
						break;
					}
				}
				if (done)
					break;
			}
			
			if (done)
				add_node(tile,j);
			else {
				if (inside_node(ctx, j, tile->center.x, tile->center.y, tile->center.z))
					add_node(tile,j);
			}
		}
	}
	return navtile;
}


void 
release_tile(struct nav_mesh_context* ctx,struct nav_tile* navtile) {
	int count = ctx->width * ctx->heigh;
	int i;
	for (i = 0;i < count;i++) {
		if (navtile[i].node != NULL)
			free(navtile[i].node);
	}
	free(navtile);
}