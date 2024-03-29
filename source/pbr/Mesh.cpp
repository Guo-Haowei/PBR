#include "Mesh.h"

namespace pbr {

VertexOnlyMesh CreateCubeMesh(float scale) {
    VertexOnlyMesh cube;
    /**
     *        E__________________ H
     *       /|                 /|
     *      / |                / |
     *     /  |               /  |
     *   A/___|______________/D  |
     *    |   |              |   |
     *    |   |              |   |
     *    |   |              |   |
     *    |  F|______________|___|G
     *    |  /               |  /
     *    | /                | /
     *   B|/_________________|C
     *
     */
    enum VertexIndex { A = 0,
                       B = 1,
                       C = 2,
                       D = 3,
                       E = 4,
                       F = 5,
                       G = 6,
                       H = 7 };

    cube.vertices = {
        { -scale, +scale, +scale },  // A
        { -scale, -scale, +scale },  // B
        { +scale, -scale, +scale },  // C
        { +scale, +scale, +scale },  // D
        { -scale, +scale, -scale },  // E
        { -scale, -scale, -scale },  // F
        { +scale, -scale, -scale },  // G
        { +scale, +scale, -scale }   // H
    };

    cube.indices = {
        uvec3(A, B, D),  // ABD
        uvec3(D, B, C),  // DBC
        uvec3(E, H, F),  // EHF
        uvec3(H, G, F),  // HGF

        uvec3(D, C, G),  // DCG
        uvec3(D, G, H),  // DGH
        uvec3(A, F, B),  // AFB
        uvec3(A, E, F),  // AEF

        uvec3(A, D, H),  // ADH
        uvec3(A, H, E),  // AHE
        uvec3(B, F, G),  // BFG
        uvec3(B, G, C),  // BGC
    };

    return cube;
}

Mesh CreateSphereMesh(float radius, uint32_t widthSegment, uint32_t heightSegment) {
    Mesh sphere;

    const float phiStart = 0.0f;
    const float phiLength = TwoPi;
    const float thetaStart = 0.0f;
    const float thetaLength = Pi;

    vector<vector<uint32_t>> grid;
    uint32_t index = 0;

    for (uint32_t iy = 0; iy <= heightSegment; ++iy) {
        const float v = (float)iy / (float)heightSegment;

        grid.push_back(vector<uint32_t>(0));

        for (uint32_t ix = 0; ix <= widthSegment; ++ix) {
            const float u = (float)ix / (float)widthSegment;
            float x = radius * -1.0f * glm::cos(u * TwoPi) * glm::sin(v * Pi);
            float y = radius * glm::cos(v * Pi);
            float z = radius * glm::sin(u * TwoPi) * glm::sin(v * Pi);

            vec3 p { x, y, z };

            sphere.vertices.push_back({ p, glm::normalize(p) });
            // sphere.vertices.push_back({ p, vec2(u, v), glm::normalize(p) });
            grid.back().push_back(index++);
        }
    }

    for (uint32_t iy = 0; iy < heightSegment; ++iy) {
        for (uint32_t ix = 0; ix < widthSegment; ++ix) {
            const uint32_t a = grid[iy][ix + 1];
            const uint32_t b = grid[iy][ix];
            const uint32_t c = grid[iy + 1][ix];
            const uint32_t d = grid[iy + 1][ix + 1];

            if (iy != 0)
                sphere.indices.push_back(uvec3(b, a, d));
            if (iy != heightSegment - 1)
                sphere.indices.push_back(uvec3(c, b, d));
        }
    }

    return sphere;
}

}  // namespace pbr
