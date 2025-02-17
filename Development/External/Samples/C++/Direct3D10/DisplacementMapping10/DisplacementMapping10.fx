//--------------------------------------------------------------------------------------
// File: DisplacementMapping10.fx
//
// The effect file for the DisplacementMapping10 sample.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#define MAX_BONE_MATRICES 255

struct VSDisplaceIn
{
    float3 Pos	: POS;			    //Position
    float4 Weights : WEIGHTS;		//Bone weights
    uint4  Bones : BONES;			//Bone indices
    float3 Norm : NORMAL;			//Normal
    float2 Tex	: TEXCOORD0;		//Texture coordinate
    float3 Tan : TANGENT;		    //Normalized Tangent vector
    uint   VertID : SV_VertexID;	//verTex ID, used for consistent tetrahedron generation
};

struct VSDisplaceOut
{
    float4 Pos	: POS;			//Position
    float3 vPos : POSVIEW;		//view pos
    float3 Norm : NORMAL;		//Normal
    float3 Tex	: TEXCOORD0;	//Texture coordinate
    float3 Tangent : TANGENT;	//Normalized Tangent vector
    uint   VertID : VertID;	    //verTex ID, used for consistent tetrahedron generation
};

struct VSSceneIn
{
    float3 Pos	: POS;			//Position
    float3 Norm : NORMAL;		//Normal
    float2 Tex	: TEXCOORD0;	//Texture coordinate
    float3 Tan : TANGENT;		//Normalized Tangent vector
};

struct PSSceneIn
{
    float4 Pos	: SV_Position;		//Position
    float3 Norm : NORMAL;			//Normal
    float3 Tan : TANGENT;			//Tangent
    float2 Tex	: TEXCOORD0;		//Texture coordinate
    float2 ShadowTex : TEXCOORD1;   //projected texture coordinate
    float3 LightDir : TEXCOORD2;
    float3 ViewDir : TEXCOORD3;
};

struct PSNormalIn
{
    float4 Pos	: SV_Position;		//Position
    float3 vPos : POSVIEW;			//world space Pos
    float3 Norm : NORMAL;			//Normal
    float2 Tex	: TEXCOORD0;		//Texture coordinate
    float3 Tangent : TANGENT;		//Normalized Tangent vector
};

struct PSDisplaceIn
{
    float4 Pos : SV_Position;
    float4 planeDist : TEXCOORD0;
    float3 vPos : TEXCOORD1;
    
    float3 Norm : TEXCOORD2;		// Normal of the first vert
    float3 TanT : TEXCOORD3;		// Tangent of the first vert
    float3 Tex : TEXCOORD4;		// Texture Coordinate of the first vert
    float3 pos0 : TEXCOORD5;			// Position of the first vert
    
    float4 GtxNx : TEXCOORD6;			// Gradient of the tetrahedron for X texcoord
    float4 GtyNx : TEXCOORD7;			// Gradient of the tetrahedron for Y texcoord
    float4 GtzNx : TEXCOORD8;			// Gradient of the tetrahedron for Z texcoord
    
    float4 GTxNy : TEXCOORD9;			// Gradient of the tetrahedron for X Tangent
    float4 GTyNy : TEXCOORD10;		// Gradient of the tetrahedron for Y Tangent
    float4 GTzNy : TEXCOORD11;		// Gradient of the tetrahedron for Z Tangent
    
    float3 GNz : TEXCOORD12;		// Gradient of the tetrahedron for X Normal
};

struct VSQuadIn
{
    float3 Pos : POS;
    float2 Tex : TEXCOORD0;
};

struct PSQuadIn
{
    float4 Pos : SV_Position;
    float2 Tex : TEXCOORD0;
};

cbuffer cbEveryFrame
{
    float4x4 g_mWorldViewProj;
    float4x4 g_mWorld;
    float4x4 g_mWorldView;
    float4x4 g_mView;
    float4x4 g_mViewProj;
    float4x4 g_mProj;
    float4x4 g_mLightViewProj;
    float3 g_vEyePt;
};

cbuffer cbAnimMatrices
{
    matrix g_mBoneWorld[MAX_BONE_MATRICES];
};

cbuffer cbUserChange
{
    float g_MaxDisplacement = 0.8;
    float g_MinDisplacement = 0.015;
    float3 g_vLightPos;
};

cbuffer cb2
{
    float kDiffuse = 1.0;
    float4 g_directional = float4(1.0,0.972,0.909,1.0);
    float4 g_ambient = float4(0.515,0.527,0.6,0.0);
    float4 g_scenespeccolor = float4(0.204,0.163,0.079,1.0);
    float4 g_objectspeccolor = float4(0.381,0.379,0.264,1.0);
};

Texture2D g_txDiffuse;
Texture2D g_txNormal;
Texture2D g_txDisplace;
Texture2D g_txShadow;
SamplerState g_samLinear
{
    Filter = ANISOTROPIC;
    AddressU = Clamp;
    AddressV = Clamp;
};

SamplerState g_samLinearShadow
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};


SamplerState g_samClamp
{
    Filter = ANISOTROPIC;
    AddressU = Clamp;
    AddressV = Clamp;
};

SamplerState g_samPoint
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Clamp;
    AddressV = Clamp;
};


RasterizerState DisableCulling
{
    CullMode = NONE;
};

RasterizerState EnableCulling
{
    CullMode = BACK;
};

RasterizerState  ReverseCulling
{
    CullMode = FRONT;
};

DepthStencilState DisableDepthTestWrite
{
    DepthEnable = FALSE;
    DepthWriteMask = ZERO;
};

DepthStencilState DisableDepthWrite
{
    DepthEnable = TRUE;
    DepthWriteMask = ZERO;
};

DepthStencilState EnableDepthTestWrite
{
    DepthEnable = TRUE;
    DepthWriteMask = ALL;
};

BlendState NoBlending
{
    AlphaToCoverageEnable = FALSE;
    BlendEnable[0] = FALSE;
};

struct SkinnedInfo
{
    float4 Pos;
    float3 Norm;
    float3 Tan;
};

SkinnedInfo SkinVert( VSDisplaceIn Input )
{
    SkinnedInfo Output = (SkinnedInfo)0;
    
    float4 Pos = float4(Input.Pos,1);
    float3 Norm = Input.Norm;
    float3 Tan = Input.Tan;
    
    uint iBone = Input.Bones.x;
    float fWeight = Input.Weights.x;
    matrix m = g_mBoneWorld[ iBone ];
    Output.Pos += fWeight * mul( Pos, m );
    Output.Norm += fWeight * mul( Norm, m );
    Output.Tan += fWeight * mul( Tan, (float3x3)m );
    
    iBone = Input.Bones.y;
    fWeight = Input.Weights.y;
    m = g_mBoneWorld[ iBone ];
    Output.Pos += fWeight * mul( Pos, m );
    Output.Norm += fWeight * mul( Norm, m );
    Output.Tan += fWeight * mul( Tan, (float3x3)m );
    
    iBone = Input.Bones.z;
    fWeight = Input.Weights.z;
    m = g_mBoneWorld[ iBone ];
    Output.Pos += fWeight * mul( Pos, m );
    Output.Norm += fWeight * mul( Norm, m );
    Output.Tan += fWeight * mul( Tan, (float3x3)m );
    
    iBone = Input.Bones.w;
    fWeight = Input.Weights.w;
    m = g_mBoneWorld[ iBone ];
    Output.Pos += fWeight * mul( Pos, m );
    Output.Norm += fWeight * mul( Norm, m );
    Output.Tan += fWeight * mul( Tan, (float3x3)m );
    
    return Output;
}

PSNormalIn VSNormalmain(VSDisplaceIn input)
{
    PSNormalIn output;
    
    SkinnedInfo vSkinned = SkinVert( input );
    output.Pos = mul( vSkinned.Pos, g_mViewProj );
    output.vPos = vSkinned.Pos;
    output.Norm = vSkinned.Norm;
    output.Tangent = normalize( vSkinned.Tan );
    output.Tex = float3(input.Tex,0);
    
    return output;
}

float4 PSNormalmain(PSNormalIn input) : SV_Target
{	
    float4 diffuse = g_txDiffuse.Sample( g_samLinear, input.Tex );
    float3 Norm = g_txNormal.Sample( g_samLinear, input.Tex );
    Norm *= 2.0;
    Norm -= float3(1,1,1);
    
    float3 lightDir = normalize( g_vLightPos - input.vPos );
    float3 viewDir = normalize( g_vEyePt - input.vPos );
    float3 BiNorm = normalize( cross( input.Norm, input.Tangent ) );
    float3x3 BTNMatrix = float3x3( BiNorm, input.Tangent, input.Norm );
    Norm = normalize( mul( Norm, BTNMatrix ) ); //world space bump
    
    //diffuse lighting
    float lightAmt = saturate( dot( lightDir, Norm ) );
    float4 lightColor = lightAmt.xxxx*g_directional + g_ambient;

    // Calculate specular power
    float3 halfAngle = normalize( viewDir + lightDir );
    float4 spec = saturate( pow( dot( halfAngle, Norm ), 64 ) );
        
    // Return combined lighting
    return lightColor*diffuse*kDiffuse + spec*g_objectspeccolor*diffuse.a;
}

float4 PSSceneBlack(PSNormalIn input) : SV_TARGET
{
    return float4(0,0,0,1);
}

VSDisplaceOut VSDisplaceMain(VSDisplaceIn input)
{
    VSDisplaceOut output;
    
    SkinnedInfo vSkinned = SkinVert( input );
    output.Pos = vSkinned.Pos;
    output.vPos = vSkinned.Pos;
    output.Norm = normalize(vSkinned.Norm);
    output.Tangent = normalize( vSkinned.Tan );
    output.Tex = float3(input.Tex,0);
    output.VertID = input.VertID;
    
    return output;
}

float RayDistToPlane( float3 vPoint, float3 vDir, float3 A, float3 planeNorm )
{	
    float Nom = dot( planeNorm, float3(A - vPoint) );
    float DeNom = dot( planeNorm, vDir );
    return Nom/DeNom;
}

void CalcGradients( inout PSDisplaceIn V0, inout PSDisplaceIn V1, inout PSDisplaceIn V2, inout PSDisplaceIn V3, float3 N0, float3 N1, float3 N2, float3 N3 )
{
    float dotN0 = dot(N0, V0.vPos - V3.vPos);
    float dotN1 = dot(N1, V1.vPos - V2.vPos);
    float dotN2 = dot(N2, V2.vPos - V1.vPos);
    float dotN3 = dot(N3, V3.vPos - V0.vPos);
    
    //Tex
    float3	Gtx =  ( V0.Tex.x / dotN0 )*N0;
            Gtx += ( V1.Tex.x / dotN1 )*N1;
            Gtx += ( V2.Tex.x / dotN2 )*N2;
            Gtx += ( V3.Tex.x / dotN3 )*N3;
    
    float3	Gty =  ( V0.Tex.y / dotN0 )*N0;
            Gty += ( V1.Tex.y / dotN1 )*N1;
            Gty += ( V2.Tex.y / dotN2 )*N2;
            Gty += ( V3.Tex.y / dotN3 )*N3;
    
    float3	Gtz =  ( V0.Tex.z / dotN0 )*N0;
            Gtz += ( V1.Tex.z / dotN1 )*N1;
            Gtz += ( V2.Tex.z / dotN2 )*N2;
            Gtz += ( V3.Tex.z / dotN3 )*N3;
        
    //Tangent	
    float3	GTx =  ( V0.TanT.x / dotN0 )*N0;
            GTx += ( V1.TanT.x / dotN1 )*N1;
            GTx += ( V2.TanT.x / dotN2 )*N2;
            GTx += ( V3.TanT.x / dotN3 )*N3;
    
    float3	GTy =  ( V0.TanT.y / dotN0 )*N0;
            GTy += ( V1.TanT.y / dotN1 )*N1;
            GTy += ( V2.TanT.y / dotN2 )*N2;
            GTy += ( V3.TanT.y / dotN3 )*N3;
    
    float3	GTz =  ( V0.TanT.z / dotN0 )*N0;
            GTz += ( V1.TanT.z / dotN1 )*N1;
            GTz += ( V2.TanT.z / dotN2 )*N2;
            GTz += ( V3.TanT.z / dotN3 )*N3;
    
    //Normal	
    float3	GNx =  ( V0.Norm.x / dotN0 )*N0;
            GNx += ( V1.Norm.x / dotN1 )*N1;
            GNx += ( V2.Norm.x / dotN2 )*N2;
            GNx += ( V3.Norm.x / dotN3 )*N3;
    
    float3	GNy =  ( V0.Norm.y / dotN0 )*N0;
            GNy += ( V1.Norm.y / dotN1 )*N1;
            GNy += ( V2.Norm.y / dotN2 )*N2;
            GNy += ( V3.Norm.y / dotN3 )*N3;
    
    float3	GNz =  ( V0.Norm.z / dotN0 )*N0;
            GNz += ( V1.Norm.z / dotN1 )*N1;
            GNz += ( V2.Norm.z / dotN2 )*N2;
            GNz += ( V3.Norm.z / dotN3 )*N3;
            
    V0.Norm = V0.Norm;
    V0.TanT = V0.TanT;
    V0.Tex = V0.Tex;
    V0.pos0 = V0.vPos;
    V0.GtxNx.xyz = Gtx;
    V0.GtyNx.xyz = Gty;
    V0.GtzNx.xyz = Gtz;
    V0.GTxNy.xyz = GTx;
    V0.GTyNy.xyz = GTy;
    V0.GTzNy.xyz = GTz;
    V0.GtxNx.w = GNx.x;
    V0.GtyNx.w = GNx.y;
    V0.GtzNx.w = GNx.z;
    V0.GTxNy.w = GNy.x;
    V0.GTyNy.w = GNy.y;
    V0.GTzNy.w = GNy.z;
    V0.GNz = GNz;
    
    V1.Norm = V0.Norm;
    V1.TanT = V0.TanT;
    V1.Tex = V0.Tex;
    V1.pos0 = V0.vPos;
    V1.GtxNx.xyz = Gtx;
    V1.GtyNx.xyz = Gty;
    V1.GtzNx.xyz = Gtz;
    V1.GTxNy.xyz = GTx;
    V1.GTyNy.xyz = GTy;
    V1.GTzNy.xyz = GTz;
    V1.GtxNx.w = GNx.x;
    V1.GtyNx.w = GNx.y;
    V1.GtzNx.w = GNx.z;
    V1.GTxNy.w = GNy.x;
    V1.GTyNy.w = GNy.y;
    V1.GTzNy.w = GNy.z;
    V1.GNz = GNz;
    
    V2.Norm = V0.Norm;
    V2.TanT = V0.TanT;
    V2.Tex = V0.Tex;
    V2.pos0 = V0.vPos;
    V2.GtxNx.xyz = Gtx;
    V2.GtyNx.xyz = Gty;
    V2.GtzNx.xyz = Gtz;
    V2.GTxNy.xyz = GTx;
    V2.GTyNy.xyz = GTy;
    V2.GTzNy.xyz = GTz;
    V2.GtxNx.w = GNx.x;
    V2.GtyNx.w = GNx.y;
    V2.GtzNx.w = GNx.z;
    V2.GTxNy.w = GNy.x;
    V2.GTyNy.w = GNy.y;
    V2.GTzNy.w = GNy.z;
    V2.GNz = GNz;
    
    V3.Norm = V0.Norm;
    V3.TanT = V0.TanT;
    V3.Tex = V0.Tex;
    V3.pos0 = V0.vPos;
    V3.GtxNx.xyz = Gtx;
    V3.GtyNx.xyz = Gty;
    V3.GtzNx.xyz = Gtz;
    V3.GTxNy.xyz = GTx;
    V3.GTyNy.xyz = GTy;
    V3.GTzNy.xyz = GTz;
    V3.GtxNx.w = GNx.x;
    V3.GtyNx.w = GNx.y;
    V3.GtzNx.w = GNx.z;
    V3.GTxNy.w = GNy.x;
    V3.GTyNy.w = GNy.y;
    V3.GTzNy.w = GNy.z;
    V3.GNz = GNz;
}

void GSCreateTetra( in VSDisplaceOut A, in VSDisplaceOut B, in VSDisplaceOut C, in VSDisplaceOut D,
                    inout TriangleStream<PSDisplaceIn> DisplaceStream )
{	
    float3 AView = normalize( A.vPos - g_vEyePt );
    float3 BView = normalize( B.vPos - g_vEyePt );
    float3 CView = normalize( C.vPos - g_vEyePt );
    float3 DView = normalize( D.vPos - g_vEyePt );
    
    PSDisplaceIn Aout;
    Aout.Pos = A.Pos;
    Aout.vPos = A.vPos;
    Aout.Norm = A.Norm;
    Aout.Tex = A.Tex;
    Aout.TanT = A.Tangent;
    
    PSDisplaceIn Bout;
    Bout.Pos = B.Pos;
    Bout.vPos = B.vPos;
    Bout.Norm = B.Norm;
    Bout.Tex = B.Tex;
    Bout.TanT = B.Tangent;
    
    PSDisplaceIn Cout;
    Cout.Pos = C.Pos;
    Cout.vPos = C.vPos;
    Cout.Norm = C.Norm;
    Cout.Tex = C.Tex;
    Cout.TanT = C.Tangent;
    
    PSDisplaceIn Dout;
    Dout.Pos = D.Pos;
    Dout.vPos = D.vPos;
    Dout.Norm = D.Norm;
    Dout.Tex = D.Tex;
    Dout.TanT = D.Tangent;
    
    float3 AB = C.vPos-B.vPos;
    float3 AC = D.vPos-B.vPos;
    float3 planeNormA = normalize( cross( AC, AB ) );
           AB = D.vPos-A.vPos;
           AC = C.vPos-A.vPos;
    float3 planeNormB = normalize( cross( AC, AB ) );
           AB = B.vPos-A.vPos;
           AC = D.vPos-A.vPos;
    float3 planeNormC = normalize( cross( AC, AB ) );
           AB = C.vPos-A.vPos;
           AC = B.vPos-A.vPos;
    float3 planeNormD = normalize( cross( AC, AB ) );
    
    Aout.planeDist.x = Aout.planeDist.y = Aout.planeDist.z = 0.0f;
    Aout.planeDist.w = RayDistToPlane( A.vPos, AView, B.vPos, planeNormA );
    
    Bout.planeDist.x = Bout.planeDist.z = Bout.planeDist.w = 0.0f;
    Bout.planeDist.y = RayDistToPlane( B.vPos, BView, A.vPos, planeNormB );
    
    Cout.planeDist.x = Cout.planeDist.y = Cout.planeDist.w = 0.0f;
    Cout.planeDist.z = RayDistToPlane( C.vPos, CView, A.vPos, planeNormC );
    
    Dout.planeDist.y = Dout.planeDist.z = Dout.planeDist.w = 0.0f;
    Dout.planeDist.x = RayDistToPlane( D.vPos, DView, A.vPos, planeNormD );
    
    CalcGradients( Aout, Bout, Cout, Dout, planeNormA, planeNormB, planeNormC, planeNormD );
    
    DisplaceStream.Append( Cout );
    DisplaceStream.Append( Bout );
    DisplaceStream.Append( Aout );
    DisplaceStream.Append( Dout );
    DisplaceStream.Append( Cout );
    DisplaceStream.Append( Bout );
    DisplaceStream.RestartStrip();
}

[maxvertexcount(18)]
void GSDisplaceMain( triangle VSDisplaceOut In[3], inout TriangleStream<PSDisplaceIn> DisplaceStream )
{
    //Don't extrude anything that's facing too far away from us
    //Just saves geometry generation
    float3 AB = In[1].Pos - In[0].Pos;
    float3 AC = In[2].Pos - In[0].Pos;
    float3 triNorm = cross( AB, AC );
    float lenTriNorm = length( triNorm );
    triNorm /= lenTriNorm;
    
    //Extrude along the Normals
    VSDisplaceOut v[6];
    [unroll] for( int i=0; i<3; i++ )
    {
        float4 PosNew = In[i].Pos;
        float4 PosExt = PosNew + float4(In[i].Norm*g_MaxDisplacement,0);
        v[i].vPos = PosNew.xyz;
        v[i+3].vPos = PosExt.xyz;
        
        v[i].Pos = mul( PosNew, g_mViewProj );
        v[i+3].Pos = mul( PosExt, g_mViewProj );
        
        v[i].Tex = float3(In[i].Tex.xy,0);
        v[i+3].Tex = float3(In[i].Tex.xy,1);
        
        v[i].Norm = In[i].Norm;
        v[i+3].Norm = In[i].Norm;
        
        v[i].Tangent = In[i].Tangent;
        v[i+3].Tangent = In[i].Tangent;
    }

    // Make sure that our prism hasn't "flipped" on itself after the extrusion
    AB = v[4].vPos - v[3].vPos;
    AC = v[5].vPos - v[3].vPos;
    float3 topNorm = cross( AB, AC );
    float lenTop = length( topNorm );
    topNorm /= lenTop;
    if( 
		lenTriNorm < 0.005f ||						//avoid tiny triangles
        dot( topNorm, triNorm ) < 0.95f ||			//make sure the top of our prism hasn't flipped
        abs((lenTop-lenTriNorm)/lenTriNorm) > 10.0f//11.0f	//make sure we don't balloon out too much
        
        )
    {
        [unroll] for( int i=0; i<3; i++ )
        {
            float4 PosNew = In[i].Pos;
            float4 PosExt = PosNew + float4(In[i].Norm*g_MinDisplacement,0);
            v[i].vPos = PosNew.xyz;
            v[i+3].vPos = PosExt.xyz;
            
            v[i].Pos = mul( PosNew, g_mViewProj );
            v[i+3].Pos = mul( PosExt, g_mViewProj );
        }
    }
    
    int index[6] = {0,1,2,3,4,5};
        
    // Create 3 tetrahedra
    GSCreateTetra( v[index[4]], v[index[5]], v[index[0]], v[index[3]], DisplaceStream );
    GSCreateTetra( v[index[5]], v[index[0]], v[index[1]], v[index[4]], DisplaceStream );
    GSCreateTetra( v[index[0]], v[index[1]], v[index[2]], v[index[5]], DisplaceStream );
}

#define MAX_DIST 1000000000.0f
#define MAX_STEPS 16
#define MIN_STEPS 4
#define STEP_SIZE (1.0f/2048.0f)
#define OFFSET_MAX 0.1f	//maximum we can cover is 1/10th of the entire texture in one march
float4 PSDisplaceMain(PSDisplaceIn input) : SV_Target
{	
    float4 modDist = float4(0,0,0,0);
    modDist.x = input.planeDist.x > 0 ? input.planeDist.x : MAX_DIST;
    modDist.y = input.planeDist.y > 0 ? input.planeDist.y : MAX_DIST;
    modDist.z = input.planeDist.z > 0 ? input.planeDist.z : MAX_DIST;
    modDist.w = input.planeDist.w > 0 ? input.planeDist.w : MAX_DIST;
    
    // find distance to the rear of the tetrahedron
    float fDist = min( modDist.x, modDist.y );
    fDist = min( fDist, modDist.z );
    fDist = min( fDist, modDist.w );
    
    // find the texture coords of the entrance point
    float3 texEnter;
    float3 relPos = input.vPos-input.pos0;
    texEnter.x = dot( input.GtxNx.xyz,relPos ) + input.Tex.x;
    texEnter.y = dot( input.GtyNx.xyz,relPos ) + input.Tex.y;
    texEnter.z = dot( input.GtzNx.xyz,relPos ) + input.Tex.z;
    
    // find the exit position
    float3 viewExitDir = normalize( input.vPos - g_vEyePt )*fDist;
    float3 viewExit = input.vPos + viewExitDir;
    
    // find the texture coords of the exit point
    float3 texExit;
    relPos = viewExit-input.pos0;
    texExit.x = dot( input.GtxNx.xyz,relPos ) + input.Tex.x;
    texExit.y = dot( input.GtyNx.xyz,relPos ) + input.Tex.y;
    texExit.z = dot( input.GtzNx.xyz,relPos ) + input.Tex.z;
	
    // March along the Texture space view ray until we either hit something
    // or we exit the tetrahedral prism
    float3 tanGrad = texExit - texEnter;
    float fTanDist = length( float3(tanGrad.xy,0) );	//length in 2d texture space
    if( fTanDist > OFFSET_MAX )
		discard;
        
    int iSteps = min( ceil( fTanDist / STEP_SIZE ), MAX_STEPS-MIN_STEPS ) + MIN_STEPS;
    tanGrad /= iSteps-1.0f;
    float3 TexCoord = float3(0,0,0);
    bool bFound = false;
	
    float height = 0;
    int i = 0;
    for( i=0; (i<iSteps && !bFound); i++ )
    {
        TexCoord = texEnter + i*tanGrad;
        height = g_txDisplace.SampleLevel( g_samPoint, float2(TexCoord.xy), 0 );
        height = max( height, g_MinDisplacement );
        
        if( TexCoord.z <= height )
        {
            bFound = true;
        }
    }
    if( !bFound )
		discard;
	
    // lookup the normal from the normal map
    float3 texNormal = g_txNormal.Sample( g_samLinear, TexCoord.xy );
    texNormal *= 2.0;
    texNormal -= float3(1,1,1);
    
    float3 foundPos = input.vPos + viewExitDir*((float)i/(float)iSteps);
    relPos = foundPos - input.pos0;
    float3 nTanT;
    nTanT.x = dot( input.GTxNy.xyz,relPos ) + input.TanT.x;
    nTanT.y = dot( input.GTyNy.xyz,relPos ) + input.TanT.y;
    nTanT.z = dot( input.GTzNy.xyz,relPos ) + input.TanT.z;
    
    float3 nNormT;
    float3 GNx = float3( input.GtxNx.w, input.GtyNx.w, input.GtzNx.w );
    float3 GNy = float3( input.GTxNy.w, input.GTyNy.w, input.GTzNy.w );
    nNormT.x = dot( GNx,relPos ) + input.Norm.x;
    nNormT.y = dot( GNy,relPos ) + input.Norm.y;
    nNormT.z = dot( input.GNz,relPos ) + input.Norm.z;
    
    float3 nBiNormT = normalize( cross( nNormT, nTanT ) );
    float3x3 BTNMatrix = float3x3( nBiNormT, nTanT, nNormT );
    texNormal = normalize( mul( texNormal, BTNMatrix ) ); //world space bump
    
    // Move the light orientation into Texture space
    float3 lightDir = normalize( g_vLightPos - input.vPos );
    float3 viewDir = normalize( -viewExitDir );
    
    // dot with Texture space light vector
    float lightAmt = saturate( dot( lightDir, texNormal ) );
    float4 lightColor = lightAmt.xxxx*g_directional + g_ambient;
    
    // Get the Diffuse and Specular Textures
    float4 diffuse = g_txDiffuse.Sample( g_samLinear, TexCoord.xy );
    float specular = diffuse.a;

    // Calculate specular power
    float3 halfAngle = normalize( viewDir + lightDir );
    float4 spec = saturate( pow( dot( halfAngle, texNormal ), 64 ) );
    
    // Return combined lighting
    return lightColor*diffuse*kDiffuse + spec*g_objectspeccolor*specular;
}

PSSceneIn VSScenemain(VSSceneIn input)
{
    PSSceneIn output;
    
    float4 wPos = mul( float4(input.Pos,1),g_mWorld );
    
    //calculate the color from the projected texture
    float4 shadowCoord = mul( wPos, g_mLightViewProj );
    shadowCoord.xy = 0.5 * shadowCoord.xy / shadowCoord.w + float2( 0.5, 0.5 ); 
    shadowCoord.y = -shadowCoord.y;	//flipy
    
    output.Pos = mul( float4(input.Pos,1), g_mWorldViewProj );
    output.Norm = mul( input.Norm, (float3x3)g_mWorld );
    output.Tan = mul( input.Tan, (float3x3)g_mWorld );
    output.Tex = input.Tex;
    output.ShadowTex = shadowCoord.xy;
    output.LightDir = g_vLightPos - wPos.xyz;
    output.ViewDir = g_vEyePt - wPos.xyz;
    
    return output;
}

float4 PSScenemain(PSSceneIn input) : SV_TARGET
{	
    float4 diffuse = g_txDiffuse.Sample( g_samLinear, input.Tex );
    float shadow = g_txShadow.Sample( g_samLinearShadow, input.ShadowTex ).r;
    float3 Norm = g_txNormal.Sample( g_samLinear, input.Tex );
    Norm *= 2.0;
    Norm -= float3(1,1,1);
    
    //move bump into world space
    float3 Binorm = normalize( cross( input.Norm, input.Tan ) );
    float3x3 BTNMatrix = float3x3( Binorm, input.Tan, input.Norm );
    Norm = mul( Norm, BTNMatrix ); //world space bump
    
    float3 NormLightDir = normalize( input.LightDir );
    float3 NormViewDir = normalize( input.ViewDir );
    float4 light = saturate( dot( NormLightDir, Norm ) );
    float3 halfAngle = normalize( NormViewDir + NormLightDir );
    float4 spec = saturate( pow( dot( halfAngle, Norm ), 4 ) );
    
    // colorize the light
    light *= g_directional*shadow;
    light += g_ambient;
    
    return diffuse*light + spec*g_scenespeccolor;   
}

PSQuadIn VSBlurmain( VSQuadIn input )
{
    PSQuadIn output;
    output.Pos = float4(input.Pos,1);
    output.Tex = input.Tex;
    return output;
}

float4 PSBlurmain( PSQuadIn input, uniform bool bHorizontal ) : SV_TARGET
{
    float4 color = float4(0,0,0,0);
    if( bHorizontal )
    {
        for( int i=-4; i<5; i++ )
        {
            color += g_txDiffuse.Sample( g_samClamp, input.Tex, int2(i,0) );
        }
    }
    else
    {
        for( int i=-4; i<5; i++ )
        {
            color += g_txDiffuse.Sample( g_samClamp, input.Tex, int2(0,i) );
        }
    }
    return color / 9.0f;
}

technique10 RenderNormal
{
    pass p0
    {
        SetVertexShader( CompileShader( vs_4_0, VSNormalmain() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PSNormalmain() ) );
        
        SetBlendState( NoBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( EnableDepthTestWrite, 0 );
        SetRasterizerState( EnableCulling );
    }  
}

technique10 RenderDisplaced
{
    pass p0
    {
        SetVertexShader( CompileShader( vs_4_0, VSDisplaceMain() ) );
        SetGeometryShader( CompileShader( gs_4_0, GSDisplaceMain() ) );
        SetPixelShader( CompileShader( ps_4_0, PSDisplaceMain() ) );
        
        SetBlendState( NoBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( EnableDepthTestWrite, 0 );
        SetRasterizerState( EnableCulling );
    }  
}

technique10 RenderScene
{
    pass p0
    {
        SetVertexShader( CompileShader( vs_4_0, VSScenemain() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PSScenemain() ) );
        
        SetBlendState( NoBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( EnableDepthTestWrite, 0 );
        SetRasterizerState( EnableCulling );
    }  
}

technique10 RenderBlack
{
    pass p0
    {
        SetVertexShader( CompileShader( vs_4_0, VSNormalmain() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PSSceneBlack() ) );
        
        SetBlendState( NoBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DisableDepthTestWrite, 0 );
        SetRasterizerState( EnableCulling );
    }  
}

technique10 BlurHorz
{
    pass p0
    {
        SetVertexShader( CompileShader( vs_4_0, VSBlurmain() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PSBlurmain(true) ) );
        
        SetBlendState( NoBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DisableDepthTestWrite, 0 );
        SetRasterizerState( EnableCulling );
    }  
}

technique10 BlurVert
{
    pass p0
    {
        SetVertexShader( CompileShader( vs_4_0, VSBlurmain() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PSBlurmain(false) ) );
        
        SetBlendState( NoBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DisableDepthTestWrite, 0 );
        SetRasterizerState( EnableCulling );
    }  
}

