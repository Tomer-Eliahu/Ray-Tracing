# Ray Tracing

This repo follows the [ray tracing books](https://raytracing.github.io/) as I code my own ray tracer from *scratch*.
While the source code for the final ray tracer is available, I did not look at it.

I chose to use C instead of C++ (what the books suggest using) or Rust as I wanted to do a more substantial C project.


## How to view the rendered images
The rendered images are in the images folder.
We use the ppm format of writing some numbers to a file to describe the image.

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

### Note
Book three (Ray Tracing: The Rest of Your Life) is mainly about making sure you have the mathematical basis
to be involved in this field professionally: 
> In this volume, I assume that you are either a highly interested student, or are someone who is pursuing a career 
> related to ray tracing. We will be diving into the math of creating a very serious ray tracer. When you are done, you
> should be well equipped to use and modify the various commercial ray tracers found in many popular domains, such as
> the movie, television, product design, and architecture industries.

<p align="center">
  <img src="images\48_Cornell_box_mixture_density_of_cosine_and_light_sampling_HQ.png" alt="Book 3: Cornell box with mixture density of cosine and light sampling"/>
</p>
<p align="center">
  Book 3: Cornell box with mixture density sampling pdf
</p>