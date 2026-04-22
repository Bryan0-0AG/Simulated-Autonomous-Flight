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

// Fractal Brownian Motion for layered detail
float fbm(vec2 p) {
    float v = 0.0;
    float a = 0.5;
    vec2 shift = vec2(100.0);
    // Rotating noise to avoid directional artifacts
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
    
    // Domain Warping: Offset UVs by noise to create organic flow/turbulence
    float time = u_time * 0.5;
    vec2 q = vec2(0.0);
    q.x = fbm(st + vec2(time, time * 0.2));
    q.y = fbm(st + vec2(1.0));

    vec2 r = vec2(0.0);
    r.x = fbm(st + 1.0 * q + vec2(1.7, 9.2) + 0.15 * time);
    r.y = fbm(st + 1.0 * q + vec2(8.3, 2.8) + 0.126 * time);

    float f = fbm(st + r);

    // Color Palette
    vec3 backgroundColor = vec3(0.02, 0.04, 0.08); // Deep Navy
    vec3 windColor1 = vec3(0.1, 0.3, 0.5);      // Mid Blue
    vec3 windColor2 = vec3(0.4, 0.7, 0.9);      // Light Cyan
    
    // Mix colors based on noise results
    vec3 color = mix(backgroundColor, windColor1, clamp(f * f * 4.0, 0.0, 1.0));
    color = mix(color, windColor2, clamp(length(q), 0.0, 1.0) * 0.4);
    
    // Add "Air Streaks" - high frequency detail moving fast
    float streaks = pow(noise(st * vec2(0.5, 20.0) + vec2(u_time * 4.0, 0.0)), 12.0);
    color += vec3(0.5, 0.8, 1.0) * streaks * 0.3;

    // Vignette/Depth effect
    float vignette = 1.0 - length(uv - 0.5) * 0.8;
    color *= vignette;

    fragColor = vec4(color, 1.0);
}
