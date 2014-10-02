uniform sampler2D myTextureSampler;
varying vec2 UV;
void main(void)
{
   gl_FragColor = texture2D( myTextureSampler, UV ).bgra;
}