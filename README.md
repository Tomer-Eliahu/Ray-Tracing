# Ray Tracing

This repo follows the [ray tracing books](https://raytracing.github.io/) as I code my own ray tracer from *scratch*.
While the source code for the final ray tracer is available, I did not look at it.

I chose to use C instead of C++ (what the books suggest using) or Rust as I wanted to do a more substantial C project.

## Status

I finished all three books. Below is a description of features added in each book (not necessarily in chronological order).

In the first book we set up the bare-bones ray tracer. It included:
* Gamma correction 
* Antialiasing
* A positionable camera
* Defocus blur
* Spheres as the only geometry type of object
* Multiple material types: diffuse materials (Lambertian), metal, dielectrics (think glass)

In book two we implemented bounding volume hierarchies (BVH) to speed up our rendering.
We also added:
* Quadrilaterals (or Quads), another geometry type, which are the foundation for making any 2D primitive. We later used Quads to make Boxes. 
* Motion-blur (capturing moving objects in an image).
* Light sources (emissive materials) instead of making the background lit, as well as the isotropic material type.
* Transforms to easily offset and rotate an object instance (including composite objects).
* Volumetric rendering (think of rendering smoke, clouds, fog, mist, etc.).
* Multiple textures in addition to solid color (which we already had implicitly in book 1): Checker, images (UV mapping), and even Perlin Noise.

### Note
Book three (Ray Tracing: The Rest of Your Life) is mainly about making sure you have the mathematical basis
to be involved in this field professionally: 
> In this volume, I assume that you are either a highly interested student, or are someone who is pursuing a career 
> related to ray tracing. We will be diving into the math of creating a very serious ray tracer. When you are done, you
> should be well equipped to use and modify the various commercial ray tracers found in many popular domains, such as
> the movie, television, product design, and architecture industries.

Thus in the final book, a focus is placed on improving rendering performance and quality. We:
* Implemented stratified sampling.
* Covered how to generate samples according to an arbitrary PDF (probability density function) using the building blocks of uniformly distributed random variables. 
* Covered Monte-Carlo Integration.
* Implemented importance sampling to converge to the correct, unbiased, image faster (with fewer samples) via generating *good* rays (towards important things like lights). This included light sampling and mixture density PDFs. 
* Later implemented sampling towards *multiple* different important things (like a light and a glass sphere which is a "proxy" light source).


## How to view the rendered images
The rendered images are in the images folder.
We use the ppm format when rendering the images.

You can use https://jumpshare.com/viewer/ppm to view the images (no download needed)
or this [VS Code extension] (what I used).


[VS Code extension]: https://marketplace.visualstudio.com/items?itemName=ngtystr.ppm-pgm-viewer-for-vscode


## Some Images
I converted some images from ppm to png, so they can be displayed here:

<p align="center">
  <img src="images\20_book_one_final_render.png" alt="Book 1: Final Render"/>
</p>
<p align="center">
  Book 1: Final Render
</p>

<p align="center">
  <img src="images\31_perlin_noise_marbled_texture.png" alt="Book 2: Perlin Noise Marbled Texture"/>
</p>
<p align="center">
  Book 2: Perlin Noise Marbled Texture
</p>

<p align="center">
  <img src="images\32_quads.png" alt="Book 2: Quadrilaterals"/>
</p>
<p align="center">
  Book 2: Quadrilaterals
</p>

<p align="center">
  <img src="images\40_book_two_final_render_high_quality.png" alt="Book 2: Final Render"/>
</p>
<p align="center">
  Book 2: Final Render
</p>

<p align="center">
  <img src="images\48_Cornell_box_mixture_density_of_cosine_and_light_sampling_HQ.png" alt="Book 3: Cornell box with mixture density of cosine and light sampling"/>
</p>
<p align="center">
  Book 3: Cornell box with mixture density sampling pdf
</p>

<p align="center">
  <img src="images\51_Book_three_final_render_Cornell_box_sampling_towards_light_and_glass_sphere.png" alt="Book 3: Final Render: Cornell box with sampling towards light and glass sphere"/>
</p>
<p align="center">
  Book 3: Final Render: Cornell box with sampling towards light and glass sphere
</p>