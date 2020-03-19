layout(points) in;
layout(triangle_strip, max_vertices = 16 /* Max safe value: 256, or MAX_GEOMETRY_OUTPUT_VERTICES */) out;

in gl_PerVertex
{
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
} gl_in[];

out gl_PerVertex
{
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
};

in vec4 vert_color;
out vec4 frag_color;

void main(void)
{
}
