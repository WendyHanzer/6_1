varying vec2 UV;
uniform sampler2D myTextureSampler;

void main(void)
{
   gl_FragColor = texture2D( myTextureSampler, UV.xy );
}