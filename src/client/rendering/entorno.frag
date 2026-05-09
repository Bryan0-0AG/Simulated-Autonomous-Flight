#version 450

uniform float u_time;
uniform vec2 u_resolution;
out vec4 fragColor;

// Hash function for randomness
float hash(vec2 p) {
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}

// Value noise with cubic interpolation
float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    
    vec2 u = f * f * (3.0 - 2.0 * f);

    return mix(mix(hash(i + vec2(0.0, 0.0)), 
                   hash(i + vec2(1.0, 0.0)), u.x),
               mix(hash(i + vec2(0.0, 1.0)), 
                   hash(i + vec2(1.0, 1.0)), u.x), u.y);
}

// Fractal Brownian Motion for layered cloud detail
float fbm(vec2 p) {
    float v = 0.0;
    float a = 0.5;
    vec2 shift = vec2(100.0);
    mat2 rot = mat2(cos(0.5), sin(0.5), -sin(0.5), cos(0.5));
    for (int i = 0; i < 5; ++i) {
        v += a * noise(p);
        p = rot * p * 2.0 + shift;
        a *= 0.5;
    }
    return v;
}

void main() {
    vec2 uv = gl_FragCoord.xy / u_resolution.xy;
    float aspect = u_resolution.x / u_resolution.y;
    
    // Wind Directional UV: Stretched on X to create "streaks"
    vec2 st = uv * vec2(aspect * 2.0, 8.0);
    
    // Domain Warping: Offset UVs by noise to create organic cloud flow
    float time = u_time * 0.3;
    vec2 q = vec2(0.0);
    q.x = fbm(st + vec2(time, time * 0.2));
    q.y = fbm(st + vec2(1.0));

    vec2 r = vec2(0.0);
    r.x = fbm(st + 1.0 * q + vec2(1.7, 9.2) + 0.15 * time);
    r.y = fbm(st + 1.0 * q + vec2(8.3, 2.8) + 0.126 * time);

    float f = fbm(st + r);

    // Daytime Sky Color Palette
    vec3 skyHigh    = vec3(0.30, 0.55, 0.92); // Clear blue sky
    vec3 skyLow     = vec3(0.55, 0.75, 0.95); // Light horizon blue
    vec3 cloudWhite = vec3(0.95, 0.97, 1.00); // Bright cloud white
    vec3 cloudGrey  = vec3(0.70, 0.75, 0.82); // Cloud shadow tint

    // Vertical gradient: brighter near bottom (horizon), deeper blue at top
    vec3 skyBase = mix(skyLow, skyHigh, uv.y * 0.8);
    
    // Layer clouds using noise
    vec3 color = mix(skyBase, cloudGrey, clamp(f * f * 3.0, 0.0, 0.6));
    color = mix(color, cloudWhite, clamp(length(q) * 0.5, 0.0, 0.4));
    
    // Bright cloud highlights drifting across the sky
    float streaks = pow(noise(st * vec2(0.5, 20.0) + vec2(u_time * 3.0, 0.0)), 10.0);
    color += vec3(1.0, 1.0, 1.0) * streaks * 0.25;

    // Subtle sun glow from upper-right corner
    float sunDist = length(uv - vec2(0.85, 0.9));
    color += vec3(1.0, 0.95, 0.8) * max(0.0, 0.15 - sunDist) * 2.0;

    // Softer vignette for daytime (less darkening at edges)
    float vignette = 1.0 - length(uv - 0.5) * 0.3;
    color *= vignette;

    fragColor = vec4(color, 1.0);
}