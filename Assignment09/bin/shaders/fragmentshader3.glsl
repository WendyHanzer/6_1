uniform sampler2D myTextureSampler;
varying vec2 UV;
varying vec3 color;
varying vec3 norm;

void main(void)
   {
	vec4 lightPosition = vec4( 10000.0, 10000.0, 11000.0, 1.0);
	vec4 ambient = vec4(0.3, 0.3, 0.0, 1);
	vec3 L = normalize( lightPosition.xyz - color );
	vec3 E = normalize( -color );
	vec3 H = normalize( L + E );
	float Kd = max( dot(L, norm), 0.0 );
	vec4 diffuse = Kd* vec4(1, 1, 1, 1);
	float Ks = pow( max(dot(norm, H), 0.0), 300.0 );
	vec4 specular = Ks * vec4(1, 1, 1, 1);
	vec4 final = ambient + specular + diffuse;
	final.a = 1.0;
	gl_FragColor = texture2D( myTextureSampler, UV.xy ) * final;
   }