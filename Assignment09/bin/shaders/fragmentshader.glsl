uniform sampler2D myTextureSampler;
varying vec2 UV;
varying vec4 color;

void main(void)
   {
	gl_FragColor = texture2D( myTextureSampler, UV.xy ) * color; // <lighting>
   }