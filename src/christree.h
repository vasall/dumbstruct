#include "connection_tree.h"

#include "log.h"
#include "utils.h"

#include <stdlib.h>



PNC_API struct pnc_ctree *pnc_catree_init(struct pnc_con_tree *tree)
{
	struct pnc_ctree *tree;
	int tmp;
	int i;

	if(!tree) {
		PNC_LOG(PNC_LOG_WARN, "tree undefined");
		return NULL;
	}

	/*
	 * Allocate memory for the tree.
	 */
	if(!(tree = pnc_malloc(sizeof(struct pnc_catree))))
		goto err_return;

	/*
	 * Allocate memory for the root node and initialize it.
	 * Also preallocate memory for the first few few branch nodes.
	 */
	if(!(tree->root = pnc_malloc(sizeof(struct pnc_catree_node))))
		goto err_free_tree;

	tree->root->type = PNC_CATREE_T_ROOT;
	tree->root->prev = NULL;
	tree->root->next_used = 0;
	tree->root->next_alloc = PNC_CATREE_NEXT_MIN;
	tree->root->before = NULL;
	tree->root->after = NULL;

	tmp = tree->root->next_alloc * sizeof(struct pnc_catree_bnode); 
	if(!(tree->root->next = pnc_malloc(tmp)))
		goto err_free_root;

	/*
	 * Create and initialize the layers.
	 */
	tree->layers_num = PNC_UNIADDR_LEN;
	tmp = tree->layers_num * sizeof(struct pnc_catree_layer);
	if(!(tree->layers = pnc_malloc(tmp)))
		goto err_free_root_next;

	for(i = 0; i < tree->layers_num; i++) {
		tree->layer[i].node = NULL;
		tree->layer[i].node_num = 0;
	}

	return tree;

err_free_root_next:
	pnc_free(tree->root->next);

err_free_root:
	pnc_free(tree->root);

err_free_tree:
	pnc_free(tree);

err_return:
	PNC_LOG(PNC_LOG_ERR, "Failed to create address tree");
	return NULL;
}


PNC_API void pnc_catree_close(struct pnc_ctree *tree)
{
	if(!tree) {
		PNC_LOG(PNC_LOG_WARN, "tree undefined");
		return -1;
	}

	/*
	 * Remove all nodes from the tree, except the root node. 
	 */


	/*
	 * Free the layers list.
	 */
	pnc_free(tree->layer);

	/*
	 * Delete the root node.
	 */
	pnc_free(tree->root->next);
	pnc_free(tree->root);

	/*
	 * Free the table struct.
	 */
	pnc_free(tree);
}


PNC_API struct pnc_catree_node *pnc_catree_new(struct pnc_catree_node *prev,
		int layer, char data)
{
	struct pnc_catree_node *node;
	int tmp;
	int i;

	/*
	 * Allocate memory for the new node.
	 */
	if(!(node = pnc_malloc(sizeof(struct pnc_catree_node))))
		goto err_return;

	/*
	 * Link the previous element.
	 */
	node->prev = prev;
	node->layer = layer;
	node->data = data;
	node->con = NULL;

	node->before = NULL;
	node->after = NULL;
		
	node->next_used = 0;
	node->next_alloc = PNC_CATREE_NEXT_MIN;

	/*
	 * Preallocate memory for the next nodes and reset the pointers.
	 */
	tmp = node->next_alloc * sizeof(struct pnc_catree_node *);
	if(!(node->next = pnc_malloc(tmp)))
		goto err_return;

	for(i = 0; i < node->next_alloc; i++)
		node->next[i] = NULL;

	return node;

err_return:
	PNC_LOG(PNC_LOG_ERR, "Failed to create address tree node");
	return NULL;
}


PNC_API void pnc_catree_del(struct pnc_catree_node *node)
{
	if(!node) {
		PNC_LOG(PNC_LOG_WARN, "node undefined");
		return;
	}

	free(node->next);

	free(node);
}


PNC_API struct pnc_catree_node *pnc_catree_get_next(struct pnc_catree_node *n,
		char data)
{
	int i;

	if(!n) {
		PNC_LOG(PNC_LOG_WARN, "n undefined");
		return NULL;
	}

	for(i = 0; i < n->next_used; i++) {
		if(n->next[i]->data == data)
			return n->next[i];
	}

	return NULL;
}


PNC_API struct pnc_catree_node *pnc_catree_get_layer(struct pnc_catree *tree,
		int layer, char data)
{
	struct pnc_catree_layer *layer;
	struct pnc_catree_node *n_ptr;

	if(!tree) {
		PNC_LOG(PNC_LOG_WARN, "layer undefined");
		return NULL;
	}

	layer = &tree->layer[layer];
	
	n_ptr = layer->node;
	while(n_ptr) {
		if(n_ptr->data == data)
			return n_ptr;

		n_ptr = n_ptr->after;
	}

	return NULL;
}


PNC_API int pnc_catree_add_next(struct pnc_catree_node *node,
		struct pnc_catree_node *next)
{
	struct pnc_catree_node **p;
	int alloc;
	int tmp;
	int i;

	if(!node || !next) {
		PNC_LOG(PNC_LOG_WARN, "node or next undefined");
		return -1;
	}

	/*
	 * Check if there's space left in the next list, and if not, allocate
	 * more.
	 */
	if(node->next_used + 1 >= node->next_alloc) {
		alloc = node->next_alloc * 1.5;
		tmp = alloc * sizeof(struct pnc_catree_node *);
		if(!(p = pnc_realloc(node->next, tmp)))
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
		if(node->next[i]->data > next->data) {
			tmp = (node->next_used - i);
			tmp += sizeof(struct pnc_catree_node *);
			memmove(node->next + i + 1, node->next + i, tmp);

			node->next[i] = next;
			return 0;
		}
	}

err_return:
	PNC_LOG(PNC_LOG_ERR, "Failed to link next element");
	return -1;
}


PNC_API pnc_catree_rmv_next(struct pnc_catree_node *node,
		struct pnc_catree_node *next)
{
	int i;
	int tmp;

	if(!node || !next) {
		PNC_LOG(PNC_LOG_WARN, "node or next undefined");
		return -1;
	}

	for(i = 0; i < node->next_used; i++) {
		if(node->next[i]->data == next->data) {
			/*
			 * Move all pointers on slot up.
			 */
			tmp = node->next_used - i - 1;
			tmp *= sizeof(struct pnc_catree_node *);
			memmove(node->next + i, node->next + i + 1, tmp);

			/*
			 * Decrement number of pointers.
			 */
			node->next_used--;
			return 0;
		}
	}

	return 0;
}

PNC_API int pnc_catree_link_layer(struct pnc_catree *tree,
		struct pnc_node *node)
{
	struct pnc_catree_layer *layer;
	int i;
	struct pnc_catree_node *n_ptr;
	struct pnc_catree_node *n_before;

	if(!tree || !node) {
		PNC_LOG(PNC_LOG_WARN, "tree or node undefined");
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
			if(n_ptr->data > node->data) {
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


PNC_API void pnc_catree_unlink_layer(struct pnc_catree *tree,
		struct pnc_node *node)
{
	struct pnc_catree_layer *layer;
	struct pnc_catree_node *n_ptr;
	struct pnc_catree_node *node;

	if(!tree || !node) {
		PNC_LOG(PNC_LOG_WARN, "tree or node undefined");
		return;
	}

	layer = &tree->layer[node->layer];
	n_ptr = layer->node;

	while(n_ptr) {
		if(n_ptr->data == node->data) {
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


PNC_API int pnc_catree_link_node(struct pnc_catree *tree,
		struct pnc_catree_node *node)
{
	if(!tree || !node) {
		PNC_LOG(PNC_LOG_WARN, "tree or node undefined");
		return -1;
	}


	/*
	 * Link to the previous element.
	 */
	if(pnc_catree_add_next(node->prev, node) < 0)
		goto err_return;

	/*
	 * Add the node to the layer list.
	 */
	if(pnc_catree_link_layer(tree, node) < 0)
		goto err_rmv_next;

	return 0;

err_rmv_next:
	pnc_catree_rmv_next(node->prev, node);

err_return:
	PNC_LOG(PNC_LOG_ERR, "Failed to link node");
	return -1;
}


PNC_API void pnc_catree_unlink_node(struct pnc_catree *tree, 
		struct pnc_catree_node *node)
{
	if(!tree || !node) {
		PNC_LOG(PNC_LOG_WARN, "tree or node undefined");
		return;
	}

	/*
	 * Unlink from the layer list.
	 */
	pnc_catree_unlink_layer(tree, node);

	/*
	 * Unlink from the previous element.
	 */
	pnc_catree_rmv_next(node->prev, node);
}


PNC_API int pnc_catree_add(struct pnc_ctree *tree,
		struct pnc_uniaddr *addr, struct pnc_con *con)
{
	int i;
	struct pnc_catree_node *node;
	struct pnc_catree_node *n_ptr;
	int j;
	char data;

	if(!tree || !addr || !con) {
		PNC_LOG(PNC_LOG_WARN, "tree or addr or con undefined");
		return -1;
	}


	n_ptr = tree->root; 
	for(i = 0; i < PNC_UNIADDR_LEN; i++) {
		data = addr->data[i];

		if(!(node = pnc_catree_get_next(n_ptr, addr->data[i]))) {
			if(!(node = pnc_catree_new(n_ptr, i, data[i]))) {
				goto err_return;
			}
			
			if(pnc_catree_link_node(n_ptr, node) < 0)
				goto err_del_node;
		}

		n_ptr = node;
	}

	/*
	 * Link the connection.
	 */
	n_ptr->con = con;

	return 0;

err_del_node:
	pnc_catree_del(node);

err_return:
	PNC_LOG(PNC_LOG_ERR, "Failed to add new node to the catree");
	return -1;
}


PNC_API void pnc_catree_rmv(struct pnc_ctree *tree,
		struct pnc_uniaddr *addr)
{
	struct pnc_catree_node *n_ptr;
	struct pnc_catree_node *n_next;
	struct pnc_catree_node *n_prev;
	int i;

	if(!tree || !addr) {
		PNC_LOG(PNC_LOG_WARN, "tree or addr undefined");
		return;
	}

	n_ptr = tree->root;
	for(i = 0; i < PNC_UNIADDR_LEN; i++) {
		if(!(n_next = pnc_catree_get_next(n_ptr, addr->data[i])))
			return;

		n_ptr = n_next;
	}

	for(i = PNC_UNIADDR_LEN; i >= 0; i--) {
		n_prev = n_ptr->prev;

		if(n_ptr->next_used > 1) {
			return;
		}

		pnc_catree_unlink_node(tree, n_ptr);

		pnc_catree_del(n_ptr);

		n_ptr = n_prev;
	}
}


PNC_API void pnc_catree_sel_hlf(struct pnc_catree_node *nc, void *d)
{
	struct pnc_catree_sel_pass *pass = d;
	int i;

	if(nc->con != NULL) {
		pass->con[pass->c] = nc->con;
		pass->c++;
	}

	if(pass->c >= pass->lim)
		return;

	for(i = 0; i < nc->next_used; i++) {
		pnc_catree_sel_hlf(nc->next[i], d);
	}
}



PNC_API int pnc_catree_sel(struct pnc_ctree *tree,
		struct pnc_unimask *mask, struct pnc_con **con, int lim)
{
	struct pnc_catree_node *n_ptr;
	int i;
	int c = 0;
	int slot;
	struct pnc_catree_sel_pass pass;
	int ret;

	if(!tree || !mask || lim < 1) {
		PNC_LOG(PNC_LOG_WARN, "tree or mask undefined");
		return -1;
	}

	/*
	 * Go through the mask to find the branches to collect the nodes from.
	 */
	if(!(n_ptr = pnc_catree_get_layer(tree, mask->off, mask->data[0])))
		return 0;

	for(i = 1; i < mask->len; i++) {
		if(!(n_ptr = pnc_catree_get_next(n_ptr, mask->addr[i])))
			return 0;
	}

	/*
	 * Recursivly collect all connection from branches below this node.
	 */
	pass.c = 0;
	pass.lim = lim;
	pass.con = con;

	pnc_catree_sel_hlf(n_ptr, &pass);

	return pass.c;

}
