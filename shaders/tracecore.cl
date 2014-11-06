#include "template.h"

float4 TEMPLATE(raytrace, iters)  (struct Ray* ray, struct Scene* scene,int traceDepth)
{
    void* intersectObj = 0;
    int intersectObjType = 0;
    float t = intersect( ray, scene, &intersectObj, &intersectObjType);

    float4 color = (float4)(0,0,0,0);
    if ( t < kMaxRenderDist ){
        float3 intersectPos = ray->origin+ray->dir*t ;
        float3 normal;

        struct Material* m = 0;

        if ( intersectObjType == 1 ){		
            normal = normalize(intersectPos-((struct Sphere*)intersectObj)->pos);
            m = ((struct Sphere*)intersectObj)->m;
        }
        else if (intersectObjType == 2 ){
            normal = ((struct Plane*)intersectObj)->normal;
            m = ((struct Plane*)intersectObj)->m;
        }

        if ( !m ){
            m = &scene->standardMaterial;
        }

        float4 diffuseColor = m->color;

        if ( m->computeColorType == 1){
            if ( (int)(intersectPos.x/5.0f) % 2 == 0 ){
                if ( (int)(intersectPos.z/5.0f) % 2 == 0 ){
                    diffuseColor = (float4)(0,0,0,0);
                }
            }
            else{
                if ( (int)(intersectPos.z/5.0f) % 2 != 0 ){
                    diffuseColor = (float4)(0,0,0,0);
                }
            }
        }
        if ( traceDepth < kMaxTraceDepth && m->reflectivity > 0 ){
            struct Ray reflectRay;
            float3 R = reflect(ray->dir, normal);
            reflectRay.origin = intersectPos + R*0.001;
            reflectRay.dir    = R;
            //diffuseColor += m->reflectivity* TEMPLATE(raytrace, iters+1)(&reflectRay, scene, traceDepth+1);
        }

        if ( traceDepth < kMaxTraceDepth && m->refractivity > 0 ){
            struct Ray refractRay;
            float3 R = refract(ray->dir, normal, 0.6);
            if ( dot(R,normal) < 0 ){
                refractRay.origin = intersectPos + R*0.001;
                refractRay.dir    = R;
                //diffuseColor = m->refractivity*TEMPLATE(raytrace, iters+1)(&refractRay, scene, traceDepth+1);
            }
        }

        for(int i = 0; i < scene->lightsCount; i++){
            float3 L = scene->lights[i].dir;
            float lightDist = kMaxRenderDist;
            if ( !scene->lights[i].directional ){
                L = scene->lights[i].pos - intersectPos ;
                lightDist = length(L);
                L = normalize(L);
            }

            float pointLit = 1;
            struct Ray shadowRay;
            shadowRay.origin = intersectPos + L*0.001;
            shadowRay.dir = L;
            t = intersect( &shadowRay, scene, &intersectObj, &intersectObjType);
            if ( t < lightDist ){
                pointLit = 0;
            }
            color += pointLit*diffuseColor*scene->lights[i].color*max(0.0f,dot(normal, L));
        }
    }
    return clamp(color,0.f,1.f);
}


