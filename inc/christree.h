#ifndef _DBS_CHRISTREE_H
#define _DBS_CHRISTREE_H

#include "define.h"

/*
 * 
 */
#define DBS_CHRISTREE_NEXT_MIN  5



struct dbs_christree_node {
	/*
	 * A pointer to both the node above and the nodes below. 
	 */
	struct dbs_christree_node       *prev;
	struct dbs_christree_node       **next;
	int                          next_used;
	int                          next_alloc;
	
	/*
	 * The cross pointers for the layer.
	 */
	struct dbs_christree_node       *before;
	struct dbs_christree_node       *after;

	/*
	 * The layernumber the node is on. 
	 */
	int                          layer;

	/*
	 * The differenciating byte.
	 */
	char                         dif;

	/*
	 * The data pointer.
	 */
	void                         *data;
};


struct dbs_christree_layer {
	struct dbs_christree_node       *node;
	int                          node_num;
};


struct dbs_christree {
	/*
	 * The root nodes for each branch of the connection address tree. 
	 */
	struct dbs_christree_node      *root;

	/*
	 * Each layer is a cross linked like a linked list. 
	 */
	struct dbs_christree_layer       *layer;
	int                          layer_num;
};


struct dbs_chrismask {
	int off;
	int len;
	unsigned char *data;
};


/*
 * 
 */
DBS_API struct dbs_christree *dbs_christree_init(int lim);


/*
 * 
 */
DBS_API void dbs_christree_close(struct dbs_christree *tree);


/*
 * 
 */
DBS_API struct dbs_christree_node *dbs_christree_get_next(struct dbs_christree_node *n,
		char dif);


/*
 * 
 */
DBS_API struct dbs_christree_node *dbs_christree_get_layer(struct dbs_christree *tree,
		int layer, char dif);


/*
 * 
 */
DBS_API struct dbs_christree_node *dbs_christree_new(struct dbs_christree_node *prev,
		int layer, char dif);


/*
 * 
 */
DBS_API void dbs_christree_del(struct dbs_christree_node *node);


/*
 * 
 */
DBS_API int dbs_christree_add_next(struct dbs_christree_node *node,
		struct dbs_christree_node *next);

/*
 * 
 */
DBS_API void dbs_christree_rmv_next(struct dbs_christree_node *node,
		struct dbs_christree_node *next);


/*
 * 
 */
DBS_API int dbs_christree_link_layer(struct dbs_christree *tree,
		struct dbs_christree_node *node);


/*
 *  
 */
DBS_API void dbs_christree_unlink_layer(struct dbs_christree *tree,
		struct dbs_christree_node *node);


/*
 * This function will link a new node into the tree and the layer lists.
 * It is essential for this function to work, that the following attributes are
 * set:
 *   - prev
 *   - layer
 *   - data
 *
 * @tree: Pointer to the connection address tree
 * @node: Pointer to the new node to link
 *
 * Returns: 0 on success or -1 if an error occurred
 */
DBS_API int dbs_christree_link_node(struct dbs_christree *tree,
		struct dbs_christree_node *node);


/*
 * 
 */
DBS_API void dbs_christree_unlink_node(struct dbs_christree *tree, 
		struct dbs_christree_node *node);


/*
 * 
 */
DBS_API int dbs_christree_add(struct dbs_christree *tree,
		unsigned char *str, void *data);


/*
 * 
 */
DBS_API void dbs_christree_rmv(struct dbs_christree *tree,
		unsigned char *str);


struct dbs_christree_sel_pass {
	int c;
	int lim;
	void **data;
};


DBS_API void dbs_christree_sel_hlf(struct dbs_christree_node *nc, void *d);


/*
 * 
 */
DBS_API int dbs_christree_sel(struct dbs_christree *tree,
		struct dbs_chrismask *mask, void **data, int lim);


#endif /* _DBS_CHRISTREE_H */
