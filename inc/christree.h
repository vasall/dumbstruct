#ifndef _DBS_CHRISTREE_H
#define _DBS_CHRISTREE_H

#include "define.h"
#include "imports.h"

/*
 * 
 */
#define DBS_CHRISTREE_PREV_MIN  5
#define DBS_CHRISTREE_NEXT_MIN  5



struct dbs_christree_node {
	/*
	 * A pointer to both the nodes above and the nodes below. 
	 */
	struct dbs_christree_node    *v_prev;

	struct dbs_christree_node    **v_next;
	s32                          v_next_used;
	s32                          v_next_alloc;
	
	/*
	 * The cross pointers for the layer.
	 */
	struct dbs_christree_node    *h_v_prev;
	struct dbs_christree_node    *h_v_next;

	/*
	 * The layernumber the node is on. 
	 */
	s32                          layer;

	/*
	 * The unique layer identifier.
	 */
	s32                          layer_id;

	/*
	 * The differenciating byte.
	 */
	u8                           dif;

	/*
	 * The data pointer.
	 */
	void                         *data;
};


struct dbs_christree_layer {
	s32                          layer_num;

	struct dbs_christree_node    *node;
	s32                          node_num;

	s32                          count;
};


struct dbs_christree {
	/*
	 * The root nodes for each branch of the connection address tree. 
	 */
	struct dbs_christree_node    *root;

	/*
	 * Each layer is a cross linked like a linked list. 
	 */
	struct dbs_christree_layer   *layer;
	s32                          layer_num;
};


struct dbs_chrismask {
	s32                          off;
	s32                          len;
	u8                           *data;
};


/*
 * Create and initialize a new christree struct.
 *
 * @lim: The number of layers in the tree
 *
 * Returns: Either a pointer to the newly created tree struct or NULL if an
 *          error occurred
 */
DBS_API struct dbs_christree *dbs_christree_init(s32 lim);


/*
 * Cleanup and destroy a christree.
 *
 * @tree: Pointer to the tree struct
 */
DBS_API void dbs_christree_close(struct dbs_christree *tree);


/*
 * Search for a node with the specified dif character in the layer list.
 *
 * @tree: Pointer to the tree struct
 * @layer: The number of the layer to search in
 * @dif: The dif character the node must have
 * @lst: A list to write all nodes in the layer with the dif code to
 * @lim: The length limit of the given list
 *
 * Returns: The number of nodes written to the list or -1 if an error occurred
 */
DBS_API s32 dbs_christree_get_layer(struct dbs_christree *tree,
		s32 layer, u8 dif, struct dbs_christree_node **lst, s32 lim);


/*
 * Create a new christree node, by allocating the necessary memory and setting
 * the base attributes. This function will not add the node to the tree.
 *
 * @layer: The number of the layer the node is on
 * @dif: The dif character for the node
 *
 * Returns: A pointer to the newly created node or NULL if an error occurred
 */
DBS_API struct dbs_christree_node *dbs_christree_new(s32 layer, u8 dif);


/*
 * Delete a node and free the allocated memory. This function will not unlink
 * the node from the tree.
 *
 * @node: Pointer to the node to delete
 */
DBS_API void dbs_christree_del(struct dbs_christree_node *node);


/*
 * Set the specified node as the v_previous one to the node.
 *
 * @node: Pointer to the node with the v_prev list
 * @v_next: Pointer to the node entry to add to the v_prev list
 *
 * Returns: 0 on success or -1 if an error occurred
 */
DBS_API s32 dbs_christree_add_v_prev(struct dbs_christree_node *node,
		struct dbs_christree_node *v_prev);


/*
 * Remove the node currently set as the v_previous node.
 *
 * @node: Pointer to the node to reset the v_prev pointer for
 */
DBS_API void dbs_christree_rmv_v_prev(struct dbs_christree_node *node);


/*
 * Add an entry to the v_next list of a node.
 *
 * @node: Pointer to the node with the v_next list
 * @v_next: Pointer to the node entry to add to the v_next list
 *
 * Returns: 0 on success or -1 if an error occurred
 */
DBS_API s32 dbs_christree_add_v_next(struct dbs_christree_node *node,
		struct dbs_christree_node *v_next);


/*
 * Remove an entry from the v_next list of a node.
 *
 * @node: Pointer to the node with the v_next list
 * @v_next: Pointer to the node entry to remove from the v_next list
 */
DBS_API void dbs_christree_rmv_v_next(struct dbs_christree_node *node,
		struct dbs_christree_node *v_next);


/*
 * Go through the v_next list of the given node and search for a node with the
 * specified diff character.
 *
 * @n: Pointer to the node to start the v_next list
 * @dif: The dif character the node must have
 *
 * Returns: Either a pointer to the node if found or NULL if no node was found
 *          or an error occurred
 */
DBS_API struct dbs_christree_node *dbs_christree_get_v_next(struct dbs_christree_node *n,
		u8 dif);


/*
 * Link a node in a layer list.
 *
 * @tree: Pointer to the tree struct
 * @node: Pointer to the node to crosslink in the layer
 *
 * Returns: 0 on success or -1 if an error occurred 
 */
DBS_API s32 dbs_christree_link_hori(struct dbs_christree *tree,
		struct dbs_christree_node *node);


/*
 * Unlink a node from a layer list. 
 *
 * @tree: Pointer to the tree struct
 * @id: The layer id of the node
 */
DBS_API void dbs_christree_unlink_hori(struct dbs_christree *tree,
		struct dbs_christree_node *node);


/*
 * Link two nodes vertically.
 *
 * @n: Pointer to the node to link to the v_previous one
 * @v_prev: Pointer to the v_previous node
 *
 * Returns: 0 on success or -1 if an error occurred
 */
DBS_API s32 dbs_christree_link_verti(struct dbs_christree_node *n,
		struct dbs_christree_node *v_prev);


/*
 * Unlink two nodes vertically.
 *
 * @n: Pointer to the node to unlink from the v_previous one
 * @v_prev: Pointer to the v_previous node
 *
 * Returns: 0 on success or -1 if an error occurred
 */
DBS_API s32 dbs_christree_unlink_verti(struct dbs_christree_node *n,
		struct dbs_christree_node *v_prev);

/*
 * This function will link a new node into the tree and the layer lists.
 * It is essential for this function to work, that the following attributes are
 * set:
 *   - layer
 *   - data
 *
 * @tree: Pointer to the tree struct
 * @node: Pointer to the new node to link
 * @v_prev: Pointer to the node above
 *
 * Returns: 0 on success or -1 if an error occurred
 */
DBS_API int dbs_christree_link_node(struct dbs_christree *tree,
		struct dbs_christree_node *node,
		struct dbs_christree_node *v_prev);


/*
 * Unlink a node both from the v_next list from the node above and layer list.
 *
 * @tree: Pointer to the tree struct
 * @node: Pointer to the node to unlink
 * @v_prev: Pointer to the node above
 */
DBS_API void dbs_christree_unlink_node(struct dbs_christree *tree, 
		struct dbs_christree_node *node,
		struct dbs_christree_node *v_prev);


/*
 * Add a new entry to the tree.
 *
 * @tree: Pointer to the tree struct
 * @str: The string to insert into the tree
 * @data: The data pointer to link to the string
 *
 * Returns: 0 on success or -1 if an error occurred
 */
DBS_API s32 dbs_christree_add(struct dbs_christree *tree,
		u8 *str, void *data);


/*
 * Remove an entry from the tree.
 *
 * @tree: Pointer to the tree struct
 * @str: The string to remove from the tree
 */
DBS_API void dbs_christree_rmv(struct dbs_christree *tree,
		u8 *str);


struct dbs_christree_sel_pass {
	s32                    c;
	s32                    lim;
	void                   **data;
};


DBS_API void dbs_christree_sel_hlf(struct dbs_christree_node *nc, void *d);


/*
 * Get data pointers from the tree by filtering using the given mask.
 *
 * @tree: Pointer to the tree struct
 * @mask: Pointer to the mask to use
 * @data: An array of pointers to write the resulting data pointers to
 * @lim: The limit of how many data pointers can be written to the array
 *
 * Returns: The number of selected pointers or -1 if an error occurred
 */
DBS_API s32 dbs_christree_sel(struct dbs_christree *tree,
		struct dbs_chrismask *mask, void **data, s32 lim);


DBS_API s32 dbs_christree_dump_rec(struct dbs_christree_node *n);

/*
 * Print the tree in the console.
 *
 * @tree: Pointer to the tree struct
 *
 * Returns: 0 on success or -1 if an error occurred
 */
DBS_API s32 dbs_christree_dump(struct dbs_christree *tree);


/*
 * Print all layers of the christree in the console.
 *
 * @tree: Pointer to the tree struct
 *
 * Returns: 0 on success or -1 if an error occurred
 */
DBS_API s32 dbs_christree_dump_layers(struct dbs_christree *tree);

#endif /* _DBS_CHRISTREE_H */
