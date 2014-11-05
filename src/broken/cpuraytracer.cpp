#define GPU_KENEL
#include "ds.hpp"
#include "const.hpp"

const bool kFullscreen = true;
float viewMatrix[16];

float sphere1Pos[3] = {0,0,10};
float sphere2Pos[3] = {0,0,-10};
float sphereVelocity = 1;
float sphereTransforms[2][16];

enum RayType{
    ORIGIN,
    REFRACTRAY,
    REFLECTRAY
};

float4 raytracer(Ray* ray, Scene* scene)
{
    bool bDone = false;
    ProgramLabels pc = ENTRY_POINT;
    float4 finalValue;

    while(!bDone)
    {
        switch (pc)
        {
            case ENTRY_POINT:
            case PRE_FACTORIAL_FUNCTION_CALL:
                Node frame;
                frame.ray = *ray;
                frame.pc = POST_FACTORIAL_FUNCTION_CALL;
                frame.parent = NULL;

                frame.colorType = ORIGIN;
                frame.m = NULL;

                frame.diffuseColor = (float4)(0,0,0, 0);
                frame.refractColor = (float4)(0,0,0, 0);

                stackPush(pStack, frame);

                pc = FACTORIAL_FUNCTION;
                break;

            case POST_FACTORIAL_FUNCTION_CALL:
                Node* currentframe = stackPop(pStack, 0);
                stackPop(pStack);

            case EXIT_POINT:
                // mark that we've done
                bDone = true;
                break;

            case FACTORIAL_FUNCTION:
                {
                    Node* currframe = stackTop(pStack, 0);

                    if(currframe->traceDepth > 5)
                    {
                        pc = currentframe->pc;
                        break;
                    }
                }

            case PRE_RECURSIVE_FACTORIAL_FUNCTION_CALL:
                Node* currentframe = stackTop(pStack, 0);
                Ray localRay = currentframe->ray;

                void* intersectObj = 0;
                int intersectObjType = 0;
                float t = intersect(localRay, scene, &intersectObj, &intersectObjType);

                if ( t < kMaxRenderDist ){
                    float3 intersectPos = locaRay->origin+localRay->dir*t ;
                    float3 normal;

                    struct Material* m = 0;

                    if ( intersectObjType == 1 ){
                        normal = normalize(intersectPos-((struct Sphere*)intersectObj)->pos);
                        m = ((struct Sphere*)intersectObj)->m;
                    } else if (intersectObjType == 2 ){
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
                        reflectRay.dir = R;

                        Node nframe;
                        nframe.intersectPoint = intersectPoint;
                        nframe.ray = reflectRay;
                        nframe.parent = currentframe;
                        nframe.diffuseColor = diffuseColor;
                        nframe.reflectratio = m.reflectivity;
                        nframe.refractratio = 0;
                        nframe.reflectColor = (float4)(0,0,0,0);
                        nframe.refractColor =  (float4)(0,0,0,0);
                        nframe.traceDepth = currentframe->traceDepth + 1;
                        nframe.pc = POST_RECURSIVE_FACTORIAL_FUNCTION_CALL ;

                        stackPush(pStack, nframe);
                    }

                    if ( traceDepth < kMaxTraceDepth && m->refractivity > 0 ){
                        struct Ray refractRay;
                        float3 R = refract(ray->dir, normal, 0.6);
                        if ( dot(R,normal) < 0 ){
                            refractRay.origin = intersectPos + R*0.001;
                            refractRay.dir    = R;

                            Node nframe;
                            nframe.intersectPoint = intersectPoint;
                            nframe.ray = refractRay;
                            nframe.parent = currentframe;
                            nframe.diffuseColor = diffuseColor;
                            nframe.refractratio = m.reflectivity;
                            nframe.refractColor = (float4)(0,0,0,0);
                            nframe.reflectratio = 0;
                            nframe.reflectColor = (float4)(0,0,0,0);
                            nframe.m = &m;
                            nframe.traceDepth = currentframe->traceDepth + 1;
                            nframe.pc = POST_RECURSIVE_FACTORIAL_FUNCTION_CALL ;
                            stackPush( pStack, nframe);
                        }
                    }
                }

                pc = FACTORIAL_FUNCTION;
                break;

            case POST_RECURSIVE_FACTORIAL_FUNCTION_CALL:
                {
                    Node* currentframe = stackTop(pStack, 0);

                    float4 diffuseColor = currentframe->diffuseColor;

                    float4 color = (float4)(0,0,0,0);
                    color += currentframe->refractivity * currentframe->refractcolor;
                    color += currentframe->reflectivity * currentframe->reflectcolor;

                    for(int i = 0; i < scene->lightsCount; i++){
                        float3 L = scene->lights[i].dir;
                        float lightDist = kMaxRenderDist;
                        if ( !scene->lights[i].directional ){
                            L = scene->lights[i].pos - currentframe->intersectPos ;
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

                    if (currentframe->type == REFRACTRAY){
                        currentframe->parent->refract += color;
                    }else if (currentframe->type == REFLECTRAY){
                        currentframe->parent->reflect += color;
                    }else{ // original ray
                        finalValue = color;
                    }           

                    // "jump" to return label
                    stackPop(pStack, 1);
                    pc = stackTop(pStack,0)->pc;
                    break;
                }

            default:
                // should never get here so just abort
                bDone = true;
                break;
        }
    }

    // return the final calculated value
    return finalValue;


}


int main() {
}

