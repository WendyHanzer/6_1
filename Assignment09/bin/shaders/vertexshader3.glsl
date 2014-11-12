attribute vec4 v_position;
attribute vec2 texcoord;
attribute vec3 normal; 

uniform mat4 mvpMatrix;
uniform mat4 ModelView;

varying vec3 color;
varying vec3 norm;
varying vec2 UV;

void main(void)
   {
	UV = texcoord;
	color = vec3(ModelView * v_position);
	norm = normalize( ModelView*vec4(normal, 0.0) ).xyz;
	gl_Position = mvpMatrix * v_position;
   }