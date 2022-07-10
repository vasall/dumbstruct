#include "christree.h"

#include "../../alarm/inc/alarm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


DBS_API struct dbs_christree *dbs_christree_init(s32 lim)
{
	struct dbs_christree *tree;
	s32 tmp;
	s32 i;

	/*
	 * Allocate memory for the tree.
	 */
	if(!(tree = malloc(sizeof(struct dbs_christree))))
		goto err_return;

	/*
	 * Allocate memory for the root node and initialize it.
	 * Also preallocate memory for the first few few branch nodes.
	 */

	if(!(tree->root = dbs_christree_new(-1, 0)))
		goto err_free_tree;

	/*
	 * Create and initialize the layers.
	 */
	tree->layer_num = lim;
	tmp = tree->layer_num * sizeof(struct dbs_christree_layer);
	if(!(tree->layer = malloc(tmp)))
		goto err_del_root;

	for(i = 0; i < tree->layer_num; i++) {
		tree->layer[i].layer_num = i;
		tree->layer[i].node = NULL;
		tree->layer[i].node_num = 0;
		tree->layer[i].count = 0;
	}

	return tree;

err_del_root:
	dbs_christree_del(tree->root);

err_free_tree:
	free(tree);

err_return:
	ALARM(ALARM_ERR, "Failed to create stress tree");
	return NULL;
}


DBS_API void dbs_christree_close(struct dbs_christree *tree)
{
	if(!tree) {
		ALARM(ALARM_WARN, "tree undefined");
		return;
	}

	/*
	 * Remove all nodes from the tree, except the root node. 
	 */


	/*
	 * Free the layers list.
	 */
	free(tree->layer);

	/*
	 * Delete the root node.
	 */
	dbs_christree_del(tree->root);

	/*
	 * Free the table struct.
	 */
	free(tree);
}


DBS_API struct dbs_christree_node *dbs_christree_new(s32 layer, u8 dif)
{
	struct dbs_christree_node *node;
	s32 tmp;
	s32 i;

	/*
	 * Allocate memory for the new node.
	 */
	if(!(node = malloc(sizeof(struct dbs_christree_node))))
		goto err_return;

	node->layer = layer;
	node->dif = dif;
	node->data = NULL;

	node->h_v_prev = NULL;
	node->h_v_next = NULL;

	node->v_prev = NULL;

	node->v_next_used = 0;
	node->v_next_alloc = DBS_CHRISTREE_NEXT_MIN;

	/*
	 * Preallocate memory for the v_next nodes and reset the pointers.
	 */

	tmp = node->v_next_alloc * sizeof(struct dbs_christree_node *);
	if(!(node->v_next = malloc(tmp)))
		goto err_free_node;

	for(i = 0; i < node->v_next_alloc; i++)
		node->v_next[i] = NULL;

	return node;


err_free_node:
	free(node);

err_return:
	ALARM(ALARM_ERR, "Failed to create stress tree node");
	return NULL;
}


DBS_API void dbs_christree_del(struct dbs_christree_node *node)
{
	if(!node) {
		ALARM(ALARM_WARN, "node undefined");
		return;
	}

	free(node->v_prev);

	free(node->v_next);

	free(node);
}


DBS_API s32 dbs_christree_get_layer(struct dbs_christree *tree,
		s32 layer, u8 dif, struct dbs_christree_node **lst, s32 lim)
{
	struct dbs_christree_node *n_ptr;
	s32 c = 0;

	if(!tree || !lst || lim < 1) {
		ALARM(ALARM_WARN, "layer or lst undefined or lim invalid");
		return -1;
	}

	n_ptr = tree->layer[layer].node;
	while(n_ptr) {
		if(n_ptr->dif == dif) {
			lst[c] = n_ptr;
			c++;
		}

		if(c >= lim)
			break;

		n_ptr = n_ptr->h_v_next;
	}

	return c;
}


DBS_API s8 dbs_christree_add_v_prev(struct dbs_christree_node *node,
		struct dbs_christree_node *v_prev)
{
	if(!node || !v_prev) {
		ALARM(ALARM_WARN, "node or v_prev undefined");
		return -1;
	}

	node->v_prev = v_prev;

	return 0;
}


DBS_API void dbs_christree_rmv_v_prev(struct dbs_christree_node *node)
{
	if(!node) {
		ALARM(ALARM_WARN, "node undefined");
		return;
	}

	node->v_prev = NULL;
}


DBS_API s8 dbs_christree_add_v_next(struct dbs_christree_node *node,
		struct dbs_christree_node *v_next)
{
	struct dbs_christree_node **p;
	s32 alloc;
	s32 tmp;
	s32 i;
	s32 j;

	if(!node || !v_next) {
		ALARM(ALARM_WARN, "node or v_next undefined");
		return -1;
	}

	/*
	 * Check if there's space left in the v_next list, and if not, allocate
	 * more.
	 */
	if(node->v_next_used + 1 >= node->v_next_alloc) {
		alloc = node->v_next_alloc * 2;
		tmp = alloc * sizeof(struct dbs_christree_node *);
		if(!(p = realloc(node->v_next, tmp)))
			goto err_return;

		for(i = node->v_next_alloc; i < alloc; i++)
			p[i] = NULL;

		node->v_next = p;
		node->v_next_alloc = alloc;
	}

	for(i = 0; i < node->v_next_alloc; i++) {
		/*
		 * If there's a free slot, use it.
		 */
		if(node->v_next[i] == NULL) {

			node->v_next[i] = v_next;
			node->v_next_used++;
			return 0;
		}

		/*
		 * If the v_next element is bigger than the one we want to insert,
		 * then move all the following ones a slot back and overwrite
		 * the slot with the given node.
		 */
		if(node->v_next[i]->dif > v_next->dif) {
			for(j = node->v_next_used; j > i ; j--)
				node->v_next[j] = node->v_next[j - 1];

			node->v_next[i] = v_next;
			node->v_next_used++;

			return 0;
		}
	}

err_return:
	ALARM(ALARM_ERR, "Failed to link v_next element");
	return -1;
}


DBS_API void dbs_christree_rmv_v_next(struct dbs_christree_node *node,
		struct dbs_christree_node *v_next)
{
	s32 i;
	s32 j;

	if(!node || !v_next) {
		ALARM(ALARM_WARN, "node or v_next undefined");
		return;
	}

	for(i = 0; i < node->v_next_used; i++) {
		if(node->v_next[i]->dif == v_next->dif) {
			/*
			 * Move all pointers on slot up.
			 */
			for(j = i + 1; j < node->v_next_used; j++)
				node->v_next[j - 1] = node->v_next[j];

			node->v_next[node->v_next_used - 1] = NULL;

			/*
			 * Decrement number of pointers.
			 */
			node->v_next_used--;
			return;
		}
	}
}


DBS_API struct dbs_christree_node *dbs_christree_get_v_next(struct dbs_christree_node *n,
		u8 dif)
{
	s32 i;

	if(!n) {
		ALARM(ALARM_WARN, "n undefined");
		return NULL;
	}

	for(i = 0; i < n->v_next_used; i++) {
		if(!n->v_next[i])
			continue;

		if(n->v_next[i]->dif == dif)
			return n->v_next[i];
	}

	return NULL;
}


DBS_API s8 dbs_christree_link_hori(struct dbs_christree *tree,
		struct dbs_christree_node *node)
{
	struct dbs_christree_layer *layer;
	struct dbs_christree_node *n_ptr;
	struct dbs_christree_node *n_h_v_prev;

	if(!tree || !node || node->layer < 0) {
		ALARM(ALARM_WARN, "tree or node undefined");
		return -1;
	}

	layer = &tree->layer[node->layer];

	node->layer_id = layer->count;
	layer->count++;

	/*
	 * Check if there are nodes cross linked on the layer.
	 */
	if(layer->node == NULL) {
		layer->node = node;

		layer->node_num++;
		return 0;
	}
	else {
		/*
		 * Otherwise, link it to the other nodes.
		 */
		n_ptr = layer->node;
		n_h_v_prev = NULL;
		while(n_ptr) {
			if(n_ptr->dif > node->dif) {
				if(n_ptr->h_v_prev == NULL) {
					layer->node = node;

					node->h_v_next = n_ptr;
					n_ptr->h_v_prev = node;

					layer->node_num++;
					return 0;
				}
				else {
					node->h_v_prev = n_ptr->h_v_prev;
					node->h_v_next = n_ptr;

					(n_ptr->h_v_prev)->h_v_next = node;
					n_ptr->h_v_prev = node;

					layer->node_num++;
					return 0;
				}
			}

			n_h_v_prev = n_ptr;
			n_ptr = n_ptr->h_v_next;
		}

		n_h_v_prev->h_v_next = node;
		node->h_v_prev = n_h_v_prev;

		layer->node_num++;
		return 0;
	}

	return -1;
}


DBS_API void dbs_christree_unlink_hori(struct dbs_christree *tree,
		struct dbs_christree_node *node)
{
	struct dbs_christree_layer *layer;
	struct dbs_christree_node *n_ptr;

	if(!tree || !node) {
		ALARM(ALARM_WARN, "tree or node undefined");
		return;
	}

	layer = &tree->layer[node->layer];
	n_ptr = layer->node;

	while(n_ptr) {
		if(n_ptr->layer_id == node->layer_id) {
			/*
			 * Relink nodes.
			 */
			if(n_ptr->h_v_next) {
				n_ptr->h_v_next->h_v_prev = n_ptr->h_v_prev;
			}

			if(n_ptr->h_v_prev == NULL) {
				layer->node = n_ptr->h_v_next;
			}
			else {
				n_ptr->h_v_prev->h_v_next = n_ptr->h_v_next;
			}

			/*
			 * Decrement number of nodes in the layer.
			 */
			layer->node_num--;

			return;
		}

		n_ptr = n_ptr->h_v_next;
	}
}


DBS_API s8 dbs_christree_link_verti(struct dbs_christree_node *n,
		struct dbs_christree_node *v_prev)
{
	if(!n || !v_prev) {
		ALARM(ALARM_WARN, "n or v_prev undefined");
		return -1;
	}

	/*
	 * Add n to the v_next list of the v_previous node.
	 */
	if(dbs_christree_add_v_next(v_prev, n) < 0)
		goto err_return;

	/*
	 * Add v_prev to the v_previous list of n.
	 */
	if(dbs_christree_add_v_prev(n, v_prev) < 0)
		goto err_rmv_v_next;

	return 0;

err_rmv_v_next:
	dbs_christree_rmv_v_next(v_prev, n);

err_return:
	return -1;
}


DBS_API s8 dbs_christree_unlink_verti(struct dbs_christree_node *n,
		struct dbs_christree_node *v_prev)
{
	if(!n || !v_prev) {
		ALARM(ALARM_WARN, "n or v_prev undefined");
		return -1;
	}

	dbs_christree_rmv_v_next(v_prev, n);

	dbs_christree_rmv_v_prev(n);

	return 0;
}


DBS_API s8 dbs_christree_link_node(struct dbs_christree *tree,
		struct dbs_christree_node *node,
		struct dbs_christree_node *v_prev)
{
	if(!tree || !node) {
		ALARM(ALARM_WARN, "tree or node undefined");
		return -1;
	}


	/*
	 * Link node vertically.
	 */
	if(dbs_christree_link_verti(node, v_prev) < 0)
		goto err_return;


	/*
	 * Add the node to the layer list.
	 */
	if(dbs_christree_link_hori(tree, node) < 0)
		goto err_unlink_verti;

	return 0;

err_unlink_verti:
	dbs_christree_unlink_verti(node, v_prev);


err_return:
	ALARM(ALARM_ERR, "Failed to link node");
	return -1;
}


DBS_API void dbs_christree_unlink_node(struct dbs_christree *tree, 
		struct dbs_christree_node *node,
		struct dbs_christree_node *v_prev)
{
	if(!tree || !node) {
		ALARM(ALARM_WARN, "tree or node undefined");
		return;
	}

	/*
	 * Unlink from the layer list.
	 */
	dbs_christree_unlink_hori(tree, node);

	/*
	 * Unlink the node vertically.
	 */
	dbs_christree_unlink_verti(node, v_prev);
}


DBS_API s8 dbs_christree_add(struct dbs_christree *tree,
		u8 *str, void *data)
{
	s32 i;
	struct dbs_christree_node *node;
	struct dbs_christree_node *n_ptr;

	if(!tree || !str || !data) {
		ALARM(ALARM_WARN, "tree or str or data undefined");
		return -1;
	}

	n_ptr = tree->root; 
	for(i = 0; i < tree->layer_num; i++) {

		/*
		 * Check if the required node is already linked below.
		 */
		if(!(node = dbs_christree_get_v_next(n_ptr, str[i]))) {

			/*
			 * Otherwise create a new node and link it.
			 */

			if(!(node = dbs_christree_new(i, str[i])))
				goto err_return;

			if(dbs_christree_link_node(tree, node, n_ptr) < 0)
				goto err_del_node;

		}
		else {
		}


		n_ptr = node;
	}

	/*
	 * Link the datanection.
	 */
	n_ptr->data = data;

	return 0;

err_del_node:
	dbs_christree_del(node);

err_return:
	ALARM(ALARM_ERR, "Failed to add new node to the catree");
	return -1;
}


DBS_API void dbs_christree_rmv(struct dbs_christree *tree,
		u8 *str)
{
	struct dbs_christree_node *n_ptr;
	struct dbs_christree_node *n_v_next;
	struct dbs_christree_node *n_v_prev;
	s32 i;

	if(!tree || !str) {
		ALARM(ALARM_WARN, "tree or str undefined");
		return;
	}	

	n_ptr = tree->root;
	for(i = 0; i < tree->layer_num; i++) {
		if(!(n_v_next = dbs_christree_get_v_next(n_ptr, str[i]))) {
			return;
		}

		n_ptr = n_v_next;
	}

	for(i = tree->layer_num - 1; i >= 0; i--) {
		n_v_prev = n_ptr->v_prev;

		if(n_ptr->v_next_used >= 1 || n_v_prev == NULL)
			return;

		dbs_christree_unlink_node(tree, n_ptr, n_v_prev);

		dbs_christree_del(n_ptr);

		n_ptr = n_v_prev;
	}
}


DBS_API void dbs_christree_sel_hlf(struct dbs_christree_node *nc, void *d)
{
	struct dbs_christree_sel_pass *pass = d;
	s32 i;

	if(nc->data != NULL) {
		pass->data[pass->c] = nc->data;
		pass->c++;
	}

	if(pass->c >= pass->lim)
		return;

	for(i = 0; i < nc->v_next_used; i++) {
		dbs_christree_sel_hlf(nc->v_next[i], d);
	}
}



DBS_API s32 dbs_christree_sel(struct dbs_christree *tree,
		struct dbs_chrismask *mask, void **data, s32 lim)
{
	struct dbs_christree_sel_pass pass;
	s32 i;
	s32 j;

	struct dbs_christree_node *lst[64];
	s32 num;

	if(!tree || !mask || lim < 1) {
		ALARM(ALARM_WARN, "tree or mask undefined");
		return -1;
	}

	/*
	 * Go through the mask to find the branches to collect the nodes from.
	 */
	if((num = dbs_christree_get_layer(tree, mask->off, mask->data[0],
					lst, 64)) < 0)
		return 0;


	/*
	 * Set the passing argument for searching recursivly.
	 */
	pass.c = 0;
	pass.lim = lim;
	pass.data = data;


	for(i = 0; i < num; i++) {
		for(j = 1; j < mask->len; j++) {
			if(!(lst[i] = dbs_christree_get_v_next(lst[i],
							mask->data[j])))
				return 0;
		}

		/*
		 * Recursivly collect all data connections from branches below
		 * this node.
		 */
		dbs_christree_sel_hlf(lst[i], &pass);
	}

	return pass.c;

}

DBS_API s32 dbs_christree_dump_rec(struct dbs_christree_node *n)
{
	s32 i;
	s32 l;

	l = n->layer + 1;
	for(i = 0; i < l; i++) {
		printf("  ");
	}

	printf("- %d (%d): ", l, n->v_next_used);
	printf("%02x (%c)\n", n->dif, (char)n->dif);


	for(i = 0; i < n->v_next_used; i++) {
		dbs_christree_dump_rec(n->v_next[i]);	
	}

	return 0;
}


DBS_API s8 dbs_christree_dump(struct dbs_christree *tree)
{
	if(!tree) {
		ALARM(ALARM_WARN, "tree undefined");
		return -1;
	}

	dbs_christree_dump_rec(tree->root);

	return 0;
}


DBS_API s8 dbs_christree_dump_layers(struct dbs_christree *tree)
{
	s32 i;
	struct dbs_christree_node *n_ptr;

	if(!tree) {
		ALARM(ALARM_WARN, "tree undefined");
		return -1;
	}

	for(i = 0; i < tree->layer_num; i++) {
		printf("Layer %d(%d): ", i, tree->layer[i].node_num);

		n_ptr = tree->layer[i].node;
		while(n_ptr) {
			printf("%02x (%c, v_next_used %d)", n_ptr->dif, 
					(char)n_ptr->dif,
					n_ptr->v_next_used);

			if(n_ptr->h_v_next)
				printf(", ");

			n_ptr = n_ptr->h_v_next;
		}

		printf("\n");
	}

	return 0;
}
