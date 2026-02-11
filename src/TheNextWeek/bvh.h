#pragma once

#include "rtweekend.h"
#include "aabb.h"
#include "hittable.h"
#include "hittable_list.h"

// #include <algorithm>-- This is for C++. We use qsort from C <stdlib> instead.

// BVH- Bounding volume hierarchy

/*
    A BVH is also going to be a hittable — just like lists of hittables.
    It’s really a container, but it can respond to the query “does this ray hit you?”.
    It is a node in a tree.

    One design question is whether we have two classes, one for the tree, and one for the nodes in the tree;
    or do we have just one class and have the root just be a node we point to.
    We decided to just have the concept of nodes encoded (not in class as we are using C).

    The hit function is pretty straightforward:
    check whether the bounding box for the node is hit, and if so, check the children and sort out any details.

*/

enum Node_Type
{
    PRIME, //< This node is built directly from a hittable object and does not point to other nodes
    COMPOSITE,
};

struct BVH_Node
{
    struct AABB bbox;         //< The bounding box of this node.
    enum Node_Type node_type; //< Is this a PRIME or COMPOSITE node?
    struct BVH_Node *left;    //< the left child node. Only valid for COMPOSITE nodes.
    struct BVH_Node *right;   //< the right child node. Only valid for COMPOSITE nodes.

    /// @brief ONLY valid for PRIME nodes. The Hittable object on which this node is built.
    const struct Hittable *hittable_object;
};

/*
 From (https://en.cppreference.com/w/c/algorithm/qsort.html):
 comparison function which returns ​a negative integer value if the first argument is less than the second,
 a positive integer value if the first argument is greater than the second
 and zero if the arguments are equivalent.
 The signature of the comparison function should be equivalent to the following:

 int cmp(const void *a, const void *b);

 The function must not modify the objects passed to it and must return consistent results when called for
 the same objects, regardless of their positions in the array.
*/

/// @brief Compare two aabb.
static int box_compare(const struct BVH_Node *a, const struct BVH_Node *b, int axis_index)
{
    const struct Interval *a_axis_interval = aabb_axis_interval(&a->bbox, axis_index);
    const struct Interval *b_axis_interval = aabb_axis_interval(&b->bbox, axis_index);

    // We do this as opposed to (int) a_axis_interval->min - b_axis_interval->min
    // to avoid problems like integer overflow and other issues
    // (e.g., 0.5-0.2=0.3, which truncates to 0, incorrectly signaling equality).
    if (a_axis_interval->min < b_axis_interval->min)
        return -1;
    if (a_axis_interval->min > b_axis_interval->min)
        return 1;
    return 0;
}

static int box_x_compare(const void *a, const void *b)
{
    return box_compare((const struct BVH_Node *)a, (const struct BVH_Node *)b, 0);
}

static int box_y_compare(const void *a, const void *b)
{
    return box_compare((const struct BVH_Node *)a, (const struct BVH_Node *)b, 1);
}

static int box_z_compare(const void *a, const void *b)
{
    return box_compare((const struct BVH_Node *)a, (const struct BVH_Node *)b, 2);
}

/// @brief Find the closest object hit by the Ray at the given Interval. Updates the Hit_Record accordingly.
/// Does this by searching recursively down the BVH tree.
/// @param node The node in the BVH tree from which to start checking (usually you would use the root of the tree).
/// @return If any object was hit.
bool BVH_node_hit(const struct BVH_Node *node, const struct Ray *r, struct Interval ray_t, struct Hit_Record *rec)
{
    if (!aabb_hit(&node->bbox, r, ray_t))
    {
        return false;
    }

    if (node->node_type == (enum Node_Type)COMPOSITE)
    {
        bool hit_left = BVH_node_hit(node->left, r, ray_t, rec);

        // If the ray hits something on the left, only consider hits on the right if they are closer
        // (strictly closer due to aabb_hit impl).
        struct Interval right_interval = {.min = ray_t.min, .max = hit_left ? rec->t : ray_t.max};
        bool hit_right = BVH_node_hit(node->right, r, right_interval, rec);

        return hit_left || hit_right;
    }
    else
    {
        // Meaning this is a PRIME node
        // Test if we actually hit the hittable (as opposed to its bounding box)
        switch (node->hittable_object->which)
        {
        case (enum Which_Hittable)Sphere:

            return sphere_hit(&node->hittable_object->object.sphere, r, ray_t, rec);
            break;

        default:
            fprintf(stderr, "Could not identify Hittable!\n");
            fflush(stderr);
            break;
        }
    }
}

/// @brief Takes a malloced array of the PRIME nodes of the world, and the start and end indexes to consider
/// of this array. Builds a subtree that covers that range. Uses **malloc**.
/// @param prime_arr
/// @return The root node of the subtree.
static struct BVH_Node *BVH_construct_sub_tree(struct BVH_Node *prime_arr, int start, int end)
{

    struct BVH_Node *root_node = malloc(sizeof(struct BVH_Node));
    root_node->node_type = (enum Node_Type)COMPOSITE;

    // We can speed up the BVH optimization a bit more. Instead of choosing a random splitting axis,
    // let's split the longest axis of the enclosing bounding box to get the most subdivision
    // (we want to build up the smaller boxes so that they have the least overlap).

    // Build the bounding box of the span of source objects.
    struct AABB span_bbox = AABB_EMPTY;
    for (int object_index = start; object_index < end; object_index++)
    {
        aabb_from_boxes(&span_bbox, &span_bbox, &prime_arr[object_index].bbox);
    }

    int axis = bbox_longest_axis(&span_bbox);
    // Note that span_bbox is precisely the bbox of this root_node.
    root_node->bbox = span_bbox;

    // comparator is a function pointer
    int (*comparator)(const void *, const void *) = (axis == 0)   ? box_x_compare
                                                    : (axis == 1) ? box_y_compare
                                                                  : box_z_compare;

    int object_span = end - start;

    /*
        The traversal algorithm should be smooth and not have to check for null pointers,
        so if we just have one element we duplicate it in each subtree.
        Checking explicitly for three elements and just following one single recursion would probably help a little,
        but we figure the whole method will get optimized later.
    */
    if (object_span == 1)
    {
        root_node->left = &prime_arr[start];
        root_node->right = &prime_arr[start];
    }
    else if (object_span == 2)
    {
        root_node->left = &prime_arr[start];
        root_node->right = &prime_arr[start + 1];
    }
    else
    {
        // Call qsort to sort the relevant part of the array in place
        qsort(prime_arr + start, object_span,
              sizeof(struct BVH_Node), comparator);

        int mid = start + object_span / 2;
        root_node->left = BVH_construct_sub_tree(prime_arr, start, mid);
        root_node->right = BVH_construct_sub_tree(prime_arr, mid, end);
    }

    // Note we already computed the bbox for this root_node above.

    return root_node;
}

/// @brief Constructs and returns a pointer to the root node of a BVH tree. Uses **malloc**.
/// @param world The list of all hittable objects in the world. Their bounding boxes must already be initilized!
/// @param world_length The exact number of hittable objects in the world.
/// @remark world length must be at least one.
/// @remark **The bounding boxes of the hittable objects must already be initilized!!!**
struct BVH_Node *BVH_construct_tree(const struct Hittable *world, int world_length)
{

    struct BVH_Node *arr = malloc(sizeof(struct BVH_Node) * world_length);
    if ((arr == NULL) || (world_length == 0))
    {
        fprintf(stderr, "Could not malloc BVH primary node array!\n");
        fflush(stderr);
        exit(EXIT_FAILURE);
    }

    // First we build an array of all bounding boxes of objects in our scene.
    for (int world_index = 0; world_index < world_length; world_index++)
    {
        // find out which Hittable object this is
        switch (world[world_index].which)
        {
        case (enum Which_Hittable)Sphere:
            arr[world_index] = (struct BVH_Node){.bbox = world[world_index].object.sphere.bbox,
                                                 .node_type = (enum Node_Type)PRIME,
                                                 .hittable_object = &world[world_index]};
            break;

        default:
            fprintf(stderr, "Could not identify Hittable in BVH_construct_tree function!\n");
            fflush(stderr);
            break;
        }
    }

    return BVH_construct_sub_tree(arr, 0, world_length);
}

/* OLD-- my approach-- does not play very nice with optimizations

struct BVH_Node *BVH_construct_tree(const struct Hittable *world, int world_length)
{

    struct BVH_Node *arr = malloc(sizeof(struct BVH_Node) * world_length);
    if ((arr == NULL) || (world_length == 0))
    {
        fprintf(stderr, "Could not malloc BVH primary node array!\n");
        fflush(stderr);
        exit(EXIT_FAILURE);
    }

    // First we build an array of all bounding boxes of objects in our scene.
    for (int world_index = 0; world_index < world_length; world_index++)
    {
        // find out which Hittable object this is
        switch (world[world_index].which)
        {
        case (enum Which_Hittable)Sphere:
            arr[world_index] = (struct BVH_Node){.bbox = world[world_index].object.sphere.bbox,
                                                 .node_type = (enum Node_Type)PRIME,
                                                 .hittable_object = &world[world_index]};
            break;

        default:
            fprintf(stderr, "Could not identify Hittable in BVH_construct_tree function!\n");
            fflush(stderr);
            break;
        }
    }

    // We are building the tree from the bottom up: We've now built the first level of our tree (all the leafs).
    int old_level_size = world_length;
    struct BVH_Node *old_level = arr;
    while (old_level_size > 1)
    {

        int new_level_size = ceil((double)old_level_size / 2.0);
        struct BVH_Node *new_level = malloc(sizeof(struct BVH_Node) * new_level_size);
        if (new_level == NULL)
        {
            fprintf(stderr, "Could not malloc new_level in BVH_construct_tree!\n");
            fflush(stderr);
            exit(EXIT_FAILURE);
        }

        // Sort the old_level randomly along one of the axis.
        int axis = random_int(0, 2);

        // comparator is a function pointer
        int (*comparator)(const void *, const void *) = (axis == 0)   ? box_x_compare
                                                        : (axis == 1) ? box_y_compare
                                                                      : box_z_compare;
        // Call qsort to sort the array in place
        qsort(old_level, old_level_size,
              sizeof(struct BVH_Node), comparator);

        // Build the next level up in the tree

        // Fill in the new level.
        for (int i = 0; i < new_level_size - 1; i++)
        {
            struct AABB bbox;
            aabb_from_boxes(&bbox, &old_level[2 * i].bbox, &old_level[2 * i + 1].bbox);
            new_level[i] = (struct BVH_Node){.bbox = bbox,
                                             .node_type = (enum Node_Type)COMPOSITE,
                                             .left = &old_level[2 * i],
                                             .right = &old_level[2 * i + 1]};
        }

        int i = new_level_size - 1; // This is a special case if old_level_size is odd.
        if (old_level_size % 2 == 0)
        {
            struct AABB bbox;
            aabb_from_boxes(&bbox, &old_level[2 * i].bbox, &old_level[2 * i + 1].bbox);
            new_level[i] = (struct BVH_Node){.bbox = bbox,
                                             .node_type = (enum Node_Type)COMPOSITE,
                                             .left = &old_level[2 * i],
                                             .right = &old_level[2 * i + 1]};
        }
        else
        {
            // If the old_level has odd length, duplicate the last element (so we have even length)
            new_level[i] = (struct BVH_Node){.bbox = old_level[old_level_size - 1].bbox,
                                             .node_type = (enum Node_Type)COMPOSITE,
                                             .left = &old_level[old_level_size - 1],
                                             .right = &old_level[old_level_size - 1]};
        }

        old_level_size = new_level_size;
        old_level = new_level;
    }

    // Set the root node pointer
    return old_level;
}
*/