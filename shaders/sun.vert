#version 330 core

layout(location = 0) in vec2 aPos;   // -0.5..0.5 quad coordinates
out vec2 fragTexCoords;

uniform mat4 view;
uniform mat4 projection;

// ---- Hard-coded sky position direction ----
// Appears “in front” of camera, slightly up.
// Change these three values to move the sun around.
uniform vec3 sunDir;

// Sky radius (distance from camera)
const float radius = 1000.0;

// Size of the billboard quad in world units
const float size = 100.0;

void main()
{
    vec3 normalizedSunDir = normalize(sunDir);
    // Compute world center from direction
    vec3 center = normalizedSunDir * radius;

    // Remove camera translation from view matrix
    mat4 rotView = mat4(
        vec4(view[0].xyz, 0.0),
        vec4(view[1].xyz, 0.0),
        vec4(view[2].xyz, 0.0),
        vec4(0.0, 0.0, 0.0, 1.0)
    );

    // Camera right / up extracted from view matrix
    vec3 right = vec3(view[0][0], view[1][0], view[2][0]);
    vec3 up    = vec3(view[0][1], view[1][1], view[2][1]);

    // Offset vertex in world space (billboard)
    vec3 worldPos = center + (right * aPos.x + up * aPos.y) * size;

    // Now transform with rotation-only camera
    gl_Position = projection * rotView * vec4(worldPos, 1.0);

    // Basic UV
    fragTexCoords = aPos + vec2(0.5);
}
