uniform sampler2D myTextureSampler;
varying vec4 v_position;
uniform float isAmb;
uniform float isDL;
uniform float isPL;
uniform float isSL;


uniform float light1x;
uniform float light1y;
uniform float light1z;


uniform float light2x;
uniform float light2y;
uniform float light2z;


varying vec2 UV;
varying vec3 fN;
varying vec3 fE;
varying vec3 fL;

struct BaseLight
{
	vec3  Color;
	float AmbientIntensity;
	float DiffuseIntensity;
	float SpecularIntensity;
	vec3  Position;

};

struct DirectionalLight
{
	BaseLight Base;
};

struct Attenuation
{
	float Constant;
	float Linear;
	float Exp;
};

struct PointLight
{
	BaseLight Base;
	Attenuation Atten;
};

struct SpotLight
{
	PointLight Base;
	float Cutoff;
	vec3 Direction;
};

/* light sources*/
const int NUM_DL = 1;
const int NUM_PL = 1;
const int NUM_SL = 2;

DirectionalLight dl[NUM_DL];
PointLight pl[NUM_PL];
SpotLight sl[NUM_SL];
/* ************* */


vec4 calcDL(BaseLight Light)
{
	


	vec3 N = normalize(fN).xyz;
	vec3 E = normalize(-fE);
	vec3 L = normalize(Light.Position);
	vec3 H = normalize( L + E );
	vec4 ambient = vec4(Light.Color, 1.0) * Light.AmbientIntensity;
	
	
	
	
	float Kd = max( dot(N, L), 0.0 ); //factor
	vec4  diffuse = Kd* vec4(Light.Color,1.0) * Light.DiffuseIntensity; /*DiffuseProduct*/;
	
	float Ks = pow( max(dot(N, H), 0.0), Light.SpecularIntensity /*Shininess*/ );
	vec4  specular = Ks * vec4(Light.Color,1.0);
	if( dot(L, N) < 0.0 )
		specular = vec4(0.0, 0.0, 0.0, 1.0);
	vec4 color = diffuse + specular + ambient;

	color.a = 1.0;
	return color;
	
}

vec4 calcPL( PointLight Light)
{
	
	vec3 N = normalize(fN).xyz;
	vec3 E = normalize(-fE);
	vec3 L = normalize(Light.Base.Position);
	vec3 H = normalize( L + E );
	vec4 ambient = vec4(Light.Base.Color, 1.0) * Light.Base.AmbientIntensity;
	
	
	float Distance = length(-L);
	vec4 Color = calcDL(Light.Base);
	float AttenuationNum =  Light.Atten.Constant + (Light.Atten.Linear * Distance) + (Light.Atten.Exp * Distance * Distance);
	return Color/AttenuationNum;
}

vec4 calcSL(SpotLight Light)
{
	vec3 LightToPixel = normalize(( vec3(0.0,0.0,0.0) - Light.Base.Base.Position));
	float SpotFactor = dot(LightToPixel, Light.Direction);
	
	if (SpotFactor > (Light.Cutoff *-1.0) ){
		vec4 Color = calcPL(Light.Base);
		return Color * (1.0 - (1.0 - SpotFactor) * 1.0/(1.0 - Light.Cutoff));
	}
	else {
		return vec4(0,0,0,0);
	}
}
void main(void)
   {
	   
	   vec4 finalColor = vec4(0,0,0,1.0);
	   //Initialize Lighting
	   
	   //DirectionalLight
	   dl[0].Base.Color = vec3(1.0,1.0,1.0);
	   dl[0].Base.AmbientIntensity = .5;
	   dl[0].Base.DiffuseIntensity = 1.0;
	   dl[0].Base.SpecularIntensity = 1000.0;
	   dl[0].Base.Position = vec3(-2.0,-7.0,6.0);
	   
	   
	   //PointLight
	   pl[0].Base.Color = vec3(1.0,.5,1.0);
	   pl[0].Base.AmbientIntensity = .05;
	   pl[0].Base.DiffuseIntensity = 1.0;
	   pl[0].Base.SpecularIntensity = 10.0;
	   pl[0].Base.Position = vec3(20.0,2.0,2.0);
	   pl[0].Atten.Constant = 0.0;
	   pl[0].Atten.Linear = .05;
	   pl[0].Atten.Exp = .01;
	   
	   //SpotLight
	   sl[0].Base.Base.Color = vec3(1.0,1.0,0.0);
	   sl[0].Base.Base.AmbientIntensity = .003;
	   sl[0].Base.Base.DiffuseIntensity = .01;
	   sl[0].Base.Base.SpecularIntensity = 100.0;
	   sl[0].Base.Base.Position = vec3(light1x,light1y,light1z);
	   sl[0].Direction = vec3(0.0,0.0,0.0);
	   sl[0].Base.Atten.Constant = 0.01;
	   sl[0].Base.Atten.Linear = .30;
	   sl[0].Base.Atten.Exp = .0;
	   sl[0].Cutoff = 5.0;
	   
	   //SpotLight2
	   sl[1].Base.Base.Color = vec3(1.0,1.0,1.0);
	   sl[1].Base.Base.AmbientIntensity = .003;
	   sl[1].Base.Base.DiffuseIntensity = .01;
	   sl[1].Base.Base.SpecularIntensity = 100.0;
	   sl[1].Base.Base.Position = vec3(light2x,light2y,light2z);
	   sl[1].Direction = vec3(0.0,0.0,0.0);
	   sl[1].Base.Atten.Constant = 0.01;
	   sl[1].Base.Atten.Linear = .30;
	   sl[1].Base.Atten.Exp = .0;
	   sl[1].Cutoff = 5.0;

	   
	   vec4 ambient = vec4(dl[0].Base.Color, 1.0) * dl[0].Base.AmbientIntensity;

	   //Directional Lighting
	   for(int i = 0; i < NUM_DL; i++)
	   {
		   if(isDL != 0.0){
		   finalColor = calcDL(dl[i].Base) + finalColor;
		   }
	   }
	   
	   //Point Lighting
	   for(int i = 0; i < NUM_PL; i++)
	   {
		   if(isPL != 0.0){
		   finalColor = calcPL(pl[i]) + finalColor;
		   }
	   }
	   
	   
	   //Spot Lighting
	   for(int i = 0; i < NUM_SL; i++)
	   {
		   if(isSL != 0.0){
		   finalColor = calcSL(sl[i]) + finalColor;
		   }
		   
	   }
	   
	   if(isAmb != 0.0)
	   {
		   finalColor = finalColor + ambient;
	   }

	
	   gl_FragColor = texture2D( myTextureSampler, UV.xy) * finalColor; // <lighting>

	   
	   /* NOTE: Normally we would make the ambient product, specular product, and diffuse product into attributes
		However, it was easier to play aound with the lighting by leaving them in them hard coded into the shaders*/

   }