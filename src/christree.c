#include "christree.h"

#include "../../alarm/inc/alarm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


DBS_API struct dbs_christree *dbs_christree_init(int lim)
{
	struct dbs_christree *tree;
	int tmp;
	int i;

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


DBS_API struct dbs_christree_node *dbs_christree_new(int layer, char dif)
{
	struct dbs_christree_node *node;
	int tmp;
	int i;

	/*
	 * Allocate memory for the new node.
	 */
	if(!(node = malloc(sizeof(struct dbs_christree_node))))
		goto err_return;

	node->layer = layer;
	node->dif = dif;
	node->data = NULL;

	node->before = NULL;
	node->after = NULL;

	node->prev = NULL;

	node->next_used = 0;
	node->next_alloc = DBS_CHRISTREE_NEXT_MIN;

	/*
	 * Preallocate memory for the next nodes and reset the pointers.
	 */

	tmp = node->next_alloc * sizeof(struct dbs_christree_node *);
	if(!(node->next = malloc(tmp)))
		goto err_free_node;

	for(i = 0; i < node->next_alloc; i++)
		node->next[i] = NULL;

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

	free(node->prev);

	free(node->next);

	free(node);
}


DBS_API int dbs_christree_get_layer(struct dbs_christree *tree,
		int layer, char dif, struct dbs_christree_node **lst, int lim)
{
	struct dbs_christree_node *n_ptr;
	int c = 0;

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

		n_ptr = n_ptr->after;
	}

	return c;
}


DBS_API int dbs_christree_add_prev(struct dbs_christree_node *node,
		struct dbs_christree_node *prev)
{
	if(!node || !prev) {
		ALARM(ALARM_WARN, "node or prev undefined");
		return -1;
	}

	node->prev = prev;

	return 0;
}


DBS_API void dbs_christree_rmv_prev(struct dbs_christree_node *node)
{
	if(!node) {
		ALARM(ALARM_WARN, "node undefined");
		return;
	}

	node->prev = NULL;
}


DBS_API int dbs_christree_add_next(struct dbs_christree_node *node,
		struct dbs_christree_node *next)
{
	struct dbs_christree_node **p;
	int alloc;
	int tmp;
	int i;
	int j;

	if(!node || !next) {
		ALARM(ALARM_WARN, "node or next undefined");
		return -1;
	}

	/*
	 * Check if there's space left in the next list, and if not, allocate
	 * more.
	 */
	if(node->next_used + 1 >= node->next_alloc) {
		alloc = node->next_alloc * 2;
		tmp = alloc * sizeof(struct dbs_christree_node *);
		if(!(p = realloc(node->next, tmp)))
			goto err_return;

		for(i = node->next_alloc; i < alloc; i++)
			p[i] = NULL;

		node->next = p;
		node->next_alloc = alloc;
	}

	for(i = 0; i < node->next_alloc; i++) {
		/*
		 * If there's a free slot, use it.
		 */
		if(node->next[i] == NULL) {

			node->next[i] = next;
			node->next_used++;
			return 0;
		}

		/*
		 * If the next element is bigger than the one we want to insert,
		 * then move all the following ones a slot back and overwrite
		 * the slot with the given node.
		 */
		if(node->next[i]->dif > next->dif) {
			for(j = node->next_used; j > i ; j--)
				node->next[j] = node->next[j - 1];

			node->next[i] = next;
			node->next_used++;

			return 0;
		}
	}

err_return:
	ALARM(ALARM_ERR, "Failed to link next element");
	return -1;
}


DBS_API void dbs_christree_rmv_next(struct dbs_christree_node *node,
		struct dbs_christree_node *next)
{
	int i;
	int j;

	if(!node || !next) {
		ALARM(ALARM_WARN, "node or next undefined");
		return;
	}

	for(i = 0; i < node->next_used; i++) {
		if(node->next[i]->dif == next->dif) {
			/*
			 * Move all pointers on slot up.
			 */
			for(j = i + 1; j < node->next_used; j++)
				node->next[j - 1] = node->next[j];

			node->next[node->next_used - 1] = NULL;

			/*
			 * Decrement number of pointers.
			 */
			node->next_used--;
			return;
		}
	}
}


DBS_API struct dbs_christree_node *dbs_christree_get_next(struct dbs_christree_node *n,
		unsigned char dif)
{
	int i;

	if(!n) {
		ALARM(ALARM_WARN, "n undefined");
		return NULL;
	}

	for(i = 0; i < n->next_used; i++) {
		if(!n->next[i])
			continue;

		if(n->next[i]->dif == dif)
			return n->next[i];
	}

	return NULL;
}


DBS_API int dbs_christree_link_layer(struct dbs_christree *tree,
		struct dbs_christree_node *node)
{
	struct dbs_christree_layer *layer;
	struct dbs_christree_node *n_ptr;
	struct dbs_christree_node *n_before;

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
		n_before = NULL;
		while(n_ptr) {
			if(n_ptr->dif > node->dif) {
				if(n_ptr->before == NULL) {
					layer->node = node;

					node->after = n_ptr;
					n_ptr->before = node;

					layer->node_num++;
					return 0;
				}
				else {
					node->before = n_ptr->before;
					node->after = n_ptr;

					(n_ptr->before)->after = node;
					n_ptr->before = node;

					layer->node_num++;
					return 0;
				}
			}

			n_before = n_ptr;
			n_ptr = n_ptr->after;
		}

		n_before->after = node;
		node->before = n_before;

		layer->node_num++;
		return 0;
	}

	return -1;
}


DBS_API void dbs_christree_unlink_layer(struct dbs_christree *tree,
		struct dbs_christree_node *node)
{
	struct dbs_christree_layer *layer;
	struct dbs_christree_node *n_ptr;

	if(!tree || !node) {
		ALARM(ALARM_WARN, "tree or node undefined");

#if DBS_DEBUG
		printf("tree or node undefined\n");
#endif

		return;
	}

	layer = &tree->layer[node->layer];
	n_ptr = layer->node;

#if DBS_DEBUG
	printf("Unlink node with dif %02x from layer %d\n",
			node->dif, layer->layer_num);
#endif

	while(n_ptr) {
#if DBS_DEBUG
		printf("Try node(%p) with dif %02x\n",
				(void *)n_ptr, n_ptr->dif);
#endif

		if(n_ptr->layer_id == node->layer_id) {
#if DBS_DEBUG
			printf("Found node.\n");
#endif

			/*
			 * Relink nodes.
			 */
			if(n_ptr->after) {
#if DBS_DEBUG
				printf("Link after(%p) to before(%p)\n",
						(void *)n_ptr->after,
						(void *)n_ptr->before);

				printf("After Before %p\n",
						(void *)n_ptr->after->before);
#endif
				n_ptr->after->before = n_ptr->before;
			}

			if(n_ptr->before == NULL) {
#if DBS_DEBUG
				printf("Set after(%p) to root\n",
						(void *)n_ptr->after);
#endif
				layer->node = n_ptr->after;
			}
			else {
#if DBS_DEBUG
				printf("Link before(%p) to after(%p)\n",
						(void *)n_ptr->before,
						(void *)n_ptr->after);
#endif
				n_ptr->before->after = n_ptr->after;
			}

			/*
			 * Decrement number of nodes in the layer.
			 */
			layer->node_num--;
#if DBS_DEBUG
			printf("Reduce number of node in layer to %d\n",
					layer->node_num);
#endif
			return;
		}

		n_ptr = n_ptr->after;
	}

#if DBS_DEBUG
	printf("Node not found in layer\n");
#endif
}


DBS_API int dbs_christree_link_verti(struct dbs_christree_node *n,
		struct dbs_christree_node *prev)
{
	if(!n || !prev) {
		ALARM(ALARM_WARN, "n or prev undefined");
		return -1;
	}

	/*
	 * Add n to the next list of the previous node.
	 */
	if(dbs_christree_add_next(prev, n) < 0)
		goto err_return;

	/*
	 * Add prev to the previous list of n.
	 */
	if(dbs_christree_add_prev(n, prev) < 0)
		goto err_rmv_next;

	return 0;

err_rmv_next:
	dbs_christree_rmv_next(prev, n);

err_return:
	return -1;
}


DBS_API int dbs_christree_unlink_verti(struct dbs_christree_node *n,
		struct dbs_christree_node *prev)
{
	if(!n || !prev) {
		ALARM(ALARM_WARN, "n or prev undefined");
		return -1;
	}

	dbs_christree_rmv_next(prev, n);

	dbs_christree_rmv_prev(n);

	return 0;
}


DBS_API int dbs_christree_link_node(struct dbs_christree *tree,
		struct dbs_christree_node *node,
		struct dbs_christree_node *prev)
{
	if(!tree || !node) {
		ALARM(ALARM_WARN, "tree or node undefined");
		return -1;
	}


	/*
	 * Link node vertically.
	 */
	if(dbs_christree_link_verti(node, prev) < 0)
		goto err_return;


	/*
	 * Add the node to the layer list.
	 */
	if(dbs_christree_link_layer(tree, node) < 0)
		goto err_unlink_verti;

	return 0;

err_unlink_verti:
	dbs_christree_unlink_verti(node, prev);


err_return:
	ALARM(ALARM_ERR, "Failed to link node");
	return -1;
}


DBS_API void dbs_christree_unlink_node(struct dbs_christree *tree, 
		struct dbs_christree_node *node,
		struct dbs_christree_node *prev)
{
	if(!tree || !node) {
		ALARM(ALARM_WARN, "tree or node undefined");
		return;
	}

	/*
	 * Unlink from the layer list.
	 */
	dbs_christree_unlink_layer(tree, node);

	/*
	 * Unlink the node vertically.
	 */
	dbs_christree_unlink_verti(node, prev);
}


DBS_API int dbs_christree_add(struct dbs_christree *tree,
		unsigned char *str, void *data)
{
	int i;
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
		if(!(node = dbs_christree_get_next(n_ptr, str[i]))) {

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
		unsigned char *str)
{
	struct dbs_christree_node *n_ptr;
	struct dbs_christree_node *n_next;
	struct dbs_christree_node *n_prev;
	int i;

	if(!tree || !str) {
		ALARM(ALARM_WARN, "tree or str undefined");
		return;
	}	

	n_ptr = tree->root;
	for(i = 0; i < tree->layer_num; i++) {
		if(!(n_next = dbs_christree_get_next(n_ptr, str[i]))) {
			return;
		}

		n_ptr = n_next;
	}

	for(i = tree->layer_num - 1; i >= 0; i--) {
		n_prev = n_ptr->prev;

		if(n_ptr->next_used >= 1 || n_prev == NULL)
			return;

		dbs_christree_unlink_node(tree, n_ptr, n_prev);

		dbs_christree_del(n_ptr);

		n_ptr = n_prev;
	}
}


DBS_API void dbs_christree_sel_hlf(struct dbs_christree_node *nc, void *d)
{
	struct dbs_christree_sel_pass *pass = d;
	int i;

	if(nc->data != NULL) {
		pass->data[pass->c] = nc->data;
		pass->c++;
	}

	if(pass->c >= pass->lim)
		return;

	for(i = 0; i < nc->next_used; i++) {
		dbs_christree_sel_hlf(nc->next[i], d);
	}
}



DBS_API int dbs_christree_sel(struct dbs_christree *tree,
		struct dbs_chrismask *mask, void **data, int lim)
{
	struct dbs_christree_sel_pass pass;
	int i;
	int j;

	struct dbs_christree_node *lst[64];
	int num;

	if(!tree || !mask || lim < 1) {
		ALARM(ALARM_WARN, "tree or mask undefined");
		return -1;
	}

	/*
	 * Go through the mask to find the branches to collect the nodes from.
	 */
	if((num = dbs_christree_get_layer(tree, mask->off, mask->data[0], lst, 64)) < 0)
		return 0;


	/*
	 * Set the passing argument for searching recursivly.
	 */
	pass.c = 0;
	pass.lim = lim;
	pass.data = data;


	for(i = 0; i < num; i++) {
		for(j = 1; j < mask->len; j++) {
			if(!(lst[i] = dbs_christree_get_next(lst[i], mask->data[j])))
				return 0;
		}

		/*
		 * Recursivly collect all data connections from branches below this node.
		 */
		dbs_christree_sel_hlf(lst[i], &pass);
	}

	return pass.c;

}

DBS_API int dbs_christree_dump_rec(struct dbs_christree_node *n)
{
	int i;
	int l;

	l = n->layer + 1;
	for(i = 0; i < l; i++) {
		printf("  ");
	}

	printf("- %d (%d): ", l, n->next_used);
	printf("%02x (%c)\n", n->dif, (char)n->dif);


	for(i = 0; i < n->next_used; i++) {
		dbs_christree_dump_rec(n->next[i]);	
	}

	return 0;
}


DBS_API int dbs_christree_dump(struct dbs_christree *tree)
{
	if(!tree) {
		ALARM(ALARM_WARN, "tree undefined");
		return -1;
	}

	dbs_christree_dump_rec(tree->root);

	return 0;
}


DBS_API int dbs_christree_dump_layers(struct dbs_christree *tree)
{
	int i;
	struct dbs_christree_node *n_ptr;

	if(!tree) {
		ALARM(ALARM_WARN, "tree undefined");
		return -1;
	}

	for(i = 0; i < tree->layer_num; i++) {
		printf("Layer %d(%d): ", i, tree->layer[i].node_num);

		n_ptr = tree->layer[i].node;
		while(n_ptr) {
			printf("%02x (%c, next_used %d)", n_ptr->dif, 
					(char)n_ptr->dif,
					n_ptr->next_used);

			if(n_ptr->after)
				printf(", ");

			n_ptr = n_ptr->after;
		}

		printf("\n");
	}

	return 0;
}
