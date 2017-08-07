#include "nav.h"

static inline void heap_clear(struct element* elt) 
{
	struct NavNode *n = (struct NavNode*)((int8_t*)elt - sizeof(struct list_node));
	n->link_parent = NULL;
	n->link_border = -1;
	n->F = n->G = n->H = 0;
	n->elt.index = 0;
}

#define RESET(mesh_ctx) do \
{\
struct NavNode * n = NULL; \
	while ((n = (struct NavNode*)LIST_POP(&mesh_ctx->closelist))) {\
	\
	n->G = n->H = n->F = 0; \
	}\
	minheap_clear(mesh_ctx->openlist, heap_clear); \
} while (false);

#define CLEAR_NODE(n) do  \
{\
	n->link_parent = NULL; \
	n->link_border = NULL; \
	n->F = n->G = n->H = 0; \
	n->elt.index = 0; \
} while (false);

void node_reset(struct NavNode* node)
{
	CLEAR_NODE(node);
}


struct vector3* make_waypoint(struct MeshContext* mesh_ctx,struct vector3* pt0,struct vector3* pt1,struct NavNode * node,int* size)
{
	struct vector3* result = (struct vector3*)malloc(sizeof(struct vector3) * 100);
	int index = 0;
	

	struct vector3* pt_wp = pt1;

	result[index].x = pt_wp->x;
	result[index].z = pt_wp->z;
	index++;

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
				result[index].x = pt0->x;
				result[index].z = pt0->z;
				index++;
				break;
			}
			else
			{
				if (forward_a > 0 && forward_b > 0)
				{
					pt_wp->x = pt_left.x;
					pt_wp->z = pt_left.z;

					result[index].x = pt_wp->x;
					result[index].z = pt_wp->z;
					index++;

					link_border = left_node->link_border;
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

					result[index].x = pt_wp->x;
					result[index].z = pt_wp->z;
					index++;

					link_border = right_node->link_border;
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

			link_border = left_node->link_border;
			border = get_border_with_id(mesh_ctx,link_border);

			vector3_copy(&pt_left,&mesh_ctx->vertices[border->a]);
			vector3_copy(&pt_right,&mesh_ctx->vertices[border->b]);

			vector3_sub(&mesh_ctx->vertices[border->a],pt_wp,&vt_left);
			vector3_sub(&mesh_ctx->vertices[border->b],pt_wp,&vt_right);

			result[index].x = pt_wp->x;
			result[index].z = pt_wp->z;
			index++;

			tmp = left_node->link_parent;
			left_node = tmp;
			right_node = tmp;

			continue;
		}

		if (forward_left_a < 0 && forward_left_b < 0 && forward_right_a < 0 && forward_right_b < 0)
		{
			vector3_copy(pt_wp,&pt_right);

			link_border = right_node->link_border;
			border = get_border_with_id(mesh_ctx,link_border);

			vector3_copy(&pt_left,&mesh_ctx->vertices[border->a]);
			vector3_copy(&pt_right,&mesh_ctx->vertices[border->b]);

			vector3_sub(&mesh_ctx->vertices[border->a],pt_wp,&vt_left);
			vector3_sub(&mesh_ctx->vertices[border->b],pt_wp,&vt_right);

			result[index].x = pt_wp->x;
			result[index].z = pt_wp->z;
			index++;

			tmp = right_node->link_parent;
			left_node = tmp;
			right_node = tmp;
			continue;
		}

		tmp = tmp->link_parent;
	}
	*size = index;
	return result;
}

struct NavNode* astar_poly_path(struct MeshContext* mesh_ctx,struct vector3* pt0,struct vector3* pt1,struct vector3*&result,int * size)
{
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
			result = make_waypoint(mesh_ctx,pt0,pt1,current,size);
			RESET((mesh_ctx));
			return current;
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
					linked_node->link_parent = current;
					linked_node->link_border = linked_node->reserve;
					minheap_push(mesh_ctx->openlist, &linked_node->elt);
				}
			}
		}
	}
}

bool raycast(struct MeshContext* mesh_ctx,int polyId,struct vector3* pt0,struct vector3* pt1,struct vector3* result)
{
	struct NavNode* navNode = find_node(mesh_ctx,polyId);

	struct vector3 vt10;
	vt10.x = pt1->x - pt0->x;
	vt10.y = 0;
	vt10.z = pt1->z - pt0->z;

	while (navNode)
	{
		if (in_node_ex(mesh_ctx,navNode->id,pt1->x,pt1->y,pt1->z))
		{
			result->x = pt1->x;
			result->y = 0;
			result->z = pt1->z;
			return true;
		}

		for (int i =0;i < navNode->size;i++)
		{
			struct Border* border = get_border_with_id(mesh_ctx,navNode->border[i]);

			struct vector3* pt3 = &mesh_ctx->vertices[border->a];
			struct vector3* pt4 = &mesh_ctx->vertices[border->b];

			struct vector3 vt30;
			vt30.x = pt3->x - pt0->x;
			vt30.y = 0;
			vt30.z = pt3->z - pt0->z;

			struct vector3 vt40;
			vt40.x = pt4->x - pt0->x;
			vt40.y = 0;
			vt40.z = pt4->z - pt0->z;

			double forwarda = cross(&vt30,&vt10);
			double forwardb = cross(&vt40,&vt10);

			if (forwarda < 0 && forwardb > 0)
			{
				int next = -1;
				if (border->node[0] !=-1)
				{
					if (border->node[0] == navNode->id)
						next = border->node[1];
					else
						next = border->node[0];
				}
				else
				{
					if (border->node[1] != -1)
					{
						if (border->node[1] != navNode->id)
							next = border->node[1];
					}
				}
				if (next == -1)
				{
					cross_pt(pt3,pt4,pt1,pt0,result);
					return true;
				}
				else
					navNode = find_node(mesh_ctx,next);
				break;
			}
		}
	}
	return false;
}