attribute vec4 v_position;
attribute vec2 texcoord;
attribute vec3 normal; // <lighting>

uniform mat4 mvpMatrix;
uniform mat4 ModelView; //<-- This will be used to represent View * Model // <lighting>

varying vec4 color;
varying vec2 UV;




void main(void)
   {
	   
	   
	   
	   
	vec4 lightPosition = vec4( 2.0 , 2.0, 2.0 ,2.0);
	vec4 ambient = vec4(.3,.3,.3,1);
	vec3 pos = (ModelView * v_position).xyz;
	vec3 L = normalize( lightPosition.xyz - pos );
	vec3 E = normalize( -pos );
	vec3 H = normalize( L + E );
	vec3 N = normalize( ModelView*vec4(normal, 0.0) ).xyz;
	UV = texcoord;
	float Kd = max( dot(L, N), 0.0 );
	vec4 diffuse = Kd* vec4(1,1,1,1) /*DiffuseProduct*/;
	float Ks = pow( max(dot(N, H), 0.0), 300.0 /*Shininess*/ );
	vec4 specular = Ks * vec4(1,1,1,1);
	gl_Position = mvpMatrix * v_position;
	color = ambient + specular + diffuse;
	color.a = 1.0;
   }