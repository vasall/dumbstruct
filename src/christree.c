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
	if(!(tree->root = malloc(sizeof(struct dbs_christree_node))))
		goto err_free_tree;

	tree->root->prev = NULL;
	tree->root->next_used = 0;
	tree->root->next_alloc = DBS_CHRISTREE_NEXT_MIN;
	tree->root->before = NULL;
	tree->root->after = NULL;

	tmp = tree->root->next_alloc * sizeof(struct dbs_christree_node);
	if(!(tree->root->next = malloc(tmp)))
		goto err_free_root;

	/*
	 * Create and initialize the layers.
	 */
	tree->layer_num = lim;
	tmp = tree->layer_num * sizeof(struct dbs_christree_layer);
	if(!(tree->layer = malloc(tmp)))
		goto err_free_root_next;

	for(i = 0; i < tree->layer_num; i++) {
		tree->layer[i].node = NULL;
		tree->layer[i].node_num = 0;
	}

	return tree;

err_free_root_next:
	free(tree->root->next);

err_free_root:
	free(tree->root);

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
	free(tree->root->next);
	free(tree->root);

	/*
	 * Free the table struct.
	 */
	free(tree);
}


DBS_API struct dbs_christree_node *dbs_christree_new(struct dbs_christree_node *prev,
		int layer, char dif)
{
	struct dbs_christree_node *node;
	int tmp;
	int i;

	/*
	 * Allocate memory for the new node.
	 */
	if(!(node = malloc(sizeof(struct dbs_christree_node))))
		goto err_return;

	/*
	 * Link the previous element.
	 */
	node->prev = prev;
	node->layer = layer;
	node->dif = dif;
	node->data = NULL;

	node->before = NULL;
	node->after = NULL;
		
	node->next_used = 0;
	node->next_alloc = DBS_CHRISTREE_NEXT_MIN;

	/*
	 * Preallocate memory for the next nodes and reset the pointers.
	 */
	tmp = node->next_alloc * sizeof(struct dbs_christree_node *);
	if(!(node->next = malloc(tmp)))
		goto err_return;

	for(i = 0; i < node->next_alloc; i++)
		node->next[i] = NULL;

	return node;

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

	free(node->next);

	free(node);
}


DBS_API struct dbs_christree_node *dbs_christree_get_next(struct dbs_christree_node *n,
		char dif)
{
	int i;

	if(!n) {
		ALARM(ALARM_WARN, "n undefined");
		return NULL;
	}

	for(i = 0; i < n->next_used; i++) {
		if(n->next[i]->dif == dif)
			return n->next[i];
	}

	return NULL;
}


DBS_API struct dbs_christree_node *dbs_christree_get_layer(struct dbs_christree *tree,
		int layer, char dif)
{
	struct dbs_christree_node *n_ptr;

	if(!tree) {
		ALARM(ALARM_WARN, "layer undefined");
		return NULL;
	}
	
	n_ptr = tree->layer[layer].node;
	while(n_ptr) {
		if(n_ptr->dif == dif)
			return n_ptr;

		n_ptr = n_ptr->after;
	}

	return NULL;
}


DBS_API int dbs_christree_add_next(struct dbs_christree_node *node,
		struct dbs_christree_node *next)
{
	struct dbs_christree_node **p;
	int alloc;
	int tmp;
	int i;

	if(!node || !next) {
		ALARM(ALARM_WARN, "node or next undefined");
		return -1;
	}

	/*
	 * Check if there's space left in the next list, and if not, allocate
	 * more.
	 */
	if(node->next_used + 1 >= node->next_alloc) {
		alloc = node->next_alloc * 1.5;
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
			return 0;
		}

		/*
		 * If the next element is bigger than the one we want to insert,
		 * then move all the following ones a slot back and overwrite
		 * the slot with the given node.
		 */
		if(node->next[i]->dif > next->dif) {
			tmp = (node->next_used - i);
			tmp += sizeof(struct dbs_christree_node *);
			memmove(node->next + i + 1, node->next + i, tmp);

			node->next[i] = next;
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
	int tmp;

	if(!node || !next) {
		ALARM(ALARM_WARN, "node or next undefined");
		return;
	}

	for(i = 0; i < node->next_used; i++) {
		if(node->next[i]->dif == next->dif) {
			/*
			 * Move all pointers on slot up.
			 */
			tmp = node->next_used - i - 1;
			tmp *= sizeof(struct dbs_christree_node *);
			memmove(node->next + i, node->next + i + 1, tmp);

			/*
			 * Decrement number of pointers.
			 */
			node->next_used--;
			return;
		}
	}
}

DBS_API int dbs_christree_link_layer(struct dbs_christree *tree,
		struct dbs_christree_node *node)
{
	struct dbs_christree_layer *layer;
	struct dbs_christree_node *n_ptr;
	struct dbs_christree_node *n_before;

	if(!tree || !node) {
		ALARM(ALARM_WARN, "tree or node undefined");
		return -1;
	}


	layer = &tree->layer[node->layer];

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
		return;
	}

	layer = &tree->layer[node->layer];
	n_ptr = layer->node;

	while(n_ptr) {
		if(n_ptr->dif == node->dif) {
			/*
			 * Relink nodes.
			 */
			n_ptr->after->before = n_ptr->before;

			if(n_ptr->before == NULL) {
				layer->node = n_ptr->after;
			}
			else {
				n_ptr->before->after = n_ptr->after;
			}

			/*
			 * Decrement number of nodes in the layer.
			 */
			layer->node_num--;
			return;
		}
	}
}


DBS_API int dbs_christree_link_node(struct dbs_christree *tree,
		struct dbs_christree_node *node)
{
	if(!tree || !node) {
		ALARM(ALARM_WARN, "tree or node undefined");
		return -1;
	}


	/*
	 * Link to the previous element.
	 */
	if(dbs_christree_add_next(node->prev, node) < 0)
		goto err_return;

	/*
	 * Add the node to the layer list.
	 */
	if(dbs_christree_link_layer(tree, node) < 0)
		goto err_rmv_next;

	return 0;

err_rmv_next:
	dbs_christree_rmv_next(node->prev, node);

err_return:
	ALARM(ALARM_ERR, "Failed to link node");
	return -1;
}


DBS_API void dbs_christree_unlink_node(struct dbs_christree *tree, 
		struct dbs_christree_node *node)
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
	 * Unlink from the previous element.
	 */
	dbs_christree_rmv_next(node->prev, node);
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
		if(!(node = dbs_christree_get_next(n_ptr, str[i]))) {
			if(!(node = dbs_christree_new(n_ptr, i, str[i]))) {
				goto err_return;
			}
			
			if(dbs_christree_link_node(tree, node) < 0)
				goto err_del_node;
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
		if(!(n_next = dbs_christree_get_next(n_ptr, str[i])))
			return;

		n_ptr = n_next;
	}

	for(i = tree->layer_num; i >= 0; i--) {
		n_prev = n_ptr->prev;

		if(n_ptr->next_used > 1) {
			return;
		}

		dbs_christree_unlink_node(tree, n_ptr);

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
	struct dbs_christree_node *n_ptr;
	struct dbs_christree_sel_pass pass;
	int i;

	if(!tree || !mask || lim < 1) {
		ALARM(ALARM_WARN, "tree or mask undefined");
		return -1;
	}

	/*
	 * Go through the mask to find the branches to collect the nodes from.
	 */
	if(!(n_ptr = dbs_christree_get_layer(tree, mask->off, mask->data[0])))
		return 0;

	for(i = 1; i < mask->len; i++) {
		if(!(n_ptr = dbs_christree_get_next(n_ptr, mask->data[i])))
			return 0;
	}

	/*
	 * Recursivly collect all datanection from branches below this node.
	 */
	pass.c = 0;
	pass.lim = lim;
	pass.data = data;

	dbs_christree_sel_hlf(n_ptr, &pass);

	return pass.c;

}
