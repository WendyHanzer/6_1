attribute vec2 texcoord;
attribute vec3 normal; // <lighting>

uniform mat4 mvpMatrix;
uniform mat4 ModelView; //<-- This will be used to represent View * Model // <lighting>
uniform mat4 Model; //<-- This will be used to represent View * Model // <lighting>


attribute vec4 v_position;
varying vec2 UV;
varying vec3 fN;
varying vec3 fE;
varying vec3 fL;


void main(void)
   {
	  fN = (ModelView*vec4(normal, 0.0)).xyz;
	  fE = (ModelView*v_position).xyz;
	  fL = vec3(2.0,2.0,2.0) -  v_position.xyz;
	 UV = texcoord;
	mat4 dummy = Model;
	   
	gl_Position = mvpMatrix * v_position;




   }
