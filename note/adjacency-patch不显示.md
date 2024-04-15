adjacency patch-list不显示问题
====================

# adjacency输出图像为空
这是和Geometroy Shader一起使用的一个primitives topology,如果使用的是triangle strip,在geometry shader中，只能显示一个三角形的三个定点。如果triangle strip adjancency,在geometry shader中，就可以访问相邻三角形的定点 [opengl](https://www.khronos.org/opengl/wiki/Primitive#Adjacency_primitives)


这个adjacency主要功能是为了提供给geometry shader 更多的图元信息，并且只有geometry shader可见，让geometry shader在使用 silhouette detection, shadow volume extrusion(人影检测，阴影体积挤出)这些功能的时候有帮助。 [direct3d11](https://learn.microsoft.com/en-us/windows/win32/direct3d11/d3d10-graphics-programming-guide-primitive-topologies)

# patch-list不显示问题
只有在启用Tessellation的时候，这个原始图元类型才有意义，这个shader的功能是用来图元细分。 [opengl](https://www.khronos.org/opengl/wiki/Primitive#Adjacency_primitives)