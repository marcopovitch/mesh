# Mesh Generator

Earth Mesh Generator using Quadrilaterally-faced hexahedron. To be used with seismic raytracing.

# Beware
 
Still usable but written 10 years ago ! Use it only if you know what you are doing ... no support will be provided !

# Compilation

To make all the needed *autotools* files, into the `mesh` directory run:

`glibtoolize -i`

`./autogen.sh`

To configure and install the mesh library in your installation directory $YOUR_PATH, run:

`./configure --prefix=$YOUR_PATH`

make install

# Gallery
Subdivided cell

<img src="https://cloud.githubusercontent.com/assets/4367036/10303095/9759448e-6c11-11e5-8789-cecd522e92c8.png" width="250">

Subdivided cell with crossing rays

<img src="https://cloud.githubusercontent.com/assets/4367036/10303111/afb8417e-6c11-11e5-827f-5d1e23f8051f.png" width="250">

Western Europe mesh sliced to the core 

<img src="https://cloud.githubusercontent.com/assets/4367036/10303100/9c62b6f4-6c11-11e5-9271-bf8fb8da275a.jpg" width="250">

Multi-layered "local" mesh with

<img src="https://cloud.githubusercontent.com/assets/4367036/10303083/82e91542-6c11-11e5-8495-2a35b0f28969.png" width="250">

Euro-Mediterranean  mesh with seismic raytracing

<img src="https://cloud.githubusercontent.com/assets/4367036/10303087/8bac89a2-6c11-11e5-8be0-a5aea23a937e.png" width="250">

Multi-layered global earth mesh

<img src="https://cloud.githubusercontent.com/assets/4367036/10303287/eba312da-6c12-11e5-9f95-e75947f102c8.jpg" width="250">