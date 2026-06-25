@vs vs
in vec2 position;
in vec4 color;
out vec4 frag_color;
out vec2 Position;

layout(binding = 0) uniform project_uniform { 
    mat4 proj;
};

void main() {
    gl_Position = proj * vec4(position, 0.0, 1.0);
    frag_color = color;
    Position = position;
}
@end

@fs fs
in vec4 frag_color;
in vec2 Position;
out vec4 out_color;

layout(binding = 1) uniform radius_uniform {
    vec2 half_size; // center of circle
    float top_anchor;
    float radius;
};

void main() {
    vec2 local_pos = Position - vec2(half_size.x, top_anchor + half_size.y);
    vec2 q = abs(local_pos) - half_size + radius;
    float dist = length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - radius;
    float px = fwidth(dist) * 0.5;
    float alpha = 1.0 - smoothstep(-px, px, dist);
    out_color = vec4(frag_color.rgb, frag_color.a * alpha);
}
@end

@program island vs fs
