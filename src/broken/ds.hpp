#ifdef GPU_KENEL
struct Material{
    /* 0 - Standard diffuse color, 1 - Compute 'Chessboard' texture */
    int computeColorType;
    float4 color;
    float reflectivity;
    float refractivity;
};

struct Material createMaterial()
{
    struct Material m;
    m.color = (float4)(1,1,1,1);
    m.computeColorType = 0;
    m.reflectivity = 0;
    m.refractivity = 0;
    return m;
}

struct Sphere{
    struct Material* m;
    float3 pos;
    float radius;
};

struct Plane{
    struct Material* m;
    float3 normal;
    float3 point;
};

struct Ray{
    float3 origin;
    float3 dir;
};

struct Light{
    float3 pos;
    float3 dir;
    bool directional;
    float4 color;
};


struct Scene{
    struct Sphere spheres[10];
    int spheresCount;

    struct Plane planes[10];
    int planesCount;

    struct Light lights[10];
    int lightsCount;

    struct Material standardMaterial;
};
#endif

