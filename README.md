

            

# 🌀 DirectX Ray Tracing Ultimate® Samples

</br>

📝This repository contains **small, progressive DirectX Ray Tracing examples**, designed to teach and demonstrate each part of the graphics pipeline in an intuitive and minimalistic way.  
Each sample is isolated and focused, making it easy to learn DirectX Ray Tracing step by step, without unnecessary complexity.

</br>




## Examples


Example | Details
---------|--------
<img src="Screenshots/Pipeline.png" width=380> | [Pipeline](Src/Pipeline)<br> This example shows how to use DirectX Raytracing (DXR) to write directly to a texture using a ray generation shader. No acceleration structures or geometry are used — this is the simplest possible DXR setup, useful to understand how DispatchRays works and how GPU threads map to pixels. (PSOs [ID3D12PipelineState])
<img src="Screenshots/AccelerationStructures.png" width=380> | [AccelerationStructures](Src/AccelerationStructures)<br> This example introduces acceleration structures (BLAS and TLAS) and shows how to trace rays against real geometry. A single triangle is built into a bottom-level acceleration structure and instanced in a top-level structure. Rays are dispatched and intersect the triangle using the ray tracing pipeline (ray generation, miss, and closest hit shaders).
<img src="Screenshots/VertexBuffer.png" width=380> | [VertexBuffer](Src/VertexBuffer)<br> Let's get some color in our scene. In this tutorial we will add color to our vertices to color our triangle. This involves updating the vertex shader to pass the color to the pixel shader, the pixel shader to output the color passed to it, the vertex structure to add a color attribute, and the input layout to include a color input element.
<img src="Screenshots/IndexBuffer.png" width=380> | [IndexBuffer](Src/IndexBuffer)<br> In this tutorial we will learn how to use indices to define our triangles. This is useful because we can remove duplicate vertices, as many times the same vertex is used in multiple triangles.




## 📘 Goals

- Help others learn how to use **pure DirectX Ray Tracing**.
- Serve as a personal reference for building engines or tools.
- Keep things clean, readable, and low-level.

## 🎯 Requirements

- Windows 10/11
- Visual Studio 2019 or newer
- DirectX Ray Tracing compatible GPU
