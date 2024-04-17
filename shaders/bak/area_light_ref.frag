// Code modified EvilRyu by JuliaPoo
// https://www.shadertoy.com/view/4tBBDK


const float PI = 3.1415926;

const float intensity = 1.5;
const float light_width = .7;
const float light_height = 0.5;

const vec3 light_col = vec3(1.0)*intensity;
const vec3 light_pos = vec3(0., 0.3, 0.);
const vec3 light_normal = vec3(0., 0., 1.);

const float LUTSIZE  = 8.0;
const float MATRIX_PARAM_OFFSET = 8.0;

const mat2 R_obj2 = mat2(
        			0.955336489125606,-0.295520206661339,
        			0.295520206661339, 0.955336489125606
                  	);



float rect(vec3 p, vec3 b)
{
  	vec3 d = abs(p) - b;
  	return min(max(d.x,max(d.y,d.z)),0.0) + length(max(d,0.0));
}

void init_rect_points(out vec3 points[4])
{
    // get the orthogonal basis of polygon light
    vec3 right=normalize(cross(light_normal, vec3(0.0, 1.0, 0.0)));
    vec3 up=normalize(cross(right, light_normal));
    
    vec3 ex = light_width * right;
    vec3 ey = light_height * up;

    points[0] = light_pos - ex - ey;
    points[1] = light_pos + ex - ey;
    points[2] = light_pos + ex + ey;
    points[3] = light_pos - ex + ey;
}


#define LIGHT 0.
#define FLOOR 1.
#define OBJ1  2.
#define OBJ2  3.

float object_id = 0.;

float sphere(vec3 p, float r)
{
   	return length(p)-r;
}

float sdRoundBox( vec3 p, vec3 b, float r )
{
  vec3 q = abs(p) - b;
  return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0) - r;
}

float map(vec3 p)
{
    vec3 p0 = p;
    p0.xz *= R_obj2;
    
    float d0=rect(p-light_pos, vec3(light_width, light_height, 0.));
    
    float d1;
    if (abs(p.y + .5) > .015) d1 = abs(p.y+.5);
    else d1=abs(p.y+0.5+texture(iChannel1, p.xz).x*.01)*.9;
    
    float d2=sphere(p-vec3(-0.4, -0.2, 1.2), 0.3);
    float d3=sdRoundBox(p0-vec3(.7, -.3, 1.2), vec3(.4, .2, .3), 0.);
    
   	float d = d0;
    object_id = LIGHT;
    
    if(d > d1)
    {
        d = d1;
        object_id=FLOOR;
    }
    
    if(d > d2)
    {
        d = d2;
        object_id=OBJ1;
    }
    
    if(d > d3)
    {
        d = d3;
        object_id=OBJ2;
    }
    
    return d;
}

vec3 get_normal(vec3 p) {
	const vec2 e = vec2(0.002, 0);
	return normalize(vec3(map(p + e.xyy)-map(p - e.xyy), 
                          map(p + e.yxy)-map(p - e.yxy),	
                          map(p + e.yyx)-map(p - e.yyx)));
}

float intersect( in vec3 ro, in vec3 rd )
{
    float t = 0.01;
    for( int i=0; i<128; i++ )
    {
        float c = map(ro + rd*t);
        if( c < 0.005 ) break;
        t += c;
        if( t>50.0 ) return -1.0;
    }
    return t;
}


// Linearly Transformed Cosines 

float IntegrateEdge(vec3 v1, vec3 v2)
{
    float cosTheta = dot(v1, v2);
    float theta = acos(cosTheta);    
    float res = cross(v1, v2).z * ((theta > 0.001) ? theta/sin(theta) : 1.0);

    return res;
}

void ClipQuadToHorizon(inout vec3 L[5], out int n)
{
    // detect clipping config
    int config = 0;
    if (L[0].z > 0.0) config += 1;
    if (L[1].z > 0.0) config += 2;
    if (L[2].z > 0.0) config += 4;
    if (L[3].z > 0.0) config += 8;

    // clip
    n = 0;

    if (config == 0)
    {
        // clip all
    }
    else if (config == 1) // V1 clip V2 V3 V4
    {
        n = 3;
        L[1] = -L[1].z * L[0] + L[0].z * L[1];
        L[2] = -L[3].z * L[0] + L[0].z * L[3];
    }
    else if (config == 2) // V2 clip V1 V3 V4
    {
        n = 3;
        L[0] = -L[0].z * L[1] + L[1].z * L[0];
        L[2] = -L[2].z * L[1] + L[1].z * L[2];
    }
    else if (config == 3) // V1 V2 clip V3 V4
    {
        n = 4;
        L[2] = -L[2].z * L[1] + L[1].z * L[2];
        L[3] = -L[3].z * L[0] + L[0].z * L[3];
    }
    else if (config == 4) // V3 clip V1 V2 V4
    {
        n = 3;
        L[0] = -L[3].z * L[2] + L[2].z * L[3];
        L[1] = -L[1].z * L[2] + L[2].z * L[1];
    }
    else if (config == 5) // V1 V3 clip V2 V4) impossible
    {
        n = 0;
    }
    else if (config == 6) // V2 V3 clip V1 V4
    {
        n = 4;
        L[0] = -L[0].z * L[1] + L[1].z * L[0];
        L[3] = -L[3].z * L[2] + L[2].z * L[3];
    }
    else if (config == 7) // V1 V2 V3 clip V4
    {
        n = 5;
        L[4] = -L[3].z * L[0] + L[0].z * L[3];
        L[3] = -L[3].z * L[2] + L[2].z * L[3];
    }
    else if (config == 8) // V4 clip V1 V2 V3
    {
        n = 3;
        L[0] = -L[0].z * L[3] + L[3].z * L[0];
        L[1] = -L[2].z * L[3] + L[3].z * L[2];
        L[2] =  L[3];
    }
    else if (config == 9) // V1 V4 clip V2 V3
    {
        n = 4;
        L[1] = -L[1].z * L[0] + L[0].z * L[1];
        L[2] = -L[2].z * L[3] + L[3].z * L[2];
    }
    else if (config == 10) // V2 V4 clip V1 V3) impossible
    {
        n = 0;
    }
    else if (config == 11) // V1 V2 V4 clip V3
    {
        n = 5;
        L[4] = L[3];
        L[3] = -L[2].z * L[3] + L[3].z * L[2];
        L[2] = -L[2].z * L[1] + L[1].z * L[2];
    }
    else if (config == 12) // V3 V4 clip V1 V2
    {
        n = 4;
        L[1] = -L[1].z * L[2] + L[2].z * L[1];
        L[0] = -L[0].z * L[3] + L[3].z * L[0];
    }
    else if (config == 13) // V1 V3 V4 clip V2
    {
        n = 5;
        L[4] = L[3];
        L[3] = L[2];
        L[2] = -L[1].z * L[2] + L[2].z * L[1];
        L[1] = -L[1].z * L[0] + L[0].z * L[1];
    }
    else if (config == 14) // V2 V3 V4 clip V1
    {
        n = 5;
        L[4] = -L[0].z * L[3] + L[3].z * L[0];
        L[0] = -L[0].z * L[1] + L[1].z * L[0];
    }
    else if (config == 15) // V1 V2 V3 V4
    {
        n = 4;
    }
    
    if (n == 3)
        L[3] = L[0];
    if (n == 4)
        L[4] = L[0];
}


vec3 LTC_Evaluate(vec3 N, vec3 V, vec3 P, mat3 Minv, vec3 points[4])
{
    // construct orthonormal basis around N
    vec3 T1, T2;
    T1 = normalize(V - N*dot(V, N));
    T2 = cross(N, T1);

    // rotate area light in (T1, T2, N) basis
    Minv = Minv * transpose(mat3(T1, T2, N));

    // polygon (allocate 5 vertices for clipping)
    vec3 L[5];
    L[0] = Minv * (points[0] - P);
    L[1] = Minv * (points[1] - P);
    L[2] = Minv * (points[2] - P);
    L[3] = Minv * (points[3] - P);

    int n=0;
    // The integration is assumed on the upper hemisphere
    // so we need to clip the frustum, the clipping will add 
    // at most 1 edge, that's why L is declared 5 elements.
    ClipQuadToHorizon(L, n);
    
    if (n == 0)
        return vec3(0, 0, 0);

    // project onto sphere
    vec3 PL[5];
    PL[0] = normalize(L[0]);
    PL[1] = normalize(L[1]);
    PL[2] = normalize(L[2]);
    PL[3] = normalize(L[3]);
    PL[4] = normalize(L[4]);

    // integrate for every edge.
    float sum = 0.0;

    sum += IntegrateEdge(PL[0], PL[1]);
    sum += IntegrateEdge(PL[1], PL[2]);
    sum += IntegrateEdge(PL[2], PL[3]);
    if (n >= 4)
        sum += IntegrateEdge(PL[3], PL[4]);
    if (n == 5)
        sum += IntegrateEdge(PL[4], PL[0]);

    sum =  max(0.0, sum);
    
    // Calculate colour
    vec3 e1 = normalize(L[0] - L[1]);
    vec3 e2 = normalize(L[2] - L[1]);
    vec3 N2 = cross(e1, e2); // Normal to light
    vec3 V2 = N2 * dot(L[1], N2); // Vector to some point in light rect
    vec2 Tlight_shape = vec2(length(L[0] - L[1]), length(L[2] - L[1]));
    V2 = V2 - L[1];
    float b = e1.y*e2.x - e1.x*e2.y + .1; // + .1 to remove artifacts
	vec2 pLight = vec2((V2.y*e2.x - V2.x*e2.y)/b, (V2.x*e1.y - V2.y*e1.x)/b);
   	pLight /= Tlight_shape;
    pLight -= .5;
    pLight /= 2.5;
    pLight += .5;
    
    vec3 ref_col = texture(iChannel3, pLight).xyz;

    vec3 Lo_i = vec3(sum) * ref_col;

    return Lo_i;
}

    
/////////////////////////////////////////////


float softshadow( in vec3 ro, in vec3 rd, float k )
{
    float res = 1.0;
    float t = 0.001;
	float h = 1.0;
    for( int i=0; i<32; i++ )
    {
        h = map(ro + rd*t);
        if(object_id==LIGHT)break;
        res = min( res, k*h/t );
        if( res<0.001 ) break;
        t += h;
    }
    return clamp(res,0.1,1.0);
}

void LTC_shading(float roughness, 
                 vec3 N, 
                 vec3 V, 
                 vec3 pos, 
                 vec3[4] points, 
                 vec3 m_spec, 
                 vec3 m_diff, 
                 inout vec3 col)
{
    
    
    float theta = acos(dot(N, V));
    vec2 uv = vec2(roughness, theta/(0.5*PI)) * float(LUTSIZE-1.);   
    vec2 wx = uv/iChannelResolution[0].xy;
            
    // The offset to get correct interpolation
    uv += vec2(0.5);

    vec4 params = texture(iChannel0, (uv+vec2(MATRIX_PARAM_OFFSET, 0.0))/iChannelResolution[0].xy);

    // The inverse transformation matrix, so we 
    // can integrate cosine distribution over the polygon 
    // transformed by this matrix, instead of integrating the
    // complicated brdf over the untransformed polygon.
    mat3 Minv = mat3(
        vec3(  1,        0,      params.y),
        vec3(  0,     params.z,   0),
        vec3(params.w,   0,      params.x)
    );

    vec3 spec = LTC_Evaluate(N, V, pos, Minv, points)*m_spec;

    spec *= texture(iChannel0, uv/iChannelResolution[0].xy).x;

    vec3 diff = LTC_Evaluate(N, V, pos, mat3(1), points)*m_diff; 

    // shadow is not area light based here.
    float sha = softshadow( pos+0.01*N, normalize(light_pos-pos), 16.0 );

    col  = light_col*(m_spec*spec + m_diff*diff)*sha;
    col /= 2.0*PI;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{  
	vec2 q = fragCoord.xy / iResolution.xy;
    vec2 p = -1.0 + 2.0 * q;
    p.x *= iResolution.x/iResolution.y;
  
    vec3 lookat = vec3(0.0, -0.5*light_height, 0.);
    vec2 M = vec2(0.25,0.45);
    float c0 = cos(M.x * 3.);
    float c1 = cos(M.y * 3.);
    float s0 = sin(M.x * 3.);
    float s1 = sin(M.y * 3.);
    mat2 R1 = mat2(
                   	c0,-s0,
        			s0, c0
                  );
   	mat2 R2 = mat2(
        			c1,-s1,
        			s1, c1
                  );
	vec3 ro = vec3(2.5, 0., 0.);
    ro.xy *= R1;
    ro.yz *= R2;
    ro -= lookat;
    
    vec3 forward=normalize(lookat-ro);
    vec3 right=normalize(cross(forward, vec3(0.0, 1.0, 0.0)));
    vec3 up=normalize(cross(right, forward));
    
    vec3 rd=normalize(p.x*right + p.y*up + 2.*forward);
    
    
    vec3 points[4];
    
    // setup the four vertices for the rect light
    init_rect_points(points);

    
    float t=intersect(ro,rd);
    vec3 col=vec3(0.);
    
    if(t>-0.5)
    {
        
        vec3 pos = ro + rd * t;
        vec3 N = get_normal(pos);
        vec3 V = -rd;
        
        if (object_id == LIGHT)
        {
            vec2 screen_pos = (pos.xy + vec2(light_width, .5-light_pos.y))/(vec2(light_width, light_height) * 2.);
            col = texture(iChannel2, screen_pos).xyz;
        }
        else
        {
            vec3 diff; vec3 spec;
            float roughness;
            
            if(object_id == FLOOR)
            {            

                roughness = texture(iChannel1, pos.xz).x;
                roughness *= roughness;
                roughness += .2;

                diff = vec3(1., .6, .4);
                spec = texture(iChannel1, pos.xz).xyz;

            }

            else if (object_id == OBJ1)
            {
                roughness = 0.2;

                diff = vec3(1.);
                spec = vec3(1., 0., 0.);

            }

            else if (object_id == OBJ2)
            {
                roughness = 0.5;

                diff = vec3(1.);
                spec = vec3(1.);

            }

            LTC_shading(roughness, N, V, pos, points, diff, spec, col);
        }
        

    }
    col=pow(clamp(col,0.0,1.0),vec3(0.45));
    col*=pow(16.0*q.x*q.y*(1.0-q.x)*(1.0-q.y), 0.1);
    fragColor.xyz=col;
    
}