/**
 * @file ryde_ggm_tree.c
 * @brief Implementation of GGM tree related functions
 */

#include <stdio.h>
#include <string.h>

#include "ggm_tree.h"
#include "seed_expand_rijndael.h"

static inline void ryde_expand_seed(uint8_t pair_node[2][RYDE_SECURITY_BYTES],
                                    const uint8_t salt[RYDE_SECURITY_BYTES],
                                    const uint32_t i,
                                    const uint8_t seed[RYDE_SECURITY_BYTES]) {
    expand_seed_rijndael(pair_node, salt, i, seed);
}


/**
 * \fn static inline size_t ryde_ggm_tree_child_0(size_t i)
 * \brief This function determines if the tree has a left child at the input node position.
 *
 * \param[in] node size_t i position in the tree
 */
static inline size_t ryde_ggm_tree_child_0(size_t i) {
    return (i * 2) + 1;
}

/**
 * \fn static inline size_t ryde_ggm_tree_child_1(size_t i)
 * \brief This function determines if the tree has a right child at the input node position.
 *
 * \param[in] i size_t Node position in the tree
 */
static inline size_t ryde_ggm_tree_child_1(size_t i) {
    return (i * 2) + 2;
}

/**
 * \fn static inline size_t ryde_ggm_tree_sibling(size_t i)
 * \brief This function determines if the tree has a sibling at the input node position.
 *
 * \param[in] i size_t Node position in the tree
 */
static inline size_t ryde_ggm_tree_sibling(size_t i) {
    return i - 1 + (i & 1U ? 2 : 0);
}

/**
 * \fn static inline size_t ryde_ggm_tree_parent(size_t i)
 * \brief This function returns the parent position of the input node position
 *
 * \param[in] i size_t Node position in the tree
 */
static inline size_t ryde_ggm_tree_parent(size_t i) {
    return (i - 1) / 2;
}

/**
 * \fn static size_t ryde_ggm_tree_insert_sorted(size_t arr[], size_t n, size_t key, size_t capacity)
 * \brief This function inserts an element in a sorted list
 *
 * \param[in/out] arr size_t[] Sorted list
 * \param[in] n size_t Length of the current list
 * \param[in] key size_t Element to be inserted in the list
 * \param[in] capacity size_t Maximum capacity Length of the list
 */
static size_t ryde_ggm_tree_insert_sorted(size_t arr[], size_t n, size_t key, size_t capacity) {
    // Cannot insert more elements if n is already
    // more than or equal to capacity

    if (n >= capacity) {
        printf("\n\nCAPACITY Exceeded!!!!\n\n");
        return n;
    }

    int i;
    for (i = (int)n - 1; (i >= 0 && arr[i] > key); i--) {
        arr[i + 1] = arr[i];
    }

    arr[i + 1] = key;

    return (n + 1);
}

/**
 * \fn static int ryde_ggm_tree_is_in_list(size_t arr[], size_t n, size_t x)
 * \brief This function checks if a given element belongs to a given list. If so, it returns its position in the list.
 *
 * \param[in] arr size_t[] Sorted list
 * \param[in] n size_t Length of the current list
 * \param[in] x size_t Element to be checked if belong to the input list
 */
static int ryde_ggm_tree_is_in_list(size_t arr[], size_t n, size_t x) {
    int l = 0;
    int r = (int)n - 1;
    // the loop will run till there are elements in the
    // subarray as l > r means that there are no elements to
    // consider in the given subarray
    while (l <= r) {
        // calculating mid point
        int m = l + (r - l) / 2;
        // Check if x is present at mid
        if (arr[m] == x) {
            return m;
        }
        // If x greater than ,, ignore left half
        if (arr[m] < x) {
            l = m + 1;
        }
            // If x is smaller than m, ignore right half
        else {
            r = m - 1;
        }
    }
    // if we reach here, then element was not present
    return -1;
}

/**
 * \fn static size_t ryde_ggm_tree_remove_from_list(size_t arr[], size_t n, size_t pos)
 * \brief This function removes an element from a given list
 *
 * \param[in] arr size_t[] Sorted list
 * \param[in] n size_t Length of the current list
 * \param[in] pos size_t Element position to be removed from the input list
 */
static size_t ryde_ggm_tree_remove_from_list(size_t arr[], size_t n, size_t pos) {
    if (pos >= n) {
        return n;
    }

    for (size_t i = pos; i < n - 1; i++) {
        arr[i] = arr[i + 1];
    }

    return n - 1;
}

/**
 * \fn ryde_ggm_tree_expand(ryde_ggm_tree_t ggm_tree, const uint8_t salt[RYDE_SALT_BYTES])
 * \brief This function computes a tree by expanding its master seed.
 *
 * \param[out] ggm_tree ryde_ggm_tree_t Representation of the tree with master seed at position zero
 * \param[in] salt uint8_t* Salt used for the signature
 */
void ryde_ggm_tree_expand(ryde_ggm_tree_t ggm_tree, const uint8_t salt[RYDE_SALT_BYTES]) {
    for (size_t i = 0; i < (RYDE_PARAM_TREE_LEAVES - 1); i++) {
        ryde_expand_seed(ggm_tree + ryde_ggm_tree_child_0(i), salt, i, ggm_tree[i]);
    }
}

/**
 * \fn int ryde_ggm_tree_get_sibling_path(ryde_ggm_tree_node_t path_seeds[RYDE_PARAM_MAX_OPEN],
 *                                       const ryde_ggm_tree_t ggm_tree,
 *                                       const size_t hidden_leaves[RYDE_PARAM_TAU])
 * \brief This function calculates the sibling path that hides the leaves, and returns the path length.
 *
 * \param[out] output ryde_ggm_tree_node_t[RYDE_PARAM_MAX_OPEN] Sibling path that hides the leaves
 * \param[in] ggm_tree ryde_ggm_tree_t Representation of the tree with master seed at position zero
 * \param[in] hidden_leaves size_t[RYDE_PARAM_TAU] List of hidden leaves
 */
int ryde_ggm_tree_get_sibling_path(ryde_ggm_tree_node_t path_seeds[RYDE_PARAM_MAX_OPEN],
                                   const ryde_ggm_tree_t ggm_tree,
                                   const size_t hidden_leaves[RYDE_PARAM_TAU]) {
    size_t path_indexes[RYDE_PARAM_MAX_OPEN] = {0};
    size_t n = 0;
    for (size_t i = 0; i < RYDE_PARAM_TAU; i++) {
        size_t node = RYDE_LEAVES_SEEDS_OFFSET + hidden_leaves[i];
        while (node > 0) {
            int pos = ryde_ggm_tree_is_in_list(path_indexes, n, node);
            if (pos >= 0) {
                n = ryde_ggm_tree_remove_from_list(path_indexes, n, (size_t)pos);
                break;
            } else {
                size_t n_prev = n;
                n = ryde_ggm_tree_insert_sorted(path_indexes, n, ryde_ggm_tree_sibling(node), RYDE_PARAM_MAX_OPEN);
                if (n_prev == n) {
                    return -1;
                }
            }
            node = ryde_ggm_tree_parent(node);
        }
    }

    for (size_t i = 0; i < n; i++) {
        memcpy(path_seeds[i], ggm_tree[path_indexes[i]], RYDE_SECURITY_BYTES);
    }

    return (int)n;
}

/**
 * \fn int ryde_ggm_tree_partial_expand(ryde_ggm_tree_t partial_ggm_tree,
                                  const uint8_t salt[RYDE_SALT_BYTES],
                                  const ryde_ggm_tree_node_t path_seeds[RYDE_PARAM_MAX_OPEN],
                                  size_t path_length,
                                  const size_t hidden_leaves[RYDE_PARAM_TAU])
 * \brief This function reconstructs the partial tree determined by the sibling path.
 *
 * \param[out] ggm_tree ryde_ggm_tree_t Representation of the partial tree determined by the sibling path
 * \param[in] salt uint8_t* Salt used for the signature
 * \param[in] path_seeds ryde_ggm_tree_node_t[RYDE_PARAM_MAX_OPEN] Sibling path that hides the leaves
 * \param[in] path_length size_t Length of the sibling path
 * \param[in] hidden_leaves size_t[RYDE_PARAM_TAU] List of hidden leaves
 */
int ryde_ggm_tree_partial_expand(ryde_ggm_tree_t partial_ggm_tree,
                                  const uint8_t salt[RYDE_SALT_BYTES],
                                  const ryde_ggm_tree_node_t path_seeds[RYDE_PARAM_MAX_OPEN],
                                  size_t path_length,
                                  const size_t hidden_leaves[RYDE_PARAM_TAU]) {
    size_t path_indexes[RYDE_PARAM_MAX_OPEN] = {0};
    size_t n = 0;
    for (size_t i = 0; i < RYDE_PARAM_TAU; i++) {
        size_t node = RYDE_LEAVES_SEEDS_OFFSET + hidden_leaves[i];
        while (node > 0) {
            int pos = ryde_ggm_tree_is_in_list(path_indexes, n, node);
            if (pos >= 0) {
                n = ryde_ggm_tree_remove_from_list(path_indexes, n, (size_t)pos);
                break;
            } else {
                size_t n_prev = n;
                n = ryde_ggm_tree_insert_sorted(path_indexes, n, ryde_ggm_tree_sibling(node), RYDE_PARAM_MAX_OPEN);
                if (n_prev == n) {
                    return -1;
                }
            }
            node = ryde_ggm_tree_parent(node);
        }
    }

    size_t k = 0;
    size_t parent_node = ryde_ggm_tree_parent(path_indexes[k]);
    uint8_t valid[RYDE_PARAM_TREE_LEAVES + 1] = {0};

    for (size_t i = 0; i < (RYDE_PARAM_TREE_LEAVES - 1); i++) {
        if (i == parent_node) {
            memcpy(partial_ggm_tree + path_indexes[k], path_seeds[k], RYDE_SECURITY_BYTES);
            if (i < (RYDE_PARAM_TREE_LEAVES / 2)) {
                valid[path_indexes[k]] = 1;
            }
            k++;
            if (k < path_length) {
                parent_node = ryde_ggm_tree_parent(path_indexes[k]);
            }
        } else {
            if (valid[i]) {
                ryde_expand_seed(partial_ggm_tree + ryde_ggm_tree_child_0(i), salt, i, partial_ggm_tree[i]);
                if (i < (RYDE_PARAM_TREE_LEAVES / 2)) {
                    valid[ryde_ggm_tree_child_0(i)] = 1;
                    valid[ryde_ggm_tree_child_1(i)] = 1;
                }
            }
        }
    }

    if (k != path_length) {
        return 1;
    }

    return 0;
}

/**
 * \fn void ryde_ggm_tree_get_leaves(ryde_ggm_tree_leaves_t output, ryde_ggm_tree_t tree)
 * \brief This function returns the leaves as a list of RYDE_PARAM_TREE_LEAVES elements.
 *
 * \param[out] output ryde_ggm_tree_leaves_t Representation of leaves
 * \param[in] ggm_tree ryde_ggm_tree_t Representation of the (partial) tree
 */
void ryde_ggm_tree_get_leaves(ryde_ggm_tree_leaves_t output, ryde_ggm_tree_t tree) {
    size_t first_leaf = RYDE_LEAVES_SEEDS_OFFSET;
    for (size_t i = first_leaf; i < (2 * RYDE_PARAM_TREE_LEAVES - 1); i++) {
        memcpy(&(output[i - first_leaf]), tree[i], RYDE_SECURITY_BYTES);
    }
}

/**
 * \fn void ryde_ggm_tree_print_sibling_path(const uint8_t *path, size_t length)
 * \brief This function prints the sibling path that hides the challenges.
 *
 * \param[in] path ryde_ggm_tree_node_t Representation of a Sibling path
 */
void ryde_ggm_tree_print_sibling_path(const ryde_ggm_tree_node_t path[RYDE_PARAM_T_OPEN]) {
    printf("\n");
    for (size_t i = 0; i < RYDE_PARAM_T_OPEN; i++) {
        printf("               + seed #%04lu: ", i);
        for (size_t j = 0; j < RYDE_SECURITY_BYTES; j++) {
            printf("%02X", path[i][j]);
        }
        printf("\n");
    }
}
