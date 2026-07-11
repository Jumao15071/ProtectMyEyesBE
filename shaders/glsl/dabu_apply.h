#include "dabu_config.h"

//math
float saturate(float x) {
    return clamp(x, 0.0, 1.0);
}

vec2 saturate(vec2 x) {
    return clamp(x, vec2(0.0), vec2(1.0));
}

vec3 saturate(vec3 x) {
    return clamp(x, vec3(0.0), vec3(1.0));
}

vec3 render_chunk_bound(vec3 oc, vec3 cp) {
    vec3 fcp = fract(cp);

    if (((cp.x < 0.0625 || cp.x > 15.9375) && (cp.y < 0.0625 || cp.y > 15.9375)) ||
        ((cp.x < 0.0625 || cp.x > 15.9375) && (cp.z < 0.0625 || cp.z > 15.9375)) ||
        ((cp.y < 0.0625 || cp.y > 15.9375) && (cp.z < 0.0625 || cp.z > 15.9375)))
    {
        oc = mix(oc, vec3(0.0, 0.0, 1.0), 0.2);
    }
    else if (((cp.x < 0.09375 || cp.x > 15.90625) || (cp.z < 0.09375 || cp.z > 15.90625)) &&
             (((fcp.x < 0.03125 || fcp.x > 0.96875) && (fcp.y < 0.03125 || fcp.y > 0.96875)) ||
              ((fcp.x < 0.03125 || fcp.x > 0.96875) && (fcp.z < 0.03125 || fcp.z > 0.96875)) ||
              ((fcp.y < 0.03125 || fcp.y > 0.96875) && (fcp.z < 0.03125 || fcp.z > 0.96875))))
    {
        oc = (oc / 0.4) * (vec3(1.0) - oc);
    }
    return oc;
}


bool get_digit_shape(int d, vec3 p)
{
    bool xL = (p.x >= 0.25 && p.x <= 0.35);
    bool xM = (p.x >= 0.35 && p.x <= 0.45);
    bool xR = (p.x >= 0.45 && p.x <= 0.55);

    bool zB = (p.z >= 0.25 && p.z <= 0.35); // Bottom
    bool zC = (p.z >= 0.45 && p.z <= 0.55); // Center
    bool zT = (p.z >= 0.65 && p.z <= 0.75); // Top
    
    bool xLR = (p.x >= 0.25 && p.x <= 0.55);
    bool xLM = (p.x >= 0.25 && p.x <= 0.45);
    bool xMR = (p.x >= 0.35 && p.x <= 0.55);
    bool zBT = (p.z >= 0.25 && p.z <= 0.75); 

    int mask = 0;
    mask |= int( (xL && zBT) || (xR && zBT) || (xM && zT) || (xM && zB) )   << 0;  // 0
    mask |= int( (xLR && zB) || (xM && p.z >= 0.35 && p.z <= 0.75) || (xR && zT) ) << 1;  // 1
    mask |= int( (xM && zC) || (xL && p.z >= 0.45 && p.z <= 0.75) || (xR && p.z >= 0.25 && p.z <= 0.55) || (xLM && zB) || (xMR && zT) ) << 2;  // 2
    mask |= int( (xL && zBT) || (xMR && zC) || (xMR && zB) || (xMR && zT) ) << 3;  // 3
    mask |= int( (xL && zBT) || (xR && p.z >= 0.45 && p.z <= 0.75) || (xM && zC) ) << 4;  // 4
    mask |= int( (xM && zC) || (xL && p.z >= 0.25 && p.z <= 0.55) || (xR && p.z >= 0.45 && p.z <= 0.75) || (xLM && zT) || (xMR && zB) ) << 5;  // 5
    mask |= int( (xM && zC) || (xL && p.z >= 0.25 && p.z <= 0.55) || (xR && p.z >= 0.35 && p.z <= 0.75) || (xLM && zT) || (xMR && zB) ) << 6;  // 6
    mask |= int( (xL && zBT) || (xMR && zT) || (xR && p.z >= 0.60 && p.z <= 0.65) ) << 7;  // 7
    mask |= int( (xM && zC) || (xL && zBT) || (xR && zBT) || (xM && zT) || (xM && zB) ) << 8;  // 8
    mask |= int( (xM && zC) || (xL && p.z >= 0.35 && p.z <= 0.75) || (xR && p.z >= 0.45 && p.z <= 0.75) || (xLR && zB) || (xMR && zT) ) << 9;  // 9

    return ((mask >> d) & 1) != 0;
}

bool get_number_match(int L, vec3 bp)
{
    if (L < 0) return false;
    if (L <= 9) return get_digit_shape(L, bp);

    bool tens = (bp.x <= 0.7 && bp.x >= 0.4 && bp.z <= 0.35 && bp.z >= 0.25)
             || (bp.x <= 0.6 && bp.x >= 0.5 && bp.z <= 0.75 && bp.z >= 0.35)
             || (bp.x <= 0.7 && bp.x >= 0.6 && bp.z <= 0.75 && bp.z >= 0.65);

    return tens || get_digit_shape(L - 10, bp + vec3(0.15, 0.0, 0.0));
}

void red_stone_level(inout bool needDiscard, inout bool isRedstoneDust, inout vec4 diffuse, vec3 cp, vec3 color)
{
    if (color.r <= color.g + color.b || color.g >= 7.0/32.0) return;

    int L = int(floor(max(0.0, color.r * 25.0 - 9.5)));
    L = clamp(L, 0, 15);

    isRedstoneDust = true;

    bool match = (cp.x <= 0.7 && cp.x >= 0.1 && cp.z <= 0.08 && cp.z >= 0.05)
              || (cp.x <= 0.7 && cp.x >= 0.1 && cp.z <= 0.20 && cp.z >= 0.17)
              || (mod(floor(float(L) / exp2(floor(6.667 * (cp.x - 0.1)))), 2.0) >= 0.5 && cp.z <= 0.15 && cp.z >= 0.1);

    if (match || get_number_match(L, cp))
    {
        diffuse = vec4(1.0, 1.0, 1.0, 1.0);
        needDiscard = false;
    }
}

void light_overlay(inout bool needDiscard, inout int isLightOverlay, vec3 cp, vec2 uv1)
{
    ivec2 lightLevel = ivec2(uv1 * 16.0 + 0.001);

    if (uv1.x + 0.001 > 0.0615 && uv1.x + 0.001 < 0.0625)
    {
        isLightOverlay = 5;
    }
    else if (uv1.y + 0.001 > 0.4995 && uv1.y + 0.001 < 0.5005)
    {
        isLightOverlay = 4;
    }
}