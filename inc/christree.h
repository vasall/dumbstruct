#ifndef _PNC_CATREE_H
#define _PNC_CATREE_H

#include "define.h"
#include "connection.h"

/*
 * 
 */
#define PNC_CATREE_LIM       32
#define PNC_CATREE_NEXT_MIN  5


struct pnc_catree_node {
	/*
	 * A pointer to both the node above and the nodes below. 
	 */
	struct pnc_catree_node       *prev;
	struct pnc_catree_node       **next;
	int                          next_used;
	int                          next_alloc;
	
	/*
	 * The cross pointers for the layer.
	 */
	struct pnc_catree_node       *before;
	struct pnc_catree_node       *after;

	/*
	 * The layernumber the node is on. 
	 */
	int                          layer;

	/*
	 * A data character to differentiate the branch.
	 */
	char                         data;

	/*
	 * Pointer to the connection. 
	 */
	struct pnc_con               *con;
};


struct pnc_catree_layer {
	struct pnc_catree_node       *node;
	int                          node_num;
};


struct pnc_catree {
	/*
	 * The root nodes for each branch of the connection address tree. 
	 */
	struct pnc_catree_rnode      *root;

	/*
	 * Each layer is a cross linked like a linked list. 
	 */
	struct pnc_catree_node       *layers;
	int                          layers_num;
};



/*
 * 
 */
PNC_API struct pnc_catree *pnc_catree_init(void);


/*
 * 
 */
PNC_API void pnc_catree_close(struct pnc_catree *tree);


/*
 * 
 */
PNC_API struct pnc_catree_node *pnc_catree_get_next(struct pnc_catree_node *n,
		char data);


/*
 * 
 */
PNC_API struct pnc_catree_node *pnc_catree_get_layer(struct pnc_catree *tree,
		int layer, char data);


/*
 * 
 */
PNC_API struct pnc_catree_node *pnc_catree_new(struct pnc_catree_node *prev,
		int layer, char data, struct pnc_con *con);


/*
 * 
 */
PNC_API int pnc_catree_add_next(struct pnc_catree_node *node,
		struct pnc_catree_node *next);

/*
 * 
 */
PNC_API void pnc_catree_rmv_next(struct pnc_catree_node *node,
		struct pnc_catree_node *next);


/*
 * 
 */
PNC_API int pnc_catree_link_layer(struct pnc_catree *tree,
		struct pnc_node *node);


/*
 *  
 */
PNC_API void pnc_catree_unlink_layer(struct pnc_catree *tree,
		struct pnc_node *node);


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
PNC_API int pnc_catree_link_node(struct pnc_catree *tree,
		struct pnc_catree_node *node);


/*
 * 
 */
PNC_API void pnc_catree_unlink_node(struct pnc_catree *tree, 
		struct pnc_catree_node *node);


/*
 * 
 */
PNC_API int pnc_catree_add(struct pnc_catree *tree,
		struct pnc_uniaddr *addr, struct pnc_con *con);


/*
 * 
 */
PNC_API void pnc_catree_rmv(struct pnc_catree *tree,
		struct pnc_uniaddr *addr);


struct pnc_catree_sel_pass {
	int c;
	int lim;
	struct pnc_con **con;
};


PNC_API void pnc_catree_sel_hlf(struct pnc_catree_node *nc, void *d);


/*
 * 
 */
PNC_API int pnc_catree_sel(struct pnc_catree *tree,
		struct pnc_unimask *mask, struct pnc_con **con, int lim);


#endif /* _PNC_CATREE_H */
