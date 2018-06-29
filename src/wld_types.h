
#ifndef WLD_TYPES_H
#define WLD_TYPES_H

#include <stdint.h>
#include "zeq_platform.h"

#ifdef ZEQ_COMPILER_MSVC
# pragma warning(disable: 4200)
#endif

#define WLD_FRAG_TEXTURE_NAME 0x03
#define WLD_FRAG_TEXTURE 0x04
#define WLD_FRAG_TEXTURE_REF 0x05
#define WLD_FRAG_SKELETON 0x10
#define WLD_FRAG_SKELETON_REF 0x11
#define WLD_FRAG_ANIMATION 0x12
#define WLD_FRAG_ANIMATION_REF 0x13
#define WLD_FRAG_MODEL 0x14
#define WLD_FRAG_MODEL_PLACEMENT 0x15
#define WLD_FRAG_MESH_REF 0x2d
#define WLD_FRAG_VERTEX_COLOR_REF 0x2f
#define WLD_FRAG_MATERIAL 0x30
#define WLD_FRAG_MATERIAL_LIST 0x31
#define WLD_FRAG_VERTEX_COLORS 0x32
#define WLD_FRAG_VERTEX_COLORS_REF 0x33
#define WLD_FRAG_MESH 0x36
#define WLD_FRAG_VERTEX_ANIMATION 0x37

#define wld_frag_length(frag) ((frag)->length + (sizeof(uint32_t) * 2))
#define wld_frag_type(frag) ((frag)->fragType)
#define wld_frag_is_type(frag, type) ((frag)->fragType == (type))

#pragma pack(1)

typedef union WldVec2 {
    struct {
        float x, y;
    };
    struct {
        float u, v;
    };
    float array[2];
} WldVec2;

typedef union WldVec3 {
    struct {
        float x, y, z;
    };
    struct {
        float u, v, w;
    };
    float array[3];
} WldVec3;

typedef struct WldColor {
    uint8_t b, g, r, a;
} WldColor;

typedef struct WldVertex {
    int16_t x, y, z;
} WldVertex;

typedef struct WldTexCoord16 {
    int16_t u, v;
} WldTexCoord16;

typedef WldVec2 WldTexCoord32;

typedef struct WldNormal {
    int8_t x, y, z;
} WldNormal;

typedef struct WldTriangle {
    uint16_t    flag;
    uint16_t    indices[3];
} WldTriangle;

typedef struct WldMap {
    uint16_t    count;
    uint16_t    index;
} WldMap;

typedef struct WldFrag {
    uint32_t    length;
    uint32_t    fragType;
    int         nameRef;
} WldFrag;

typedef struct WldFragSimpleRef {
    WldFrag     frag;
    int         ref;
    uint32_t    flag;
} WldFragSimpleRef;

typedef WldFragSimpleRef WldFragTextureRef;
typedef WldFragSimpleRef WldFragSkeletonRef;
typedef WldFragSimpleRef WldFragMeshRef;
typedef WldFragSimpleRef WldFragVertexColorsRef;
typedef WldFragSimpleRef WldFragUnknownRef;

typedef struct WldFragTextureName {
    WldFrag     frag;
    uint32_t    flag;
    uint16_t    stringLength;
    char        string[0];
} WldFragTextureName;

typedef struct WldFragTexture {
    WldFrag     frag;
    uint32_t    flag;
    int         count;
    union {
        int     ref;
        struct {
            uint32_t    frameDelayMs;
            int         refList[0];
        } animated;
    };
} WldFragTexture;

typedef struct WldSubFragBone {
    int         nameRef;
    uint32_t    flag;
    int         refA;
    int         refB;
    int         size;
} WldSubFragBone;

typedef struct WldFragSkeleton {
    WldFrag     frag;
    uint32_t    flag;
    int         count;
    int         ref;
} WldFragSkeleton;

typedef struct WldSubFragAnimation {
    struct {
        int16_t w, x, y, z;
    } rotation;
    struct {
        int16_t x, y, z, denom;
    } shift;
} WldSubFragAnimation;

typedef struct WldFragAnimation {
    WldFrag             frag;
    uint32_t            flag;
    uint32_t            count;
    WldSubFragAnimation entries[0];
} WldFragAnimation;

typedef struct WldFragAnimationRef {
    WldFrag     frag;
    int         ref;
    uint32_t    flag;
    uint32_t    frameDelayMs;
} WldFragAnimationRef;

typedef struct WldFragModel {
    WldFrag     frag;
    uint32_t    flag;
    int         refA;
    int         skippableCount;
    int         meshRefCount;
    int         refB;
} WldFragModel;

typedef struct WldFragModelPlacement {
    WldFrag     frag;
    int         objNameRef;
    uint32_t    flag;
    int         refB;
    WldVec3     position;
    WldVec3     rotation;
    WldVec3     scale;
    int         refC;
    uint32_t    refCParam;
} WldFragModelPlacement;

typedef struct WldFragMaterial {
    WldFrag     frag;
    uint32_t    flag;
    uint32_t    blendMode;
    uint32_t    unknownA;       /* 0x00b2b2b2 (mobs, objects) or 0x004e4e4e (zones) */
    float       unknownB[2];    /* [0] = 0, 1, or 0.1; [1] = 0.75 or 0 */
    int         ref;
    int         unknownC[2];    /* Always seem to be 0 */
} WldFragMaterial;

typedef struct WldFragMaterialList {
    WldFrag     frag;
    uint32_t    flag;
    uint32_t    count;
    int         refList[0];
} WldFragMaterialList;

typedef struct WldFragVertexColors {
    WldFrag     frag;
    uint32_t    flag;
    uint32_t    count;
    uint32_t    unknown[3];
    WldColor    colors[0];
} WldFragVertexColors;

typedef struct WldFragMesh {
    WldFrag     frag;
    uint32_t    flag;
    int         materialListRef;
    int         animVertRef;
    int         unknownA[2];
    WldVec3     position;
    WldVec3     rotation;
    float       maxDist;
    WldVec3     min;
    WldVec3     max;
    uint16_t    vertexCount;
    uint16_t    uvCount;
    uint16_t    normalCount;
    uint16_t    colorCount;
    uint16_t    triangleCount;
    uint16_t    boneMappingCount;
    uint16_t    triangleTextureCount;
    uint16_t    vertexTextureCount;
    uint16_t    size;
    uint16_t    scale;
} WldFragMesh;

typedef struct WldFragVertexAnimation {
    WldFrag     frag;
    uint32_t    flag;
    uint16_t    vertexCount;
    uint16_t    frameCount;
    uint32_t    frameDelayMs;
    uint16_t    scale;
    WldVertex   vertices[0];
} WldFragVertexAnimation;

#pragma pack()

#endif/*WLD_TYPES_H*/
