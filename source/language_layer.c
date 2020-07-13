// NOTE(allen): Things I added durring this jam
// Inf32
// NegInf32
// AbsoluteValue
// StringSkip
// StringChop
// "strings.c"
// RangeuContains
// RangeSplit
// bug in RectContains
// bug in RectOverlaps
// StringMatchFlag_MatchCase -> StringMatchFlag_CaseInsensitive
// bug in StringListJoin
// fleshed out Length/Norm types for vectors
// TODO(allen): change the other scalar functions
// StringSplit
// S8Range
// f32Round

////////////////////////////////
// NOTE(allen): Scalar

#define FMod fmodf
#define SquareRoot sqrtf
#define Sin sinf
#define Cos cosf
#define Tan tanf
#define Pow32 powf

internal i32
i32Ceil(f32 x)
{
    if (x > 0)
    {
        return(((i32) x) + 1);
    }
    else
    {
        return(((i32) x));
    }
}

internal i32
i32Floor(f32 x)
{
    if (x > 0)
    {
        return(((i32) x));
    }
    else
    {
        return(((i32) x) - 1);
    }
}

internal f32
f32Ceil(f32 x)
{
    if (x > 0)
    {
        return((f32) ((i32) x) + 1);
    }
    else
    {
        return((f32) ((i32) x));
    }
}

internal f32
f32Floor(f32 x)
{
    if (x > 0)
    {
        return((f32) ((i32) x));
    }
    else
    {
        return((f32) ((i32) x) - 1);
    }
}

internal f32
f32Round(f32 x)
{
    return(f32Floor(x + 0.5f));
}

internal f32
Lerp(f32 a, f32 t, f32 b)
{
    return(a + (b - a)*t);
}

internal f32
AbsoluteValue(f32 f){
    union { u32 u; f32 f; } x;
    x.f = f;
    x.u &= ~Sign32;
    return(x.f);
}

internal f32
Inf32(void){
    union { u32 u; f32 f; } x;
    x.u = Exponent32;
    return(x.f);
}

internal f32
NegInf32(void){
    union { u32 u; f32 f; } x;
    x.u = Sign32 | Exponent32;
    return(x.f);
}

internal f32
SignOfSide(Side side){
    return((side == Side_Min)?-1.f:1.f);
}

////////////////////////////////
// NOTE(allen): vector

#define V2Expand(v) ((v).x), ((v).y)
#define V3Expand(v) ((v).x), ((v).y), ((v).z)
#define V4Expand(v) ((v).x), ((v).y), ((v).z), ((v).w)

internal v2
V2Add(v2 a, v2 b)
{
    v2 c = { a.x + b.x, a.y + b.y };
    return c;
}
internal v3
V3Add(v3 a, v3 b)
{
    v3 c = { a.x + b.x, a.y + b.y, a.z + b.z };
    return c;
}
internal v4
V4Add(v4 a, v4 b)
{
    v4 c = { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
    return c;
}

internal v2
V2Sub(v2 a, v2 b)
{
    v2 c = { a.x - b.x, a.y - b.y };
    return c;
}
internal v3
V3Sub(v3 a, v3 b)
{
    v3 c = { a.x - b.x, a.y - b.y, a.z - b.z };
    return c;
}
internal v4
V4Sub(v4 a, v4 b)
{
    v4 c = { a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w };
    return c;
}

internal v2
V2Mul(v2 v, f32 f)
{
    v.x *= f;
    v.y *= f;
    return v;
}
internal v3
V3Mul(v3 v, f32 f)
{
    v.x *= f;
    v.y *= f;
    v.z *= f;
    return v;
}
internal v4
V4Mul(v4 v, f32 f)
{
    v.x *= f;
    v.y *= f;
    v.z *= f;
    v.w *= f;
    return v;
}

internal f32
V2Dot(v2 a, v2 b)
{
    return a.x*b.x + a.y*b.y;
}
internal f32
V3Dot(v3 a, v3 b)
{
    return a.x*b.x + a.y*b.y + a.z*b.z;
}
internal f32
V4Dot(v4 a, v4 b)
{
    return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
}

internal v2
V2Hadamard(v2 a, v2 b)
{
    v2 v = {a.x*b.x, a.y*b.y};
    return(v);
}
internal v3
V3Hadamard(v3 a, v3 b)
{
    v3 v = {a.x*b.x, a.y*b.y, a.z*b.z};
    return(v);
}
internal v4
V4Hadamard(v4 a, v4 b)
{
    v4 v = {a.x*b.x, a.y*b.y, a.z*b.z, a.w*b.w};
    return(v);
}

internal f32
V2Length1(v2 a)
{
    return(AbsoluteValue(a.x) + AbsoluteValue(a.y));
}
internal f32
V3Length1(v3 a)
{
    return(AbsoluteValue(a.x) + AbsoluteValue(a.y) + AbsoluteValue(a.z));
}
internal f32
V4Length1(v4 a)
{
    return(AbsoluteValue(a.x) + AbsoluteValue(a.y) + AbsoluteValue(a.z) + AbsoluteValue(a.w));
}

internal f32
V2Length2(v2 a)
{
    return SquareRoot(V2Dot(a,a));
}
internal f32
V3Length2(v3 a)
{
    return SquareRoot(V3Dot(a,a));
}
internal f32
V4Length2(v4 a)
{
    return SquareRoot(V4Dot(a,a));
}

#define V2Length V2Length2
#define V3Length V3Length2
#define V4Length V4Length2

internal f32
V2LengthInf(v2 a)
{
    f32 f1 = AbsoluteValue(a.x);
    f32 f2 = AbsoluteValue(a.y);
    f1 = Max(f1, f2);
    return(f1);
}
internal f32
V3LengthInf(v3 a)
{
    f32 f1 = AbsoluteValue(a.x);
    f32 f2 = AbsoluteValue(a.y);
    f32 f3 = AbsoluteValue(a.z);
    f1 = Max(f1, f2);
    f1 = Max(f1, f3);
    return(f1);
}
internal f32
V4LengthInf(v4 a)
{
    f32 f1 = AbsoluteValue(a.x);
    f32 f2 = AbsoluteValue(a.y);
    f32 f3 = AbsoluteValue(a.z);
    f32 f4 = AbsoluteValue(a.w);
    f1 = Max(f1, f2);
    f1 = Max(f1, f3);
    f1 = Max(f1, f4);
    return(f1);
}

#define V2Max V2LengthInf
#define V3Max V3LengthInf
#define V4Max V4LengthInf

internal f32
V2Min(v2 a)
{
    f32 f1 = AbsoluteValue(a.x);
    f32 f2 = AbsoluteValue(a.y);
    f1 = Min(f1, f2);
    return(f1);
}
internal f32
V3Min(v3 a)
{
    f32 f1 = AbsoluteValue(a.x);
    f32 f2 = AbsoluteValue(a.y);
    f32 f3 = AbsoluteValue(a.z);
    f1 = Min(f1, f2);
    f1 = Min(f1, f3);
    return(f1);
}
internal f32
V4Min(v4 a)
{
    f32 f1 = AbsoluteValue(a.x);
    f32 f2 = AbsoluteValue(a.y);
    f32 f3 = AbsoluteValue(a.z);
    f32 f4 = AbsoluteValue(a.w);
    f1 = Min(f1, f2);
    f1 = Min(f1, f3);
    f1 = Min(f1, f4);
    return(f1);
}

internal v2
V2Norm1(v2 v)
{
    f32 inv_length = 1.f/V2Length1(v);
    v2 result = { v.x*inv_length, v.y*inv_length, };
    return result;
}
internal v3
V3Norm1(v3 v)
{
    f32 inv_length = 1.f/V3Length1(v);
    v3 result = { v.x*inv_length, v.y*inv_length, v.z*inv_length, };
    return result;
}
internal v4
V4Norm1(v4 v)
{
    f32 inv_length = 1.f/V4Length1(v);
    v4 result = { v.x*inv_length, v.y*inv_length, v.z*inv_length, v.w*inv_length, };
    return result;
}

internal v2
V2Norm2(v2 v)
{
    f32 inv_length = 1.f/V2Length2(v);
    v2 result = { v.x*inv_length, v.y*inv_length, };
    return result;
}
internal v3
V3Norm2(v3 v)
{
    f32 inv_length = 1.f/V3Length2(v);
    v3 result = { v.x*inv_length, v.y*inv_length, v.z*inv_length, };
    return result;
}
internal v4
V4Norm2(v4 v)
{
    f32 inv_length = 1.f/V4Length2(v);
    v4 result = { v.x*inv_length, v.y*inv_length, v.z*inv_length, v.w*inv_length, };
    return result;
}

#define V2Normalize V2Norm2
#define V3Normalize V3Norm2
#define V4Normalize V4Norm2

internal v2
V2NormInf(v2 v)
{
    f32 inv_length = 1.f/V2LengthInf(v);
    v2 result = { v.x*inv_length, v.y*inv_length, };
    return result;
}
internal v3
V3NormInf(v3 v)
{
    f32 inv_length = 1.f/V3LengthInf(v);
    v3 result = { v.x*inv_length, v.y*inv_length, v.z*inv_length, };
    return result;
}
internal v4
V4NormInf(v4 v)
{
    f32 inv_length = 1.f/V4LengthInf(v);
    v4 result = { v.x*inv_length, v.y*inv_length, v.z*inv_length, v.w*inv_length, };
    return result;
}

internal v3
V3Cross(v3 a, v3 b)
{
    v3 result =
    {
        a.y*b.z - a.z*b.y,
        a.z*b.x - a.x*b.z,
        a.x*b.y - a.y*b.x,
    };
    return result;
}

////////////////////////////////
// NOTE(allen): matrix

internal m4
M4InitD(f32 diagonal)
{
    m4 m =
    {
        {
            { diagonal                },
            { 0.f, diagonal           },
            { 0.f, 0.f, diagonal      },
            { 0.f, 0.f, 0.f, diagonal },
        }
    };
    return m;
}

internal m4
M4MultiplyM4(m4 a, m4 b)
{
    m4 c = {0};
    
    for(int j = 0; j < 4; ++j)
    {
        for(int i = 0; i < 4; ++i)
        {
            c.elements[i][j] = (a.elements[0][j]*b.elements[i][0] +
                                a.elements[1][j]*b.elements[i][1] +
                                a.elements[2][j]*b.elements[i][2] +
                                a.elements[3][j]*b.elements[i][3]);
        }
    }
    
    return c;
}

internal m4
M4MultiplyF32(m4 a, f32 b)
{
    for(int j = 0; j < 4; ++j)
    {
        for(int i = 0; i < 4; ++i)
        {
            a.elements[i][j] *= b;
        }
    }
    
    return a;
}

internal v4
V4MultiplyM4(v4 v, m4 m)
{
    v4 result = {0};
    
    for(int i = 0; i < 4; ++i)
    {
        result.elements[i] = (v.elements[0]*m.elements[0][i] +
                              v.elements[1]*m.elements[1][i] +
                              v.elements[2]*m.elements[2][i] +
                              v.elements[3]*m.elements[3][i]);
    }
    
    return result;
}

internal m4
M4TranslateV3(v3 translation)
{
    m4 result = M4InitD(1.f);
    result.elements[3][0] = translation.x;
    result.elements[3][1] = translation.y;
    result.elements[3][2] = translation.z;
    return result;
}

internal m4
M4ScaleV3(v3 scale)
{
    m4 result = M4InitD(1.f);
    result.elements[0][0] = scale.x;
    result.elements[1][1] = scale.y;
    result.elements[2][2] = scale.z;
    return result;
}

internal m4
M4Perspective(f32 fov, f32 aspect_ratio, f32 near_z, f32 far_z)
{
    m4 result = {0};
    f32 tan_theta_over_2 = Tan(fov * (PI / 360.f));
    result.elements[0][0] = 1.f / tan_theta_over_2;
    result.elements[1][1] = aspect_ratio / tan_theta_over_2;
    result.elements[2][3] = -1.f;
    result.elements[2][2] = (near_z + far_z) / (near_z - far_z);
    result.elements[3][2] = (2.f * near_z * far_z) / (near_z - far_z);
    result.elements[3][3] = 0.f;
    return result;
}

internal m4
M4LookAt(v3 eye, v3 center, v3 up)
{
    m4 result;
    
    v3 f = V3Normalize(V3Sub(center, eye));
    v3 s = V3Normalize(V3Cross(f, up));
    v3 u = V3Cross(s, f);
    
    result.elements[0][0] = s.x;
    result.elements[0][1] = u.x;
    result.elements[0][2] = -f.x;
    result.elements[0][3] = 0.0f;
    
    result.elements[1][0] = s.y;
    result.elements[1][1] = u.y;
    result.elements[1][2] = -f.y;
    result.elements[1][3] = 0.0f;
    
    result.elements[2][0] = s.z;
    result.elements[2][1] = u.z;
    result.elements[2][2] = -f.z;
    result.elements[2][3] = 0.0f;
    
    result.elements[3][0] = -V3Dot(s, eye);
    result.elements[3][1] = -V3Dot(u, eye);
    result.elements[3][2] = V3Dot(f, eye);
    result.elements[3][3] = 1.0f;
    
    return result;
}

internal m4
M4Inverse(m4 m)
{
    f32 coef00 = m.elements[2][2] * m.elements[3][3] - m.elements[3][2] * m.elements[2][3];
    f32 coef02 = m.elements[1][2] * m.elements[3][3] - m.elements[3][2] * m.elements[1][3];
    f32 coef03 = m.elements[1][2] * m.elements[2][3] - m.elements[2][2] * m.elements[1][3];
    f32 coef04 = m.elements[2][1] * m.elements[3][3] - m.elements[3][1] * m.elements[2][3];
    f32 coef06 = m.elements[1][1] * m.elements[3][3] - m.elements[3][1] * m.elements[1][3];
    f32 coef07 = m.elements[1][1] * m.elements[2][3] - m.elements[2][1] * m.elements[1][3];
    f32 coef08 = m.elements[2][1] * m.elements[3][2] - m.elements[3][1] * m.elements[2][2];
    f32 coef10 = m.elements[1][1] * m.elements[3][2] - m.elements[3][1] * m.elements[1][2];
    f32 coef11 = m.elements[1][1] * m.elements[2][2] - m.elements[2][1] * m.elements[1][2];
    f32 coef12 = m.elements[2][0] * m.elements[3][3] - m.elements[3][0] * m.elements[2][3];
    f32 coef14 = m.elements[1][0] * m.elements[3][3] - m.elements[3][0] * m.elements[1][3];
    f32 coef15 = m.elements[1][0] * m.elements[2][3] - m.elements[2][0] * m.elements[1][3];
    f32 coef16 = m.elements[2][0] * m.elements[3][2] - m.elements[3][0] * m.elements[2][2];
    f32 coef18 = m.elements[1][0] * m.elements[3][2] - m.elements[3][0] * m.elements[1][2];
    f32 coef19 = m.elements[1][0] * m.elements[2][2] - m.elements[2][0] * m.elements[1][2];
    f32 coef20 = m.elements[2][0] * m.elements[3][1] - m.elements[3][0] * m.elements[2][1];
    f32 coef22 = m.elements[1][0] * m.elements[3][1] - m.elements[3][0] * m.elements[1][1];
    f32 coef23 = m.elements[1][0] * m.elements[2][1] - m.elements[2][0] * m.elements[1][1];
    
    v4 fac0 = { coef00, coef00, coef02, coef03 };
    v4 fac1 = { coef04, coef04, coef06, coef07 };
    v4 fac2 = { coef08, coef08, coef10, coef11 };
    v4 fac3 = { coef12, coef12, coef14, coef15 };
    v4 fac4 = { coef16, coef16, coef18, coef19 };
    v4 fac5 = { coef20, coef20, coef22, coef23 };
    
    v4 vec0 = { m.elements[1][0], m.elements[0][0], m.elements[0][0], m.elements[0][0] };
    v4 vec1 = { m.elements[1][1], m.elements[0][1], m.elements[0][1], m.elements[0][1] };
    v4 vec2 = { m.elements[1][2], m.elements[0][2], m.elements[0][2], m.elements[0][2] };
    v4 vec3 = { m.elements[1][3], m.elements[0][3], m.elements[0][3], m.elements[0][3] };
    
    v4 inv0 = V4Add(V4Sub(V4Hadamard(vec1, fac0), V4Hadamard(vec2, fac1)), V4Hadamard(vec3, fac2));
    v4 inv1 = V4Add(V4Sub(V4Hadamard(vec0, fac0), V4Hadamard(vec2, fac3)), V4Hadamard(vec3, fac4));
    v4 inv2 = V4Add(V4Sub(V4Hadamard(vec0, fac1), V4Hadamard(vec1, fac3)), V4Hadamard(vec3, fac5));
    v4 inv3 = V4Add(V4Sub(V4Hadamard(vec0, fac2), V4Hadamard(vec1, fac4)), V4Hadamard(vec2, fac5));
    
    v4 sign_a = { +1, -1, +1, -1 };
    v4 sign_b = { -1, +1, -1, +1 };
    
    m4 inverse;
    for(u32 i = 0; i < 4; ++i)
    {
        inverse.elements[0][i] = inv0.elements[i] * sign_a.elements[i];
        inverse.elements[1][i] = inv1.elements[i] * sign_b.elements[i];
        inverse.elements[2][i] = inv2.elements[i] * sign_a.elements[i];
        inverse.elements[3][i] = inv3.elements[i] * sign_b.elements[i];
    }
    
    v4 row0 = { inverse.elements[0][0], inverse.elements[1][0], inverse.elements[2][0], inverse.elements[3][0] };
    v4 m0 = { m.elements[0][0], m.elements[0][1], m.elements[0][2], m.elements[0][3] };
    v4 dot0 = V4Hadamard(m0, row0);
    f32 dot1 = (dot0.x + dot0.y) + (dot0.z + dot0.w);
    
    f32 one_over_det = 1 / dot1;
    
    return M4MultiplyF32(inverse, one_over_det);
}

internal m4
M4RemoveRotation(m4 mat)
{
    v3 scale =
    {
        V3Length(v3(mat.elements[0][0], mat.elements[0][1], mat.elements[0][2])),
        V3Length(v3(mat.elements[1][0], mat.elements[1][1], mat.elements[1][2])),
        V3Length(v3(mat.elements[2][0], mat.elements[2][1], mat.elements[2][2])),
    };
    
    mat.elements[0][0] = scale.x;
    mat.elements[1][0] = 0.f;
    mat.elements[2][0] = 0.f;
    
    mat.elements[0][1] = 0.f;
    mat.elements[1][1] = scale.y;
    mat.elements[2][1] = 0.f;
    
    mat.elements[0][2] = 0.f;
    mat.elements[1][2] = 0.f;
    mat.elements[2][2] = scale.z;
    
    return mat;
}

////////////////////////////////
// NOTE(allen): color

internal v3
RGBToHSV(v3 rgb)
{
    f32 c_max = V3Max(rgb);
    f32 c_min = V3Min(rgb);
    f32 delta = c_max - c_min;
    b32 c_max_is_r = rgb.r > rgb.g && rgb.r > rgb.b;
    b32 c_max_is_g = rgb.g > rgb.r && rgb.g > rgb.b;
    b32 c_max_is_b = rgb.b > rgb.r && rgb.b > rgb.g;
    f32 h = (c_max_is_r ? (rgb.g - rgb.b) / delta + 0 : c_max_is_g ? (rgb.b - rgb.r) / delta + 2 : c_max_is_b ? (rgb.r - rgb.g) / delta + 4 : 0);
    f32 s = c_max == 0 ? 0 : (delta / c_max);
    f32 v = c_max;
    v3 hsv = {h / 6.f, s, v};
    return hsv;
}

internal v3
HSVToRGB(v3 hsv)
{
    f32 h = FMod(hsv.x * 360.f, 360.f);
    f32 s = hsv.y;
    f32 v = hsv.z;
    
    f32 c = v * s;
    f32 x = c * (1 - AbsoluteValue(FMod((h / 60.f), 2) - 1));
    f32 m = v - c;
    
    f32 r;
    f32 g;
    f32 b;
    
    if((h >= 0.f && h < 60.f) || (h >= 360.f && h < 420.f))
    {
        r = c;
        g = x;
        b = 0;
    }
    else if(h >= 60.f && h < 120.f)
    {
        r = x;
        g = c;
        b = 0;
    }
    else if(h >= 120.f && h < 180.f)
    {
        r = 0;
        g = c;
        b = x;
    }
    else if(h >= 180.f && h < 240.f)
    {
        r = 0;
        g = x;
        b = c;
    }
    else if(h >= 240.f && h < 300.f)
    {
        r = x;
        g = 0;
        b = c;
    }
    else if((h >= 300.f && h <= 360.f) ||
            (h >= -60.f && h <= 0.f))
    {
        r = c;
        g = 0;
        b = x;
    }
    
    v3 rgb = {r + m, g + m, b + m};
    return rgb;
}

////////////////////////////////
// NOTE(allen): interval

internal Range
MakeRange(f32 a, f32 b)
{
    Range range = {a, b};
    if (a > b)
    {
        Swap(f32, range.min, range.max);
    }
    return(range);
}
internal Rangei
MakeRangei(i64 a, i64 b)
{
    Rangei range = {a, b};
    if (a > b)
    {
        Swap(i64, range.min, range.max);
    }
    return(range);
}
internal Rangeu
MakeRangeu(u64 a, u64 b)
{
    Rangeu range = {a, b};
    if (a > b)
    {
        Swap(u64, range.min, range.max);
    }
    return(range);
}

#define RangeSize(range) ((range).max - (range).min)

internal Range
RangeUnion(Range a, Range b)
{
    Range result;
    result.min = Min(a.min, b.min);
    result.max = Max(a.max, b.max);
    return(result);
}

internal Range
RangeIntersection(Range a, Range b)
{
    Range result;
    result.min = Max(a.min, b.min);
    result.max = Min(a.max, b.max);
    result.min = ClampTop(result.min, result.max);
    return(result);
}

internal Range
RangeGrow(Range range, f32 x)
{
    range.min -= x;
    range.max += x;
    return(range);
}

internal Range
RangeShrink(Range range, f32 x)
{
    range.min += x;
    range.max -= x;
    range.min = ClampTop(range.min, range.max);
    return(range);
}

internal Range
RangeSplit(Range *free_range, Side side, f32 amt){
    Range range;
    if (side == Side_Min){
        range.min = free_range->min;
        range.max = range.min + amt;
        range.max = ClampTop(range.max, free_range->max);
        free_range->min = range.max;
    }
    else{
        range.max = free_range->max;
        range.min = range.max - amt;
        range.min = ClampBot(range.min, free_range->min);
        free_range->max = range.min;
    }
    return(range);
}

internal b32
RangeContains(Range range, f32 x)
{
    return(range.min <= x && x < range.max);
}

internal b32
RangeOverlaps(Range a, Range b)
{
    return(a.min < b.max && b.min < a.max);
}

internal b32
RangeuContains(Rangeu range, u64 x)
{
    return(range.min <= x && x < range.max);
}

internal Rect
MakeRect(f32 x0, f32 y0, f32 x1, f32 y1)
{
    Rect rect = {x0, y0, x1, y1};
    return(rect);
}
internal Rect
MakeRectVec(v2 p0, v2 p1)
{
    Rect rect = {p0.x, p0.y, p1.x, p1.y};
    return(rect);
}
internal Rect
MakeRectRanges(Range x, Range y)
{
    Rect rect = {x.min, y.min, x.max, y.max};
    return(rect);
}

internal Rect
RectUnion(Rect a, Rect b)
{
    Rect result;
    result.x0 = Min(a.x0, b.x0);
    result.y0 = Min(a.y0, b.y0);
    result.x1 = Max(a.x1, b.x1);
    result.y1 = Max(a.y1, b.y1);
    return(result);
}

internal Rect
RectIntersect(Rect a, Rect b)
{
    Rect result;
    result.x0 = Max(a.x0, b.x0);
    result.y0 = Max(a.y0, b.y0);
    result.x1 = Min(a.x1, b.x1);
    result.y1 = Min(a.y1, b.y1);
    result.x0 = ClampTop(result.x0, result.x1);
    result.y0 = ClampTop(result.y0, result.y1);
    return(result);
}

internal Rect
RectGrow(Rect rect, f32 v)
{
    rect.x0 -= v;
    rect.y0 -= v;
    rect.x1 += v;
    rect.y1 += v;
    return(rect);
}

internal Rect
RectShrink(Rect rect, f32 v)
{
    rect.x0 += v;
    rect.y0 += v;
    rect.x1 -= v;
    rect.y1 -= v;
    rect.x0 = ClampTop(rect.x0, rect.x1);
    rect.y0 = ClampTop(rect.y0, rect.y1);
    return(rect);
}

internal Range
RectGetRange(Rect rect, Dimension dim)
{
    Range range = {rect.p0.v[dim], rect.p1.v[dim]};
    return(range);
}

internal b32
RectContains(Rect rect, v2 p)
{
    return(rect.x0 <= p.x && p.x < rect.x1 && rect.y0 <= p.y && p.y < rect.y1);
}

internal b32
RectOverlaps(Rect a, Rect b)
{
    return(a.x0 < b.x1 && b.x0 < a.x1 && a.y0 < b.y1 && b.y0 < a.y1);
}

internal v2
RectGetDim(Rect rect)
{
    v2 p = {rect.x1 - rect.x0, rect.y1 - rect.y0};
    return(p);
}
#define RectSize(r) RectGetDim(r)

internal v2
RectGetCenter(Rect rect)
{
    v2 p = {0.5f*(rect.x1 + rect.x0), 0.5f*(rect.y1 + rect.y0)};
    return(p);
}

////////////////////////////////
//~ NOTE(allen): String

internal b32
CharIsSpace(char c)
{
    return((c) <= 32);
}

internal b32
CharIsAlpha(char c)
{
    return ((c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z'));
}

internal b32
CharIsDigit(char c)
{
    return (c >= '0' && c <= '9');
}

internal b32
CharIsSymbol(char c)
{
    return (c == '~' ||
            c == '!' ||
            c == '%' ||
            c == '^' ||
            c == '&' ||
            c == '*' ||
            c == '(' ||
            c == ')' ||
            c == '[' ||
            c == ']' ||
            c == '{' ||
            c == '}' ||
            c == '-' ||
            c == '+' ||
            c == '=' ||
            c == ';' ||
            c == ':' ||
            c == '<' ||
            c == '>' ||
            c == '/' ||
            c == '?' ||
            c == '.' ||
            c == ',');
}

internal char
CharToLower(char c)
{
    if(c >= 'A' && c <= 'Z')
    {
        return c + 32;
    }
    return c;
}

internal char
CharToUpper(char c)
{
    if(c >= 'a' && c <= 'z')
    {
        return c - 32;
    }
    return c;
}

internal String8
S8(u8 *str, u64 size)
{
    String8 result = {str, size};
    return(result);
}

internal String8
S8Range(u8 *first, u8 *one_past_last)
{
    String8 result = {first, one_past_last - first};
    return(result);
}

internal String8
S8Zero(void){
    String8 result = {0, 0};
    return(result);
}

internal String8
String8FromCString(char *cstring)
{
    String8 string = {0};
    string.str = (u8 *)cstring;
    string.size = CalculateCStringLength(cstring);
    return string;
}

internal b32
StringMatchGeneric(String8 a, String8 b, StringMatchFlags flags)
{
    b32 result = 0;
    
    if(a.size == b.size || (flags & StringMatchFlag_RightSideSloppy))
    {
        b32 insensitive = (flags & StringMatchFlag_CaseInsensitive);
        u64 size = Min(a.size, b.size);
        result = 1;
        for(u64 i = 0; i < size; ++i)
        {
            u8 at = a.str[i];
            u8 bt = b.str[i];
            if(insensitive)
            {
                at = CharToUpper(at);
                bt = CharToUpper(bt);
            }
            if(at != bt)
            {
                result = 0;
                break;
            }
        }
    }
    
    return result;
}

internal b32
StringMatch(String8 a, String8 b)
{
    return StringMatchGeneric(a, b, 0);
}

internal b32
StringMatchCaseInsensitive(String8 a, String8 b)
{
    return StringMatchGeneric(a, b, StringMatchFlag_CaseInsensitive);
}

internal u64
StringFindSubstringStart(String8 a, String8 sub, StringMatchFlags flags)
{
    u64 result = ~(u64)0;
    for (u64 i = 0; i < a.size; i += 1){
        if (a.str[i] == sub.str[0]){
            if (StringMatchGeneric(a, sub, flags | StringMatchFlag_RightSideSloppy)){
                result = i;
                break;
            }
        }
    }
    return(result);
}

internal String8
StringSubstring(String8 string, Rangeu range)
{
    range.max = ClampTop(range.max, string.size);
    range.min = ClampTop(range.min, string.size);
    string.str += range.min;
    string.size = RangeSize(range);
    return(string);
}

internal String8
StringPrefix(String8 string, u64 size)
{
    string.size = ClampTop(size, string.size);
    return(string);
}

internal String8
StringSkip(String8 string, u64 val)
{
    val = ClampTop(val, string.size);
    string.str += val;
    string.size -= val;
    return(string);
}

internal String8
StringPostfix(String8 string, u64 size)
{
    size = ClampTop(size, string.size);
    string.str = (string.str + string.size) - size;
    string.size = size;
    return(string);
}

internal String8
StringChop(String8 string, u64 val)
{
    val = ClampTop(val, string.size);
    string.size -= val;
    return(string);
}

internal String8
PushStringCat(M_Arena *arena, String8 a, String8 b){
    String8 str;
    str.size = a.size + b.size;
    str.str = PushArray(arena, u8, str.size);
    MemoryCopy(str.str, a.str, a.size);
    MemoryCopy(str.str + a.size, b.str, b.size);
    return(str);
}

internal String8
PushStringFV(M_Arena *arena, char *format, va_list args)
{
    va_list args2;
    va_copy(args2, args);
    u32 needed_bytes = vsnprintf(0, 0, format, args) + 1;
    String8 result = {0};
    result.str = PushArray(arena, u8, needed_bytes);
    result.size = vsnprintf((char*)result.str, needed_bytes, format, args2);
    result.str[result.size] = 0;
    return(result);
}

internal String8
PushStringF(M_Arena *arena, char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    String8 result = PushStringFV(arena, fmt, args);
    va_end(args);
    return(result);
}

////////////////////////////////
//~ NOTE(allen): String List

internal String8_Node*
StringListPush(M_Arena *arena, String8_List *list, String8 string)
{
    String8_Node *node = PushArray(arena, String8_Node, 1);
    SLLQueuePush(list->first, list->last, node);
    list->node_count += 1;
    list->total_size += string.size;
    node->string = string;
    return(node);
}

internal String8_Node*
StringListPushFront(M_Arena *arena, String8_List *list, String8 string)
{
    String8_Node *node = PushArray(arena, String8_Node, 1);
    SLLQueuePushFront(list->first, list->last, node);
    list->node_count += 1;
    list->total_size += string.size;
    node->string = string;
    return(node);
}

internal String8_Node*
StringListPushF(M_Arena *arena, String8_List *list, char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    String8 string = PushStringFV(arena, fmt, args);
    String8_Node *result = StringListPush(arena, list, string);
    va_end(args);
    return(result);
}

internal String8_Node*
StringListPushFrontF(M_Arena *arena, String8_List *list, char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    String8 string = PushStringFV(arena, fmt, args);
    String8_Node *result = StringListPushFront(arena, list, string);
    va_end(args);
    return(result);
}

internal void
StringListRemoveEmpties(String8_List *list)
{
    String8_Node *new_first = 0;
    String8_Node *new_last = 0;
    u64 new_count = 0;
    for (String8_Node *node = list->first, *next = 0;
         node != 0;
         node = next)
    {
        next = node->next;
        if (node->string.size > 0){
            SLLQueuePush(new_first, new_last, node);
            new_count += 1;
        }
    }
    list->first = new_first;
    list->last = new_last;
    list->node_count = new_count;
}

internal String8_List
StringSplit(M_Arena *arena, String8 string, u8 *split_chars, u64 split_char_count)
{
    String8_List list = {0};
    
    u8 *ptr = string.str;
    u8 *opl = string.str + string.size;
    for (;ptr < opl;)
    {
        u8 *first = ptr;
        for (;ptr < opl; ptr += 1)
        {
            u8 c = *ptr;
            b32 is_split = 0;
            for (u64 i = 0; i < split_char_count; i += 1)
            {
                if (split_chars[i] == c)
                {
                    is_split = 1;
                    break;
                }
            }
            if (is_split)
            {
                break;
            }
        }
        
        StringListPush(arena, &list, S8Range(first, ptr));
        ptr += 1;
    }
    
    return(list);
}

internal String8
StringListJoin(M_Arena *arena, String8_List *list, String_Join *optional_join)
{
    String_Join join = {0};
    if (optional_join != 0)
    {
        MemoryCopyStruct(&join, optional_join);
    }
    
    String8 result;
    result.size = join.pre.size + join.post.size + (list->node_count - 1)*join.sep.size + list->total_size;
    u8 *ptr = result.str = PushArray(arena, u8, result.size);
    
    MemoryCopy(ptr, join.pre.str, join.pre.size);
    ptr += join.pre.size;
    for (String8_Node *node = list->first;
         node != 0;
         node = node->next)
    {
        MemoryCopy(ptr, node->string.str, node->string.size);
        ptr += node->string.size;
        if (node->next != 0)
        {
            MemoryCopy(ptr, join.sep.str, join.sep.size);
            ptr += join.sep.size;
        }
    }
    MemoryCopy(ptr, join.post.str, join.post.size);
    ptr += join.post.size;
    
    return(result);
}

////////////////////////////////
//~ NOTE(allen): Additional String Stuff

internal i64
GetFirstIntegerFromString(String8 string)
{
    i32 result = 0;
    b32 found_first_digit = 0;
    u32 integer_write_pos = 0;
    u8 integer[64] = {0};
    u32 read_pos = 0;
    for(;; ++read_pos)
    {
        if(string.str[read_pos] == 0)
        {
            break;
        }
        if(found_first_digit)
        {
            if(integer_write_pos == sizeof(integer))
            {
                integer[sizeof(integer) - 1] = 0;
                break;
            }
            if(CharIsDigit(string.str[read_pos]) || string.str[read_pos] == '-')
            {
                integer[integer_write_pos++] = string.str[read_pos];
            }
            else
            {
                integer[integer_write_pos++] = 0;
                break;
            }
        }
        else
        {
            if(CharIsDigit(string.str[read_pos]) || string.str[read_pos] == '-')
            {
                integer[integer_write_pos++] = string.str[read_pos];
                found_first_digit = 1;
            }
        }
    }
    result = CStringToI32(integer);
    return result;
}

internal f32
GetFirstF32FromCString(char *str)
{
    f32 result = 0;
    b32 found_first_digit = 0;
    u32 float_write_pos = 0;
    char float_str[64] = {0};
    u32 read_pos = 0;
    for(;; ++read_pos)
    {
        if(str[read_pos] == 0)
        {
            break;
        }
        if(found_first_digit)
        {
            if(float_write_pos == sizeof(float_str))
            {
                float_str[sizeof(float_str) - 1] = 0;
                break;
            }
            if(CharIsDigit(str[read_pos]) || str[read_pos] == '.' || str[read_pos] == '-')
            {
                float_str[float_write_pos++] = str[read_pos];
            }
            else
            {
                float_str[float_write_pos++] = 0;
                break;
            }
        }
        else
        {
            if(CharIsDigit(str[read_pos]) || str[read_pos] == '.' || str[read_pos] == '-')
            {
                float_str[float_write_pos++] = str[read_pos];
                found_first_digit = 1;
            }
        }
    }
    result = CStringToF32(float_str);
    return result;
}

internal void
CopySubstringToStringUntilCharN(char *str1, u32 str1_max,
                                const char *str2, char str2_term)
{
    u32 write_pos = 0;
    while(1)
    {
        if(str2[write_pos] == str2_term || write_pos == str1_max - 1)
        {
            str1[write_pos++] = 0;
            break;
        }
        else
        {
            str1[write_pos] = str2[write_pos];
            ++write_pos;
        }
    }
}

internal void
CopyStringToFixedSizeBuffer(char *dest, u32 dest_max,
                            const char *src)
{
    u32 read_pos = 0;
    u32 write_pos = 0;
    for(;;)
    {
        if(src[read_pos] == 0 || write_pos >= dest_max)
        {
            break;
        }
        dest[write_pos++] = src[read_pos++];
    }
    if(write_pos >= dest_max)
    {
        dest[dest_max - 1] = 0;
    }
    else
    {
        dest[write_pos++] = 0;
    }
}

internal u32
CStringIndexAfterSubstring(char *str, char *substr)
{
    u32 result = 0;
    for(u32 i = 0; str[i]; ++i)
    {
        if(str[i] == substr[0])
        {
            if(CStringMatchCaseInsensitiveN(str + i, substr, CalculateCStringLength(substr)))
            {
                result = i + CalculateCStringLength(substr);
            }
        }
    }
    return result;
}

internal u32
CStringFirstIndexAfterSubstring(char *str, char *substr)
{
    u32 result = 0;
    for(u32 i = 0; str[i]; ++i)
    {
        if(str[i] == substr[0])
        {
            if(CStringMatchCaseInsensitiveN(str + i, substr, CalculateCStringLength(substr)))
            {
                result = i + CalculateCStringLength(substr);
                break;
            }
        }
    }
    return result;
}

internal void
CopyCStringToFixedSizeBuffer(char *destination, u32 destination_max, char *source)
{
    for(u32 i = 0; i < destination_max; ++i)
    {
        destination[i] = source[i];
        if(source[i] == 0)
        {
            break;
        }
    }
    destination[destination_max-1] = 0;
}

internal void
CopyCStringToFixedSizeBufferN(char *destination, u32 destination_max, char *source, u32 source_max)
{
    for(u32 i = 0; i < destination_max && i < source_max; ++i)
    {
        destination[i] = source[i];
        if(source[i] == 0)
        {
            break;
        }
    }
    destination[destination_max-1] = 0;
}

internal char *
ConvertCStringToLowercase(char *str)
{
    for(int i = 0; str[i]; ++i)
    {
        str[i] = CharToLower(str[i]);
    }
    return str;
}

internal char *
ConvertCStringToUppercase(char *str)
{
    for(int i = 0; str[i]; ++i)
    {
        str[i] = CharToUpper(str[i]);
    }
    return str;
}

internal char *
ConvertCStringToLowercaseWithUnderscores(char *str)
{
    for(int i = 0; str[i]; ++i)
    {
        if(str[i] == ' ')
        {
            str[i] = '_';
        }
        else
        {
            str[i] = CharToLower(str[i]);
        }
    }
    return str;
}

static unsigned int global_crc32_table[] =
{
    0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9,
    0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005,
    0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
    0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd,
    0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9,
    0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
    0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011,
    0x791d4014, 0x7ddc5da3, 0x709f7b7a, 0x745e66cd,
    0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
    0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5,
    0xbe2b5b58, 0xbaea46ef, 0xb7a96036, 0xb3687d81,
    0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
    0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49,
    0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95,
    0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
    0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d,
    0x34867077, 0x30476dc0, 0x3d044b19, 0x39c556ae,
    0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
    0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16,
    0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca,
    0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
    0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02,
    0x5e9f46bf, 0x5a5e5b08, 0x571d7dd1, 0x53dc6066,
    0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
    0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e,
    0xbfa1b04b, 0xbb60adfc, 0xb6238b25, 0xb2e29692,
    0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
    0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a,
    0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e,
    0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
    0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686,
    0xd5b88683, 0xd1799b34, 0xdc3abded, 0xd8fba05a,
    0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
    0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb,
    0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f,
    0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
    0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47,
    0x36194d42, 0x32d850f5, 0x3f9b762c, 0x3b5a6b9b,
    0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
    0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623,
    0xf12f560e, 0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7,
    0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
    0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f,
    0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3,
    0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
    0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b,
    0x9b3660c6, 0x9ff77d71, 0x92b45ba8, 0x9675461f,
    0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
    0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640,
    0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c,
    0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
    0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24,
    0x119b4be9, 0x155a565e, 0x18197087, 0x1cd86d30,
    0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
    0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088,
    0x2497d08d, 0x2056cd3a, 0x2d15ebe3, 0x29d4f654,
    0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
    0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c,
    0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18,
    0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
    0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0,
    0x9abc8bd5, 0x9e7d9662, 0x933eb0bb, 0x97ffad0c,
    0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
    0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4,
};

internal u32
CStringCRC32N(char *name, u32 n)
{
    u32 crc = 0;
    for(u32 i = 0; name[i] && i < n; ++i)
    {
        crc = (crc << 8) ^ global_crc32_table[((crc >> 24) ^ name[i]) & 255];
    }
    return crc;
}

internal u32
CStringCRC32(char *name)
{
    return CStringCRC32N(name, (u32)(u32)(-1));
}

internal void
AppendToFixedSizeCString(char *destination, u32 destination_max, char *str)
{
    u32 i = 0;
    for(; i < destination_max && destination[i]; ++i);
    if(destination_max > i)
    {
        CopyCStringToFixedSizeBuffer(destination + i, destination_max - i, str);
    }
}
