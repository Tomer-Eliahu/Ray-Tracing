#pragma once

#include "hittable.h"
#include "vec3.h"
#include "sphere.h"
#include "quad.h"
#include "constant_medium.h"

/*Book 2: Section 8.1: Instance Translation.

Note we actually move the *bbox* of the object in the scene (world) space.
But we don't have to move the underlying object.
Instead we can offset the rays that hit the moved bbox, to test if they would hit
the underlying object.

Think of this as a change of coordinates (because that is what it is):

1. Change the ray from world space to object space
2. Determine whether an intersection exists in object space (and if so, where)
3. Change the intersection point from object space to world space

We are moving the incident ray backwards the offset amount,
determining if an intersection occurs, and then moving that intersection point forward the offset amount.

*/

/// @brief A Translation of some Hittable_Object. This in itself is a Hittable_Object. **Must** call tran_init()
/// after init the struct with the tran_object and offset values (to init the bbox).
struct Translate
{
    struct Hittable *tran_object; //< The underlying object we "offset".
    vec3 offset;

    struct AABB bbox; //< The bbox of *this* Translate object (!= the underlying object's bbox).
};

/*Book 2: Section 8.2: Instance Rotation

Rotation is a change of coordinates.

Rotating an object will not only change the point of intersection,
but will also change the surface normal vector,
which will change the direction of reflections and refractions.
So we need to change the normal as well.
Fortunately, the normal will rotate similarly to a vector, so we can use the same formulas as above.
While normals and vectors may appear identical for an object undergoing rotation and translation,
an object undergoing scaling requires special attention to keep the normals orthogonal to the surface.
We won't cover that here, but **you should research surface normal transformations if you implement scaling**.

We need to start by changing the ray from world space to object space,
which for rotation means rotating by −θ.

x′=cos(θ)⋅x−sin(θ)⋅z
z′=sin(θ)⋅x+cos(θ)⋅z

We can now create a Hittable for y-rotation (rotation about the y-axis).

For details on where the rotation equations come from see

https://www.geeksforgeeks.org/maths/rotation-matrix/

*/

/// @brief A Rotation of some Hittable_Object. This in itself is a Hittable_Object. **Must** call rotate_init()
/// after init the struct with the rotate_object (to init the bbox).
struct Rotate_y
{
    struct Hittable *rotate_object; //< The underlying object we "rotate".
    double sin_theta;
    double cos_theta;

    struct AABB bbox; //< The bbox of *this* Rotation object (!= the underlying object's bbox).
};

/// @brief A hittable made up of other hittables. **Note this is a transparent container**
/// it is *not* its own object that exists in the scene. This enables certain efficiencies (like instead of rotating
/// many different objects, making a composite hittable from them and then simply rotating
/// that 1 composite hittable's bbox).
/// Note the bbox of this object is root->bbox.
/// Must init the root by calling BVH_construct_tree().
struct Composite_Hittable
{
    struct BVH_Node *root;
};

/// @brief An enum of all possible hittable objects (we can then have an array of the type [Hittable]
/// for a list of hittalbe objects).
///
/// [Hittable]: https://github.com/Tomer-Eliahu/Ray-Tracing/blob/main/src/InOneWeekend/hittable_list.h
enum Which_Hittable
{
    Sphere,
    Quad,
    Translate,
    Rotate_y,
    Constant_Medium,
    Composite_Hittable
};

union Hittable_Object
{
    struct Sphere sphere;
    struct Quad quad;
    struct Translate translate;
    struct Rotate_y rotate_y;
    struct Constant_Medium cm;
    struct Composite_Hittable comp;
};

struct Hittable
{
    enum Which_Hittable which;
    union Hittable_Object object;
};

// Forward declare object_hit

/// @brief Detect if the ray hits this Hittable object (and updates the hit record accordingly).
/// @param h_object
/// @param r
/// @param ray_t
/// @param rec The Hit_Record
/// @return bool if the Hittable object was hit by the given ray
bool object_hit(const struct Hittable *h_object, const struct Ray *r, struct Interval ray_t, struct Hit_Record *rec);

// Forward declare get_bbox

/// @brief Find out the bbox of a Hittable object. Returns a non-null pointer to it.
/// If this function fails, the program exits.
const struct AABB *get_bbox(const struct Hittable *h_object);

/// @brief Init the bbox of this Translate struct.
/// @remark Must have struct Hittable *tran_object (including the underlying object's bbox!)
/// and vec3 offset initialized before this call!
void tran_init(struct Translate *tran)
{
    // Find out the underlying object bbox
    const struct AABB *underlying_bbox = get_bbox(tran->tran_object);

    // We know underlying_bbox != nullptr
    bbox_offset(&tran->bbox, underlying_bbox, tran->offset);
}

/// @brief Detect if the ray hits the translated_object (and updates the hit record with the
/// data we would have had *as if* we had actually moved the underlying object in the scene).
/// @param r
/// @param ray_t
/// @param rec
/// @return bool if the underlying object was hit by given ray.
bool translate_hit(const struct Translate *tran,
                   const struct Ray *r, struct Interval ray_t, struct Hit_Record *rec)
{
    // Move the ray backwards by the offset
    struct Ray offset_r = {.tm = r->tm};
    subtract(offset_r.origin, (double *)r->origin, (double *)tran->offset);
    // Note the direction of the ray is unchanged
    memcpy(offset_r.direction, (double *)r->direction, sizeof(double) * 3);

    // Determine whether an intersection exists along the offset ray (and if so, where)
    if (!object_hit(tran->tran_object, &offset_r, ray_t, rec))
    {
        return false;
    }

    // Move the intersection point forwards by the offset
    add(rec->p, rec->p, (double *)tran->offset);
    return true;
}

/// @brief Init the bbox of this Rotate struct (+ rot->sin_theta and rot->cos_theta).
/// @remark Must have struct Hittable *rotate_object (including the underlying object's bbox!)
/// initialized before this call!
void rotate_init(struct Rotate_y *rot, double angle)
{
    double radians = degrees_to_radians(angle);
    rot->sin_theta = sin(radians);
    rot->cos_theta = cos(radians);

    const struct AABB *underlying_bbox = get_bbox(rot->rotate_object);

    // We know underlying_bbox != nullptr

    point3 min = {infinity, infinity, infinity};
    point3 max = {-infinity, -infinity, -infinity};

    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            for (int k = 0; k < 2; k++)
            {
                // Visit all vertices of the underlying bbox
                double x = i * underlying_bbox->x.max + (1 - i) * underlying_bbox->x.min;
                double y = j * underlying_bbox->y.max + (1 - j) * underlying_bbox->y.min;
                double z = k * underlying_bbox->z.max + (1 - k) * underlying_bbox->z.min;

                // Transform each vertex from world space to object space (that is rotate the vector from
                // the origin to this vertex).
                double newx = rot->cos_theta * x + rot->sin_theta * z;
                double newz = (-1.0) * rot->sin_theta * x + rot->cos_theta * z;

                vec3 tester = {newx, y, newz};

                // Update our new proposed axis-aligned bbox extrema
                for (int c = 0; c < 3; c++)
                {
                    min[c] = fmin(min[c], tester[c]);
                    max[c] = fmax(max[c], tester[c]);
                }
            }
        }
    }

    aabb_from_points(&rot->bbox, min, max);
}

/// @brief Detect if the ray hits the rotated object (and updates the hit record with the
/// data we would have had *as if* we had actually rotated the underlying object in the scene).
/// @param r
/// @param ray_t
/// @param rec
/// @return bool if the underlying object was hit by given ray.
bool rotate_hit(const struct Rotate_y *rot,
                const struct Ray *r, struct Interval ray_t, struct Hit_Record *rec)
{

    // Transform the ray from world space to object space.
    struct Ray rotated_r = {.tm = r->tm};
    vec_rotate(rotated_r.origin, r->origin, rot->cos_theta, rot->sin_theta);
    // **NOTE** unlike translation which left direction unchanged, rotating not only changes the origin
    // of the Ray, but also its direction (think about it in 2D).
    // You can also think about it like so, the ray starts from the origin and at time t=1 is at a point
    // in space. The difference between those 2 points in the direction of the ray. That is why it is
    // unchanged in translation but changed in rotation.
    //
    // Note that if it is unclear why the direction just gets rotated the same,
    // draw it on paper, the old_direction from old_ray origin to old_ray at t=1
    // is parallel to old_direction as a vector from the origin. Make that into a rectangle,
    // we are rotating the entire rectangle (so new_dir from the origin is just rotated old_dir
    // and it is parallel to the the vector from new_origin to new_ray at t=1).
    vec_rotate(rotated_r.direction, r->direction, rot->cos_theta, rot->sin_theta);

    // Determine whether an intersection exists in object space (and if so, where).
    if (!object_hit(rot->rotate_object, &rotated_r, ray_t, rec))
    {
        return false;
    }

    // Transform the intersection from object space back to world space (rotate the other way).
    // NOTE we use (-1.0) * sin_theta (that is the entire difference).
    point3 rotated_intersection;
    vec3 rotated_normal;
    vec_rotate(rotated_intersection, rec->p, rot->cos_theta, (-1.0) * rot->sin_theta);
    memcpy(rec->p, rotated_intersection, sizeof(double) * 3);

    vec_rotate(rotated_normal, rec->normal, rot->cos_theta, (-1.0) * rot->sin_theta);
    memcpy(rec->normal, rotated_normal, sizeof(double) * 3);

    return true;
}

bool object_hit(const struct Hittable *h_object, const struct Ray *r, struct Interval ray_t, struct Hit_Record *rec)
{
    switch (h_object->which)
    {
    case (enum Which_Hittable)Sphere:

        return sphere_hit(&h_object->object.sphere, r, ray_t, rec);
        break;

    case (enum Which_Hittable)Quad:

        return quad_hit(&h_object->object.quad, r, ray_t, rec);
        break;

    case (enum Which_Hittable)Translate:

        return translate_hit(&h_object->object.translate, r, ray_t, rec);
        break;

    case (enum Which_Hittable)Rotate_y:

        return rotate_hit(&h_object->object.rotate_y, r, ray_t, rec);
        break;

    case (enum Which_Hittable)Constant_Medium:

        return cm_hit(&h_object->object.cm, r, ray_t, rec);
        break;

    case (enum Which_Hittable)Composite_Hittable:

        return BVH_node_hit(h_object->object.comp.root, r, ray_t, rec);
        break;

    default:
        fprintf(stderr, "Could not identify Hittable in object_hit!\n");
        fflush(stderr);
        exit(EXIT_FAILURE);
        break;
    }
}

/// @brief Creates and returns an array of size 6 of struct Hittable objects. **Uses malloc**.
/// These 6 objects are the rectangles (Quads)
/// that correspond to the (axis aligned) 3D box (six sides) that contains the two opposite vertices a & b.
/// @param mat The material the sides are going to be made of.
/// @remark This function also init the bounding boxes for these quads.
struct Hittable *world_add_box(const point3 a, const point3 b, const struct Material_Cfg *mat)
{
    struct Hittable *world = malloc(sizeof(struct Hittable) * 6);
    if (world == NULL)
    {
        fprintf(stderr, "Could not malloc in world_add_box!\n");
        fflush(stderr);
        exit(EXIT_FAILURE);
    }

    // Construct the two opposite vertices with the minimum and maximum coordinates.
    point3 min = {fmin(a[0], b[0]), fmin(a[1], b[1]), fmin(a[2], b[2])};
    point3 max = {fmax(a[0], b[0]), fmax(a[1], b[1]), fmax(a[2], b[2])};

    vec3 dx = {max[0] - min[0], 0, 0};
    vec3 dy = {0, max[1] - min[1], 0};
    vec3 dz = {0, 0, max[2] - min[2]};

    // front
    world[0] = (struct Hittable){.which = Quad,
                                 .object.quad =
                                     {.Q = {min[0], min[1], max[2]},
                                      .mat_cfg = mat}};
    memcpy(world[0].object.quad.u, dx, sizeof(double) * 3);
    memcpy(world[0].object.quad.v, dy, sizeof(double) * 3);

    // right
    world[1] = (struct Hittable){.which = Quad,
                                 .object.quad =
                                     {.Q = {max[0], min[1], max[2]},
                                      .mat_cfg = mat}};
    negate(world[1].object.quad.u, dz);
    memcpy(world[1].object.quad.v, dy, sizeof(double) * 3);

    // back
    world[2] = (struct Hittable){.which = Quad,
                                 .object.quad =
                                     {.Q = {max[0], min[1], min[2]},
                                      .mat_cfg = mat}};
    negate(world[2].object.quad.u, dx);
    memcpy(world[2].object.quad.v, dy, sizeof(double) * 3);

    // left
    world[3] = (struct Hittable){.which = Quad,
                                 .object.quad =
                                     {.Q = {min[0], min[1], min[2]},
                                      .mat_cfg = mat}};
    memcpy(world[3].object.quad.u, dz, sizeof(double) * 3);
    memcpy(world[3].object.quad.v, dy, sizeof(double) * 3);

    // top
    world[4] = (struct Hittable){.which = Quad,
                                 .object.quad =
                                     {.Q = {min[0], max[1], max[2]},
                                      .mat_cfg = mat}};
    memcpy(world[4].object.quad.u, dx, sizeof(double) * 3);
    negate(world[4].object.quad.v, dz);

    // bottom
    world[5] = (struct Hittable){.which = Quad,
                                 .object.quad =
                                     {.Q = {min[0], min[1], min[2]},
                                      .mat_cfg = mat}};
    memcpy(world[5].object.quad.u, dx, sizeof(double) * 3);
    memcpy(world[5].object.quad.v, dz, sizeof(double) * 3);

    // Initialize bounding boxes for the quads.
    for (int i = 0; i < 6; i++)
    {
        quad_init(&world[i].object.quad);
    }

    return world;
}

/// @brief Creates and returns an array of size 18 of struct Hittable objects. **Uses malloc**.
/// The first 6 objects are the rectangles (Quads)
/// that correspond to the (axis aligned) 3D box (six sides) that contains the two opposite vertices a & b.
/// The next 6 are these Quads rotated by the angle given.
/// The final 6 are these rotated Quads translated by the vec3 offset.
/// @param mat The material the sides are going to be made of.
/// @param offset Move the box by this offset.
/// @param angle The angle by which to rotate the box about the y-axis.
/// @remark This function also init the bounding boxes for all 18 of these quads.
/// @remark If you just want a box with no rotation or translation, then use world_add_box instead.
/// @remark Because of how we implemented translation and rotation, the order here **matters**.
/// That is rotating and then translating is different than translating and then rotating.
/// That is because offsetting the incoming ray and then rotating it is different than first rotating it,
/// and then translating it (because we are rotating relative to the world's y-axis so we are drastically
/// changing the position of objects when doing so as opposed to rotating each object relative to its own y-axis).
/// Think of a rectangle in 2D centered at (2,0), we rotate 90 degrees counter-clockwise and then translate it down
/// 2 units so it is now centered at the origin. This is different then first moving it 2 units down (so it is
/// centered at (2,-2)), and then rotating it. The result would not be centered at the origin.
struct Hittable *world_add_box_rotated_translated(const point3 a, const point3 b,
                                                  const struct Material_Cfg *mat, const vec3 offset, double angle)
{
    struct Hittable *small_world = world_add_box(a, b, mat);
    struct Hittable *world = realloc(small_world, sizeof(struct Hittable) * 18);
    if (world == NULL)
    {
        fprintf(stderr, "Could not malloc in world_add_box_translated_rotated!\n");
        fflush(stderr);
        exit(EXIT_FAILURE);
    }
    small_world = nullptr;

    // Rotate the quads
    for (int i = 0; i < 6; i++)
    {
        world[i + 6] = (struct Hittable){.which = Rotate_y,
                                         .object.rotate_y = {.rotate_object = &world[i]}};
        // Init the bbox
        rotate_init(&world[i + 6].object.rotate_y, angle);
    }

    // Translate the rotated quads
    for (int i = 0; i < 6; i++)
    {
        world[i + 12] = (struct Hittable){.which = Translate,
                                          .object.translate = {.tran_object = &world[i + 6],
                                                               .offset = {offset[0], offset[1], offset[2]}}};
        // Init the bbox
        tran_init(&world[i + 12].object.translate);
    }

    return world;
}
