const float gaussianWeight[4] = { 0.25, 0.125, 0.0625, 0.03125 };

uint childIndices[8];
uvec4 childBrickIndices[8];

void loadChildTile(int chlidIdx) {
    for (int i = 0; i < 8; ++i) {
        childIndices[i] = imageLoad(u_octreeNodeIdx, chlidIdx + i).x;
        childBrickIndices[i] = imageLoad(u_octreeNodeBrickIdx, chlidIdx + i);
    }
}

vec4 getColor(ivec3 pos) {
    ivec3 childPos = ivec3(round(vec3(pos) / 4.0));
    int offset = childPos.x + 2 * childPos.y + 4 * childPos.z;
    if ((childIndices[offset] & NODE_MASK_INDEX) == 0 && u_level != u_octreeLevel - 2)
        return u_emptyColor;

    ivec3 localPos = pos - 2 * childPos;
    ivec3 childBrickAddress = ivec3(childBrickIndices[offset].xyz);
    uint val = imageLoad(u_octreeBrickValue, childBrickAddress + localPos);
    return convRGBA8ToVec4(val);
}

vec4 mipmapIsotropic(ivec3 pos) {
    vec4 col = vec4(0);
    float weightSum = 0.0;

    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            for (int z = -1; z <= 1; ++z) {

                ivec3 lookupPos = pos + ivec3(x, y, z);

                if (lookupPos.x >= 0 && lookupPos.y >= 0 && lookupPos.z >= 0 &&
                    lookupPos.x <= 4 && lookupPos.y <= 4 && lookupPos.z <= 4)
                {
                    int manhattanDist = abs(x) + abs(y) + abs(z);
                    float weight = gaussianWeight[manhattanDist];
                    vec4 lookupColor = getColor(lookupPos);

                    col += weight * lookupColor;
                    weightSum += weight;
                }
            }
        }
    }

    return col / weightSum;
}