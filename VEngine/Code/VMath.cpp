#include "vpch.h"
#include "VMath.h"
#include <cmath>
#include <algorithm>
#include <random>
#include "Actors/Actor.h"
#include "Camera.h"
#include "Components/CameraComponent.h"
#include "Render/RenderTypes.h"

namespace VMath
{
    void MatrixAddScale(float s, XMMATRIX& m)
    {
        m.r[0].m128_f32[0] = s;
        m.r[1].m128_f32[1] = s;
        m.r[2].m128_f32[2] = s;
    }

    //GLOBAL VECTOR DIRECTIONS (X+ = Right, Y+ = Up, Z+ = forward)
    XMVECTOR GlobalRightVector()
    {
        return XMVectorSet(1.f, 0.f, 0.f, 0.f);
    }

    XMVECTOR GlobalUpVector()
    {
        return XMVectorSet(0.f, 1.f, 0.f, 0.f);
    }

    XMVECTOR GlobalForwardVector()
    {
        return XMVectorSet(0.f, 0.f, 1.f, 0.f);
    }

    XMMATRIX ZeroMatrix()
    {
        return XMMATRIX();
    }

    bool Float3IsZero(XMFLOAT3& float3)
    {
        return float3.x == 0.f && float3.y == 0.f && float3.z == 0.f;
    }

    void RoundFloat3(XMFLOAT3& float3)
    {
        float3.x = std::round(float3.x);
        float3.y = std::round(float3.y);
        float3.z = std::round(float3.z);
    }

    void SaturateFloat4(XMFLOAT4& float4)
    {
        float4.x = std::clamp(float4.x, 0.f, 1.0f);
        float4.y = std::clamp(float4.y, 0.f, 1.0f);
        float4.z = std::clamp(float4.z, 0.f, 1.0f);
        float4.w = std::clamp(float4.w, 0.f, 1.0f);
    }

    void RoundVector(XMVECTOR& v)
    {
        v.m128_f32[0] = std::roundf(v.m128_f32[0]);
        v.m128_f32[1] = std::roundf(v.m128_f32[1]);
        v.m128_f32[2] = std::roundf(v.m128_f32[2]);
        v.m128_f32[3] = std::roundf(v.m128_f32[3]);
    }

    XMVECTOR XMVectorConstantLerp(FXMVECTOR V0, FXMVECTOR V1, float deltaTime, float speed)
    {
        XMVECTOR v = XMVectorLerp(V0, V1, deltaTime * speed);
        return v;
    }

    //REF:https://docs.microsoft.com/en-us/windows/win32/api/directxmath/nf-directxmath-xmvectorlerp
    XMVECTOR SmoothStep(XMVECTOR V0, XMVECTOR V1, float t)
    {
        t = (t > 1.0f) ? 1.0f : ((t < 0.0f) ? 0.0f : t);  // Clamp value to 0 to 1
        t = t * t * (3.f - 2.f * t);
        return XMVectorLerp(V0, V1, t);
    }

    XMFLOAT3 PitchYawRollFromMatrix(XMMATRIX m)
    {
        //REF: https://en.wikipedia.org/wiki/Rotation_formalisms_in_three_dimensions#Conversion_formulae_between_formalisms
        float pitch = acosf(m.r[2].m128_f32[2]);
        float yaw = atan2f(m.r[0].m128_f32[2], m.r[1].m128_f32[2]);
        float roll = atan2f(m.r[2].m128_f32[0], m.r[2].m128_f32[1]);

        pitch = XMConvertToDegrees(pitch);
        yaw = XMConvertToDegrees(yaw);
        roll = XMConvertToDegrees(roll);

        return XMFLOAT3(pitch, yaw, roll);
    }

    //Finding this was the fucking worst. All sorts of wrong mathematical answers on established sites
    //where answers/proofs were mistaking roll for pitch, converting to degrees in radian operations,
    //incorrect asin/atan2 operations. What a fucking mess. In the end, a Unity answer was correct.
    //Ref:https://answers.unity.com/questions/416169/finding-pitchrollyaw-from-quaternions.html
    void PitchYawRollFromQuaternion(float& roll, float& pitch, float& yaw, XMVECTOR quat)
    {
        XMFLOAT4 q{};
        XMStoreFloat4(&q, quat);
        roll = std::atan2(2 * q.y * q.w - 2 * q.x * q.z, 1 - 2 * q.y * q.y - 2 * q.z * q.z);
        pitch = std::atan2(2 * q.x * q.w - 2 * q.y * q.z, 1 - 2 * q.x * q.x - 2 * q.z * q.z);
        yaw = std::asin(2 * q.x * q.y + 2 * q.z * q.w);
    }

    XMVECTOR LookAtRotation(XMVECTOR lookAtPoint, XMVECTOR currentPosition)
    {
        XMVECTOR forward = XMVectorSubtract(lookAtPoint, currentPosition);
        forward = XMVector3Normalize(forward);

        XMVECTOR right = XMVector3Cross(GlobalUpVector(), forward);
        right = XMVector3Normalize(right);

        XMVECTOR up = XMVector3Normalize(XMVector3Cross(forward, right));
        XMMATRIX m = {};
        m.r[0] = right;
        m.r[1] = up;
        m.r[2] = forward;
        m.r[3] = currentPosition;

        XMVECTOR Q = XMQuaternionRotationMatrix(m);
        return Q;
    }

    XMVECTOR ForwardFromQuat(XMVECTOR rot)
    {
        auto m = XMMatrixRotationQuaternion(rot);
        auto f = XMVector3Normalize(m.r[2]);
        return f;
    }

    XMVECTOR RightFromQuat(XMVECTOR rot)
    {
        auto m = XMMatrixRotationQuaternion(rot);
        auto r = XMVector3Normalize(m.r[0]);
        return r;
    }

    XMVECTOR UpFromQuat(XMVECTOR rot)
    {
        auto m = XMMatrixRotationQuaternion(rot);
        auto u = XMVector3Normalize(m.r[1]);
        return u;
    }

    void RotateTowardsCamera(Transform& transform, float zAngle)
    {
        const float posX = transform.position.x;
        const float posZ = transform.position.z;

        //Make sure to use the world matrix here because some camera (Player camera) will 
        //have the camera at an offset from its parent.
        const XMMATRIX cameraWorldMatrix = activeCamera->GetWorldMatrix();

        const float angle = atan2(cameraWorldMatrix.r[3].m128_f32[0] - posX,
            cameraWorldMatrix.r[3].m128_f32[2] - posZ) * (180.0 / XM_PI);

        const float rotation = XMConvertToRadians(angle);

        XMMATRIX m = XMMatrixRotationY(rotation);
        if (zAngle != 0.f)
        {
            //Rotate on the "Z-axis" facing towards camera
            m *= XMMatrixRotationAxis(m.r[2], XMConvertToRadians(zAngle));
        }
        const XMVECTOR rot = XMQuaternionRotationMatrix(m);

        //Set rotation
        XMStoreFloat4(&transform.rotation, rot);
    }

    bool VecEqual(XMVECTOR v1, XMVECTOR v2, float epsilon)
    {
        return fabsf(v1.m128_f32[0] - v2.m128_f32[0]) <= epsilon &&
            fabsf(v1.m128_f32[1] - v2.m128_f32[1]) <= epsilon &&
            fabsf(v1.m128_f32[2] - v2.m128_f32[2]) <= epsilon;
    }

    bool FloatEqualRange(float f1, float f2, float epsilon)
    {
        return fabsf(f1 - f2) <= epsilon;
    }

    bool Float3Equal(XMFLOAT3& f1, XMFLOAT3& f2)
    {
        if (f1.x == f2.x && f1.y == f2.y && f1.z == f2.z)
        {
            return true;
        }

        return false;
    }

    float FindMaxInVector(XMVECTOR v)
    {
        float max = 0.f;
        for (int i = 0; i < 4; i++)
        {
            if (max < v.m128_f32[i])
            {
                max = v.m128_f32[i];
            }
        }

        return max;
    }

    //Copied heavily from UE4
    XMVECTOR VectorConstantLerp(XMVECTOR current, XMVECTOR target, float deltaTime, float interpSpeed)
    {
        XMVECTOR toTarget = XMVectorSubtract(target, current);

        const float magnitude = XMVector3Length(toTarget).m128_f32[0];
        const float step = deltaTime * interpSpeed;

        if (magnitude > step)
        {
            if (step > 0.0f)
            {
                const XMVECTOR diff = toTarget / magnitude;
                return current + diff * step;
            }
            else
            {
                return current;
            }
        }
        
        return target;
    }

    //Copied heavily from UE4
    XMVECTOR QuatConstantLerp(XMVECTOR current, XMVECTOR target, float deltaTime, float interpSpeed)
    {
        float deltaInterpSpeed = std::clamp(deltaTime * interpSpeed, 0.f, 1.f);
        float AngularDistance = std::max(1.e-8f, QuatAngularDistance(target, current));
        float alpha = std::clamp(deltaInterpSpeed / AngularDistance, 0.f, 1.f);

        return XMQuaternionSlerp(current, target, alpha);
    }

    float QuatAngularDistance(XMVECTOR q1, XMVECTOR q2)
    {
        float innerProduct = q1.m128_f32[0] * q2.m128_f32[0] + 
            q1.m128_f32[1] * q2.m128_f32[1] +
            q1.m128_f32[2] * q2.m128_f32[2] +
            q1.m128_f32[3] * q2.m128_f32[3];
        return acosf((2 * innerProduct * innerProduct) - 1.f);
    }

    XMMATRIX GetBoundingBoxMatrix(BoundingOrientedBox& boundingBox, Actor* actor)
    {
        UpdateBoundingBox(boundingBox, actor);
        
        XMFLOAT3 extents = XMFLOAT3(boundingBox.Extents.x * 2.f, boundingBox.Extents.y * 2.f,
            boundingBox.Extents.z * 2.f);
        XMMATRIX boxBoundsMatrix = XMMatrixAffineTransformation(XMLoadFloat3(&extents),
            XMVectorSet(0.f, 0.f, 0.f, 1.f),
            XMLoadFloat4(&boundingBox.Orientation),
            XMLoadFloat3(&boundingBox.Center));

        return boxBoundsMatrix;
    }

    void UpdateBoundingBox(BoundingOrientedBox& boundingBox, Actor* actor)
    {
        XMVECTOR actorPos = actor->GetPositionV();
        XMVECTOR boundingBoxCenter = XMLoadFloat3(&boundingBox.Center);
        XMVECTOR offset = actorPos + boundingBoxCenter;
        offset.m128_f32[3] = 1.0f;

        XMVECTOR actorScale = actor->GetScaleV();
        XMVECTOR extents = XMLoadFloat3(&boundingBox.Extents);
        XMVECTOR scale = extents * actorScale;
        scale.m128_f32[3] = 1.0f;

        XMVECTOR orientation = actor->GetRotationV();

        XMStoreFloat3(&boundingBox.Center, offset);
        XMStoreFloat3(&boundingBox.Extents, scale);
        XMStoreFloat4(&boundingBox.Orientation, orientation);
    }

    BoundingOrientedBox GetBoundingBoxInWorld(SpatialComponent* sc)
    {
        //Position
        XMVECTOR worldPos = sc->GetWorldPositionV();
        XMVECTOR boundingBoxCenter = XMLoadFloat3(&sc->boundingBox.Center);
        XMVECTOR offset = worldPos + boundingBoxCenter;
        offset.m128_f32[3] = 1.0f;

        //Scale
        XMVECTOR spatialComponentScale = sc->GetScaleV();
        XMVECTOR extents = XMLoadFloat3(&sc->boundingBox.Extents);
        XMVECTOR scale = extents * spatialComponentScale;
        scale.m128_f32[3] = 1.0f;

        //Rotation
        XMVECTOR orientation = sc->GetRotationV();

        BoundingOrientedBox orientedBox{};
        XMStoreFloat3(&orientedBox.Center, offset);
        XMStoreFloat3(&orientedBox.Extents, scale);
        XMStoreFloat4(&orientedBox.Orientation, orientation);

        return orientedBox;
    }

    BoundingOrientedBox CreateBoundingBox(Vertex* vertices, size_t verticesCount)
    {
        assert(verticesCount > 0);

        float minX = vertices[0].pos.x;
        float minY = vertices[0].pos.y;
        float minZ = vertices[0].pos.z;

        float maxX = minX;
        float maxY = minY;
        float maxZ = minZ;

        for (size_t i = 0; i < verticesCount; i++)
        {
            float x = vertices[i].pos.x;
            float y = vertices[i].pos.y;
            float z = vertices[i].pos.z;

            if (x > maxX) maxX = x;
            if (x < minX) minX = x;

            if (y > maxY) maxY = y;
            if (y < minY) minY = y;

            if (z > maxZ) maxZ = z;
            if (z < minZ) minZ = z;
        }

        //Remember that extents are halfed
        return BoundingOrientedBox(XMFLOAT3(0.f, 0.f, 0.f),
            XMFLOAT3((maxX - minX) / 2.f, (maxY - minY) / 2.f, (maxZ - minZ) / 2.f),
            XMFLOAT4(0.f, 0.f, 0.f, 1.f));
    }

    float Distance(XMVECTOR pos1, XMVECTOR pos2)
    {
        return XMVector3Length(pos1 - pos2).m128_f32[0];
    }

    XMVECTOR CalcMidPoint(XMVECTOR p0, XMVECTOR p1)
    {
        return (p0 + p1) / 2;
    }

    float RandomRange(float min, float max)
    {
        if (max < min)
        {
            return min;
        }

        if (min > max)
        {
            return max;
        }

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution dist(min, max);
        float result = dist(gen);
        return result;
    }

    int RandomRangeInt(int min, int max)
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution dist(min, max);
        int result = dist(gen);
        return result;
    }

    XMFLOAT3 Float3Subtract(XMFLOAT3& f0, XMFLOAT3& f1)
    {
        return XMFLOAT3(f0.x - f1.x, f0.y - f1.y, f0.z - f1.z);
    }

    XMFLOAT2 Float2Subtract(XMFLOAT2& f0, XMFLOAT2& f1)
    {
        return XMFLOAT2(f0.x - f1.x, f0.y - f1.y);
    }

    //https://stackoverflow.com/questions/72209091/how-to-given-uv-on-a-triangle-find-xyz
    XMVECTOR TriangleUVToXYZ(XMFLOAT2 uv, Vertex tri[3])
    {
        XMVECTOR tri12 = XMLoadFloat2(&tri[1].uv) - XMLoadFloat2(&tri[0].uv);
        XMVECTOR tri13 = XMLoadFloat2(&tri[2].uv) - XMLoadFloat2(&tri[0].uv);
        XMVECTOR p1 = XMLoadFloat2(&uv) - XMLoadFloat2(&tri[0].uv);

        XMVECTOR a23 = XMVector2Cross(tri12, tri13);

        XMVECTOR a12 = XMVector2Cross(p1, tri12) / -a23;
        XMVECTOR a13 = XMVector2Cross(p1, tri13) / a23;

        return XMLoadFloat3(&tri[0].pos)
            + a12 * (XMLoadFloat3(&tri[1].pos) - XMLoadFloat3(&tri[0].pos))
            + a13 * (XMLoadFloat3(&tri[2].pos) - XMLoadFloat3(&tri[0].pos));
    }

    //https://stackoverflow.com/questions/2049582/how-to-determine-if-a-point-is-in-a-2d-triangle
    bool IsUVInTriangleUVs(XMFLOAT2 pt, XMFLOAT2 v1, XMFLOAT2 v2, XMFLOAT2 v3)
    {
        auto sign = [](const XMFLOAT2& p1, const XMFLOAT2& p2, const XMFLOAT2& p3)
        {
            return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
        };

        float d1 = sign(pt, v1, v2);
        float d2 = sign(pt, v2, v3);
        float d3 = sign(pt, v3, v1);

        bool hasNeg = (d1 < 0) || (d2 < 0) || (d3 < 0);
        bool hasPos = (d1 > 0) || (d2 > 0) || (d3 > 0);

        return !(hasNeg && hasPos);
    }
}
