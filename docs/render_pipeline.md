# The Rendering Pipeline
## Overview
This software renderer uses the following pipeline for taking mesh data from a file and converting it into a bitmap that can be blitted to the screen. The stages of this pipeline are as follows:
    - Initial set up:
        - Load all meshes (typically from .obj files - also load supporting resources such as textures).
        - Creation of model structures for each renderable entity (store pointer to underlying mesh).
    - Main / render loop:
        - Transform all models into world space (applying model transform matrix) and then build container of all triangles.
        - Transform all triangles into camera space by applying camera transform matrix.
        - Apply projection matrix to generate projected coordinates in 4d homogeneous form except depth - preserve notions of relative depth to facillitate depth buffering, avoid texture warp, etc. Coordinates are now in clip space.
        - Clip triangles against near and far planes and left, right, top and bottom bounds.
        - Convert to normalised coordinates (i.e. applying perspective divide and aspect ratio).
        - Convert to pixel space.
        - Rasterise with lighting interpolation, depth buffering and texture sampling.

## Transformation to World Space
TODO

## Transformation to Camera Space
TODO

## Perspective Projection
The perspective projection stage of the rendering pipeline, with one caveat that will be discussed shortly, maps coordinates, and the triangles they make up, from 3d space onto a 2d plane / screen. This is necessary since we represent all of our triangles as 3d Cartesian coordinates in world space, but we want to draw them on our window, which is a buffer of pixels that is interpretted as a 2d grid. Perspective projection is a particular mapping which maps ths points to where the would appear on a screen if seen through it, as opposed to simply just the nearest coordinate on the plane (an orthogonal projection).

In a pure mathematical projection, we would map the points onto the plane. This would mean mapping the depth (z coordinate) of the space onto the plane as well. This is not particularly helpful for our purposes, as in order to know which pixels to draw in front of others, we must preserve some notion of the relative depths of the triangles' vertices (this is also necessary for texture sampling, lighting, etc.). Therefore, we do something a bit different.

To perform a perspective projection on a 3d coordinate $(x, y, z)^{\top}$, we first need to define the plane to project onto - we call this the "viewing plane" or the "view plane". In this renderer, we define the width of the viewing plane to be $2$ as it spans from $-1$ to $1$ in the $x$-axis in world space. The height is defined relative to this in world space $\frac{\text{width}}{\text{aspect ratio}}$ as aspect ratio is $\frac{width}{height}$. We take a field of view, which describes the angle of world-space that we want to view from down the forward $z$-axis to the $x$-axis. We will call this FOV $\theta$. The angle between the $z$-axis and the $y$-axis (i.e. the vertical FOV) is relative to this FOV, using the height (derived from the aspect ratio).

Ignoring the quirk about depth and the $z$ coordinates (which we will build up to in the following), the $x$ and $y$ coordinates are simple. Consider that $(x, y, z)$ in Cartesian coordinates must me mapped to some coordinate $(x', y', z')$ where $z'$ is the $z$ ordinate of the viewing plane. Perspective projection of a point is defined such that it preserves the line through it - i.e. the projected coordinate exists on the line from the origin to $(x, y, z)$. Hence we obtain two similar triangles - one with vertices $(0, 0, 0)$, $(0, 0, z')$ and $(x', y', z')$ and one with vertices $(0, 0, 0)$, $(0, 0, z)$ and $(x, y, z)$. Hence, we obtain that:
$$\frac{x'}{z'} = \frac{x}{z}$$
Which gives:
$$x' = \frac{x \times z'}{z}$$
The same logic can also be applied to $y$:
$$y' = \frac{y \times z'}{z}$$

This gives us the $x$ and $y$ coordinates on the viewing plane - representative of our window / screen. As mentioned previously, a "true" perspective projection, like the one we jut did, would also map the $z$ coordinate to the near plane $z$ ordinate, which we called $z'$ in the above. For the rest of our rendering pipeling to work without resorting to hacky methods, we need to preserve some notion of relative depth. The way we do this is by defining a near plane (our viewing plane) and a far plane - a plane parallel to the near plane, marking the maximum distance at which we consider vertices to be in view.