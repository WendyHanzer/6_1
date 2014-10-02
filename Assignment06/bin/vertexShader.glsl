attribute vec3 v_position;
attribute vec2 texcoord;
varying vec2 UV;
uniform mat4 mvpMatrix;

void main(void)
{
   UV = texcoord;
   gl_Position = mvpMatrix * vec4(v_position, 1.0);
}
