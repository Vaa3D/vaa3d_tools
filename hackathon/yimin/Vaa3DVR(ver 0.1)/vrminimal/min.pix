#version 410 // -*- c++ -*-

const float PI = 3.1415927;

in Varying {
    vec3            normal;
    vec2            texCoord;
    vec3            position;
    vec3            tangent;
    flat float      tangentYSign;
} interpolated;

layout(shared, row_major) uniform Uniform {
    mat3x3          objectToWorldNormalMatrix;
    mat4x4          objectToWorldMatrix;
    mat4x4          modelViewProjectionMatrix;
    vec3            light;
    vec3            cameraPosition;
} object;

uniform sampler2D   colorTexture;

out vec4            pixelColor;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// The following BRDF is based on https://github.com/wdas/brdf/blob/master/src/brdfs/disney.brdf
// It is overkill for this demo and isn't optimized for real-time rendering, but shows something
// of what modern shading code looks like.

float square(float x) { return x * x; }

float SchlickFresnel(float u) {
    // pow(m,5)
    float m = clamp(1.0 - u, 0.0, 1.0);
    return square(square(m)) * m;
}

float GTR1(float NdotH, float a) {
    if (a >= 1.0) { return 1.0 / PI; }
    float a2 = square(a);
    float t = 1.0 + (a2 - 1.0) * NdotH * NdotH;
    return (a2 - 1.0) / (PI * log(a2) * t);
}

float GTR2(float NdotH, float a) {
    float a2 = square(a);
    float t = 1.0 + (a2 - 1.0) * NdotH * NdotH;
    return a2 / (PI * t * t);
}

float GTR2_aniso(float NdotH, float HdotX, float HdotY, float ax, float ay) {
    return 1.0 / ( PI * ax * ay * square(square(HdotX / ax) + square(HdotY / ay) + NdotH * NdotH ));
}

float SmithG_GGX(float Ndotv, float alphaG) {
    float a = alphaG * alphaG;
    float b = Ndotv * Ndotv;
    return 1.0 / (Ndotv + sqrt(a + b - a * b));
}

// See http://blog.selfshadow.com/publications/s2012-shading-course/burley/s2012_pbs_disney_brdf_notes_v3.pdf
// and https://github.com/wdas/brdf/blob/master/src/brdfs/disney.brdf
// for documentation of material parameters. Unlike their reference code, our baseColor is in linear
// space (not gamma encoded)
//
// L is the unit vector to the light source (omega_in) in world space
// N is the unit normal in world space
// V is the vector to the eye (omega_out) in world space
// X and Y are the tangent directions in world space
vec3 evaluateDisneyBRDF
   (vec3    baseColor,
    float   metallic,
    float   subsurface,
    float   specular,
    float   roughness,
    float   specularTint, 
    float   anisotropic,
    float   sheen,
    float   sheenTint,
    float   clearcoat, 
    float   clearcoatGloss,
    vec3    L,
    vec3    V,
    vec3    N,
    vec3    X,
    vec3    Y) {

    float NdotL = dot(N, L);
    float NdotV = dot(N, V);
    if (NdotL < 0 || NdotV < 0) return vec3(0);

    vec3 H = normalize(L + V);
    float NdotH = dot(N, H);
    float LdotH = dot(L, H);

    float luminance = dot(baseColor, vec3(0.3, 0.6, 0.1));

    // normalize luminance to isolate hue and saturation components
    vec3 Ctint = (luminance > 0.0) ? baseColor / luminance : vec3(1.0); 
    vec3 Cspec0 = mix(specular * 0.08 * mix(vec3(1.0), Ctint, specularTint), baseColor, metallic);
    vec3 Csheen = mix(vec3(1.0), Ctint, sheenTint);

    // Diffuse fresnel - go from 1 at normal incidence to .5 at grazing
    // and mix in diffuse retro-reflection based on roughness
    float FL = SchlickFresnel(NdotL), FV = SchlickFresnel(NdotV);
    float Fd90 = 0.5 + 2.0 * LdotH * LdotH * roughness;
    float Fd = mix(1, Fd90, FL) * mix(1, Fd90, FV);

    // Based on Hanrahan-Krueger BRDF approximation of isotropic BSSRDF
    // 1.25 scale is used to (roughly) preserve albedo
    // Fss90 used to "flatten" retroreflection based on roughness
    float Fss90 = LdotH * LdotH * roughness;
    float Fss = mix(1.0, Fss90, FL) * mix(1.0, Fss90, FV);
    float ss = 1.25 * (Fss * (1 / (NdotL + NdotV) - 0.5) + 0.5);

    // Specular
    float aspect = sqrt(1.0 - anisotropic * 0.9);
    float ax = max(0.001, square(roughness) / aspect);
    float ay = max(0.001, square(roughness) * aspect);
    float Ds = GTR2_aniso(NdotH, dot(H, X), dot(H, Y), ax, ay);
    float FH = SchlickFresnel(LdotH);
    vec3 Fs = mix(Cspec0, vec3(1.0), FH);
    float roughg = square(roughness * 0.5 + 0.5);
    float Gs = SmithG_GGX(NdotL, roughg) * SmithG_GGX(NdotV, roughg);

    // sheen
    vec3 Fsheen = FH * sheen * Csheen;

    // clearcoat (ior = 1.5 -> F0 = 0.04)
    float Dr = GTR1(NdotH, mix(0.1, 0.001, clearcoatGloss));
    float Fr = mix(0.04, 1.0, FH);
    float Gr = SmithG_GGX(NdotL, 0.25) * SmithG_GGX(NdotV, 0.25);

    return ((1.0 / PI) * mix(Fd, ss, subsurface) * baseColor + Fsheen) * (1.0 - metallic) + 
        Gs * Fs * Ds + 0.25 * clearcoat * Gr * Fr * Dr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

// Normalized Blinn-Phong glossy model, applying Fresnel
vec3 evaluateBlinnPhongBSDF
   (vec3    baseColor,
    float   metallic,
    float   subsurface,
    float   specular,
    float   roughness,
    float   specularTint, 
    float   anisotropic,
    float   sheen,
    float   sheenTint,
    float   clearcoat, 
    float   clearcoatGloss,
    vec3    w_i,
    vec3    w_o,
    vec3    n,
    vec3    tangentX,
    vec3    tangentY) {

    // Material parameter mapping
    float exponent = 2.0 / square(roughness) + 2.0;
    vec3 glossyReflectivity     = mix(vec3(1.0), baseColor, max(specularTint, metallic)) * min(1.0, (metallic * 0.9 + 0.1) + specular);
    vec3 lambertianReflectivity = baseColor * min(subsurface + (1.0 - metallic), 1.0);

    // Angular terms
    vec3 w_h = normalize(w_i + w_o);
    float cos_i = max(0.0, dot(n, w_i));
    float cos_h = max(0.0, dot(n, w_h));
    float normalizedGlossyLobe = (((exponent + 8.0) / 8.0) * pow(cos_h, exponent));

    // Fresnel
    vec3 Fg = mix(glossyReflectivity, vec3(1.0), SchlickFresnel(cos_h));

    // Diffuse fresnel - go from 1 at normal incidence to .5 at grazing
    // and mix in diffuse retro-reflection based on roughness
    float FL = SchlickFresnel(cos_i), FV = SchlickFresnel(max(0.0, dot(w_o, n)));
    float Fd90 = 0.5 + 2.0 * cos_i * cos_h * roughness;
    float Fd = mix(1, Fd90, FL) * mix(1, Fd90, FV);

    return (Fd * lambertianReflectivity + Fg * normalizedGlossyLobe) * cos_i / PI;
}


void main () {
    // The normal is also the z-axis of tangent space
    vec3 normal   = normalize(interpolated.normal);
    vec3 tangentX = normalize(interpolated.tangent);
    vec3 tangentY = normalize(cross(interpolated.normal, interpolated.tangent)) * interpolated.tangentYSign;

    vec3 baseColor = texture(colorTexture, interpolated.texCoord).rgb;

    // Disney BRDF parameters
    const float metallic       = 0.0;
    const float subsurface     = 0.2;
    const float specular       = 0.5;
    const float smoothness     = 0.5;
    float       roughness      = square(1.0 - smoothness);
    const float specularTint   = 0.0;
    const float anisotropic    = 0.0;
    const float sheen          = 0.2;
    const float sheenTint      = 0.0;
    const float clearcoat      = 0.0;
    const float clearcoatGloss = 0.0;

    vec3 view            = normalize(object.cameraPosition - interpolated.position);
    vec3 lightRadiance   = vec3(0.8, 0.75, 0.68) * 4.0;
    vec3 ambientRadiance = mix(vec3(0.2), vec3(0.1, 0.25, 0.35), normal.y * 0.5 + 0.5) * 2.5 / PI;

    vec3 bsdf =
        evaluateBlinnPhongBSDF
        //evaluateDisneyBRDF
        (baseColor, metallic, subsurface, specular, roughness, specularTint, anisotropic, sheen, sheenTint, clearcoat, clearcoatGloss, object.light, view, normal, tangentX, tangentY);

    // Direct light plus a very coarse ambient term, ignorant of BSDF
    vec3 radiance = lightRadiance * bsdf + ambientRadiance * baseColor;
    pixelColor.rgb = pow(radiance, vec3(1.0 / 2.2));

    // Debugging code:
    // pixelColor.rgb = interpolated.normal * 0.5 + 0.5 + tangentX * 0.0001 + baseColor.x * 0.0001;
    // pixelColor.rgb = object.objectToWorldNormalMatrix[2] * 0.5 + 0.5 + 0.0001 * pixelColor.rgb;

    pixelColor.a = 1.0;
}

