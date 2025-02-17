//--------------------------------------------------------------------------------------
// File: PrtMesh.cs
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//#define DEBUG_VS   // Uncomment this line to debug vertex shaders 
//#define DEBUG_PS   // Uncomment this line to debug pixel shaders 

using System;
using System.Collections;
using System.Drawing;
using System.IO;
using System.Text;
using Microsoft.DirectX;
using Microsoft.DirectX.Direct3D;
using Microsoft.Samples.DirectX.UtilityToolkit;

namespace PrtPerVertexSample
{
    /// <summary>
    /// The PrtMesh Exception
    /// </summary>
    public class PrtMeshException: DirectXSampleException
    {
        public PrtMeshException() : base("Exception in the PrtMesh class.") {}
        public PrtMeshException(string errorDescription) : base(errorDescription) {}
        public PrtMeshException(Exception inner) : base("Exception in the PrtMesh class.", inner) {}
    }

    class PrtMesh
    {
        private struct ReloadState
        {
            public bool  UseReloadState;
            public bool  LoadCompressed;
            public string MeshFileName;
            public string PrtBufferFileName;
            public CompressionQuality Quality;
            public int NumberClusters;
            public int NumberPcaVectors;
        };

        #region Constants
        // These constants are described in the article by Peter-Pike Sloan titled 
        // "Efficient Evaluation of Irradiance Environment Maps" in the book 
        // "ShaderX 2 - Shader Programming Tips and Tricks" by Wolfgang F. Engel.
        private static readonly float SqrtPI = (float) Math.Sqrt(Math.PI);
        private static readonly float C0 = 1.0f / (2.0f * SqrtPI);
        private static readonly float C1 = (float) Math.Sqrt(3.0f) / (3.0f * SqrtPI);
        private static readonly float C2 = (float) Math.Sqrt(15.0f) / (8.0f * SqrtPI);
        private static readonly float C3 = (float) Math.Sqrt(5.0f) /(16.0f * SqrtPI);
        private static readonly float C4 = 0.5f * C2;
        #endregion

        #region Instance Data
        private Mesh mesh = null;
        private ArrayList albedoTextures = new ArrayList();
        private ExtendedMaterial[] materials = null;
        private float objectRadius = 0.0f;
        private Vector3 objectCenter = new Vector3(0, 0, 0);

        private PrtBuffer prtBuffer = null;
        private PrtCompressedBuffer prtCompBuffer = null;
        
        private Effect ndotlEffect = null;
        private Effect prtEffect = null;
        private Effect shIrradEnvMapEffect = null;
        private int order = 0;

        private ReloadState reloadState;

        // The basis buffer is a large array of floats where 
        // Call D3DXSHPrtCompExtractBasis() to extract the basis 
        // for every cluster.  The basis for a cluster is an array of
        // (NumberPcaVectors + 1) * (NumberChannels * order^2) floats.
        // The "1+" is for the cluster mean.
        private float[] clusterBases = null;

        // prtConstants stores the incident radiance dotted with the transfer function.
        // Each cluster has an array of floats which is the size of 
        // 4 + MaxNumberChannels * NumberPcaVectors . This number comes from: there can 
        // be up to 3 channels (R,G,B), and each channel can 
        // have up to NumberPcaVectors of Pca vectors.  Each cluster also has 
        // a mean Pca vector which is described with 4 floats (and hence the +4).
        private float[] prtConstants = null;
        #endregion

        #region Properties
        public float Radius { get { return objectRadius; } set { objectRadius = value; } }
        public Vector3 Center { get { return objectCenter; } set { objectCenter = value; } }
        
        public Texture AlbedoTexture { get { return (Texture) albedoTextures[0]; } }
        public ExtendedMaterial[] Materials { get { return materials; } }
        public PrtCompressedBuffer CompressedBuffer { get { return prtCompBuffer; } }
        public int Order { get { return order; } }

        public bool IsMeshLoaded { get { return mesh != null; } }
        public bool IsUncompressedBufferLoaded { get { return prtBuffer != null; } }
        public bool IsCompBufferLoaded { get { return prtCompBuffer != null; } }
        public bool IsShaderDataExtracted { get { return clusterBases != null; } }
        public bool IsPrtEffectLoaded { get { return prtEffect != null; } }
        #endregion

        public Mesh Mesh { get{ return mesh;} } 

        public void SetMesh(Device device, Mesh value) 
        {    // Release any previous mesh object
            if(mesh != null)
            {
                mesh.Dispose();
                mesh = null;
            }

            mesh = AdjustMeshDecl( device, value);

            // Sort the attributes
            int[] adjacency = new int[mesh.NumberFaces * 3];
            mesh.GenerateAdjacency(1e-6f, adjacency);

            mesh.OptimizeInPlace(MeshFlags.OptimizeVertexCache | MeshFlags.OptimizeAttributeSort | MeshFlags.OptimizeIgnoreVerts, 
                adjacency);
        } 
        
        public void Dispose()
        {
            if(mesh != null)
            {
                mesh.Dispose();
                mesh = null;
            }

            if(prtCompBuffer != null)
            {
                prtCompBuffer.Dispose();
                prtCompBuffer = null;
            }

            if(prtBuffer != null)
            {
                prtBuffer.Dispose();
                prtBuffer = null;
            }
        }

        public void RenderWithPrt( Device device, Matrix worldViewProjection, bool isRenderWithAlbedoTexture )
        {
            prtEffect.SetValue( "worldViewProjection", worldViewProjection );

            bool hasAlbedoTexture = false;
            for(int i = 0; i < albedoTextures.Count; i++ )
            {
                
                if( albedoTextures[i] != null)
                {
                    hasAlbedoTexture = true;
                    break;
                }
            }
            if( !hasAlbedoTexture )
                isRenderWithAlbedoTexture = false;

            prtEffect.Technique = isRenderWithAlbedoTexture ? "RenderWithPrtColorLights"
                                                            : "RenderWithPrtColorLightsNoAlbedo";

            if( !isRenderWithAlbedoTexture )
            {
                prtEffect.SetValue( "MaterialDiffuseColor", PrtPerVertex.White);
            }

            int cPasses = prtEffect.Begin(0);

            for (int pass = 0; pass < cPasses; pass++)
            {
                prtEffect.BeginPass(pass);

                int attributeTableLength = mesh.GetAttributeTable().Length;
                for( int i = 0; i < attributeTableLength ; i++ )
                {            
                    if( isRenderWithAlbedoTexture )
                    {
                        if( albedoTextures.Count > i )
                            prtEffect.SetValue( "AlbedoTexture", albedoTextures[i] as Texture );

                        prtEffect.SetValue( "MaterialDiffuseColor", materials[i].Material3D.DiffuseColor);
                        prtEffect.CommitChanges();
                    }
                    mesh.DrawSubset(i);
                }

                prtEffect.EndPass();
            }

            prtEffect.End();
        }

        public void RenderWithSHIrradEnvMap( Device device, Matrix worldViewProjection, bool isRenderWithAlbedoTexture )
        {
            shIrradEnvMapEffect.SetValue( "worldViewProjection", worldViewProjection );

            bool hasAlbedoTexture = false;
            for(int i = 0; i < albedoTextures.Count; i++ )
            {
                if( albedoTextures[i] != null )
                {
                    hasAlbedoTexture = true;
                    break;
                }
            }
            if( !hasAlbedoTexture )
                isRenderWithAlbedoTexture = false;

            shIrradEnvMapEffect.Technique = isRenderWithAlbedoTexture ? "RenderWithSHIrradEnvMap"
                                                                    : "RenderWithSHIrradEnvMapNoAlbedo";

            if( !isRenderWithAlbedoTexture )
            {
                shIrradEnvMapEffect.SetValue("MaterialDiffuseColor", PrtPerVertex.White);
            }

            int cPasses = shIrradEnvMapEffect.Begin(0);

            for (int pass = 0; pass < cPasses; pass++)
            {
                shIrradEnvMapEffect.BeginPass(pass);

                int attributeTableLength = mesh.GetAttributeTable().Length;
                for( int i = 0; i < attributeTableLength ; i++ )
                {
                    if( isRenderWithAlbedoTexture )
                    {
                        if( albedoTextures.Count > i )
                            shIrradEnvMapEffect.SetValue( "AlbedoTexture", albedoTextures[i] as Texture );
                        shIrradEnvMapEffect.SetValue( "MaterialDiffuseColor", ColorValue.FromColor(materials[i].Material3D.Diffuse) );
                        shIrradEnvMapEffect.CommitChanges();
                    }
                    mesh.DrawSubset(i);
                }

                shIrradEnvMapEffect.EndPass();
            }

            shIrradEnvMapEffect.End();
        }

        public void RenderWithNDotL( Device device, Matrix worldViewProjection, Matrix worldInv, bool isRenderWithAlbedoTexture, DirectionWidget[] lightControl, int numberLights, float lightScale )
        {
            ndotlEffect.SetValue( "worldViewProjection", worldViewProjection );
            ndotlEffect.SetValue( "worldInv", worldInv );

            Vector4[] lightDir = new Vector4[PrtPerVertex.MaximumLights];
            Vector4[] lightsDiffuse = new Vector4[PrtPerVertex.MaximumLights];
            Vector4 lightOn = new Vector4(1,1,1,1);
            Vector4 lightOff = new Vector4(0,0,0,0);
            lightOn *= lightScale;

            for( int i = 0; i < numberLights; i++ )
                lightDir[i] = new Vector4( lightControl[i].LightDirection.X, lightControl[i].LightDirection.Y, lightControl[i].LightDirection.Z, 0 );
            for( int i = 0; i < PrtPerVertex.MaximumLights; i++ )
                lightsDiffuse[i] = (numberLights > i) ? lightOn : lightOff;

            bool hasAlbedoTexture = false;
            for(int i = 0; i < albedoTextures.Count; i++ )
            {
                if( albedoTextures[i] != null )
                {
                    hasAlbedoTexture = true;
                    break;
                }
            }
            if( !hasAlbedoTexture )
                isRenderWithAlbedoTexture = false;

            ndotlEffect.Technique = isRenderWithAlbedoTexture ? "RenderWithNDotL"
                : "RenderWithNDotLNoAlbedo";

            if( isRenderWithAlbedoTexture )
            {
                ndotlEffect.SetValue( "MaterialDiffuseColor", PrtPerVertex.White);
            }

            int cPasses = ndotlEffect.Begin(0);

            for (int pass = 0; pass < cPasses; pass++)
            {
                ndotlEffect.BeginPass(pass);

                // 10 and 20 are the register constants
                device.SetVertexShaderConstant( 10, lightDir);
                device.SetVertexShaderConstant( 20, lightsDiffuse);

                int attributeTableLength = mesh.GetAttributeTable().Length;
                for( int i = 0; i < attributeTableLength ; i++ )
                {
                    if( isRenderWithAlbedoTexture )
                    {
                        if( albedoTextures.Count> i )
                            ndotlEffect.SetValue( "AlbedoTexture", albedoTextures[i] as Texture );
                        ndotlEffect.SetValue( "MaterialDiffuseColor", ColorValue.FromColor(materials[i].Material3D.Diffuse) );
                        ndotlEffect.CommitChanges();
                    }
                    mesh.DrawSubset(i);
                }

                ndotlEffect.EndPass();
            }

            ndotlEffect.End();
        }

        public void LoadPrtBufferFromFile( string fileName )
        {
            if(prtBuffer != null)
            {
                prtBuffer.Dispose();
                prtBuffer = null;
            }
            if(prtCompBuffer != null)
            {
                prtCompBuffer.Dispose();
                prtCompBuffer = null;
            }

            string mediaFileLongName = Utility.FindMediaFile(fileName);
            reloadState.PrtBufferFileName =  mediaFileLongName;
            prtBuffer = PrtBuffer.FromFile(mediaFileLongName);
            order = GetOrderFromNumCoefficients( prtBuffer.NumberCoefficients);
        }

        public void LoadCompPrtBufferFromFile( string fileName )
        {
            if(prtBuffer != null)
            {
                prtBuffer.Dispose();
                prtBuffer = null;
            }
            if(prtCompBuffer != null)
            {
                prtCompBuffer.Dispose();
                prtCompBuffer = null;
            }

            string mediaFileLongName;
            if (File.Exists(fileName))
            {
                mediaFileLongName = fileName;
            }
            else
                mediaFileLongName = Utility.FindMediaFile(fileName);
            reloadState.PrtBufferFileName = mediaFileLongName;

            prtCompBuffer = PrtCompressedBuffer.FromFile(reloadState.PrtBufferFileName);
            reloadState.UseReloadState = true;
            reloadState.LoadCompressed = true;
            order = GetOrderFromNumCoefficients( prtCompBuffer.NumberCoefficients);
        }

        public void LoadEffects( Device device, Caps caps)
        {
            int numberChannels      = prtCompBuffer.NumberChannels;
            int numberClusters      = prtCompBuffer.NumberClusters;
            int numberPcaVectors    = prtCompBuffer.NumberPcaVectors;

            // The number of vertex consts need by the shader can't exceed the 
            // amount the HW can support
            int numberVConsts = numberClusters * (1 + numberChannels * numberPcaVectors / 4) + 4;
            if( numberVConsts > caps.MaxVertexShaderConst )
                throw new PrtMeshException("Number of Vertex shader constants cannot exceed the amount the Hardware can support");

            if(prtEffect != null)
            {
                prtEffect.Dispose();
                prtEffect = null;
            }
            if(shIrradEnvMapEffect != null)
            {
                shIrradEnvMapEffect.Dispose();
                shIrradEnvMapEffect = null;
            }
            if(ndotlEffect != null)
            {
                ndotlEffect.Dispose();
                ndotlEffect = null;
            }

            Macro[] defines = new Macro[3];

            string maximumNumberClusters = string.Format( "{0}", numberClusters );
            string maximumNumberPcaVectors = string.Format("{0}", numberPcaVectors );
            defines[0].Name = "NumberClusters";
            defines[0].Definition = maximumNumberClusters;
            defines[1].Name = "NumberPcaVectors";
            defines[1].Definition = maximumNumberPcaVectors;
            defines[2].Name = String.Empty;
            defines[2].Definition = String.Empty;

            // Define DEBUG_VS and/or DEBUG_PS to debug vertex and/or pixel shaders with the shader debugger.  
            // Debugging vertex shaders requires either REF or software vertex processing, and debugging 
            // pixel shaders requires REF.  The "ShaderFlags.Force{Pixel/Vertex}ShaderSoftwareNoOptimizations" flag improves the debug 
            // experience in the shader debugger.  It enables source level debugging, prevents instruction 
            // reordering, prevents dead code elimination, and forces the compiler to compile against the next 
            // higher available software target, which ensures that the unoptimized shaders do not exceed 
            // the shader model limitations.  Setting these flags will cause slower rendering since the shaders 
            // will be unoptimized and forced into software.  See the DirectX documentation for more information 
            // about using the shader debugger.
            ShaderFlags shaderFlags = ShaderFlags.NotCloneable;
            #if(DEBUG_VS)
                shaderFlags |= ShaderFlags.ForceVertexShaderSoftwareNoOptimizations;
            #endif
            #if(DEBUG_PS)
                shaderFlags |= ShaderFlags.ForcePixelShaderSoftwareNoOptimizations;
            #endif
        
            // Read the effect file
            string mediaFileLongName = Utility.FindMediaFile("PRTColorLights.fx");

            // If this fails, there should be debug output
            // that the .fx file failed to compile
            string errors;
            prtEffect = Effect.FromFile(device, mediaFileLongName, defines, null, null, shaderFlags, null, out errors);
            if ( (errors != null) && (errors.Length > 0) )
                throw new InvalidOperationException(errors);

            // Make sure the technique works on this card
            prtEffect.ValidateTechnique("RenderWithPrtColorLights");

            mediaFileLongName = Utility.FindMediaFile("SimpleLighting.fx");
            ndotlEffect = Effect.FromFile(device, mediaFileLongName, null, null, shaderFlags, null);

            mediaFileLongName = Utility.FindMediaFile("SHIrradianceEnvMap.fx");
            shIrradEnvMapEffect = Effect.FromFile(device, mediaFileLongName, null, null, shaderFlags, null);
        }
   
        public void ComputeSHIrradEnvMapConstants( float[] shCoefficientsRed, float[] shCoefficientsGreen, float[] shCoefficientsBlue )
        {
            float[][] light = { shCoefficientsRed, shCoefficientsGreen, shCoefficientsBlue };

            // Lighting environment coefficients
            Vector4[] coefficients = new Vector4[3];

            for( int channel = 0; channel < 3; channel++ )
            {
                coefficients[channel].X = -C1 * light[channel][3];
                coefficients[channel].Y = -C1 * light[channel][1];
                coefficients[channel].Z =  C1 * light[channel][2];
                coefficients[channel].W =  C0 * light[channel][0] - C3 * light[channel][6];
            }

            shIrradEnvMapEffect.SetValue("ar", coefficients[0]);
            shIrradEnvMapEffect.SetValue("ag", coefficients[1]);
            shIrradEnvMapEffect.SetValue("ab", coefficients[2]);

            for( int channel = 0; channel < 3; channel++ )
            {
                coefficients[channel].X =        C2 * light[channel][4];
                coefficients[channel].Y =       -C2 * light[channel][5];
                coefficients[channel].Z = 3.0f * C3 * light[channel][6];
                coefficients[channel].W =       -C2 * light[channel][7];
            }

            shIrradEnvMapEffect.SetValue("br", coefficients[0]);
            shIrradEnvMapEffect.SetValue("bg", coefficients[1]);
            shIrradEnvMapEffect.SetValue("bb", coefficients[2]);

            coefficients[0].X = C4 * light[0][8];
            coefficients[0].Y = C4 * light[1][8];
            coefficients[0].Z = C4 * light[2][8];
            coefficients[0].W = 1.0f;

            shIrradEnvMapEffect.SetValue("c", coefficients[0]);
        }

        public void ComputeShaderConstants( float[] shCoefficientsRed, float[] shCoefficientsGreen, float[] shCoefficientsBlue, int numberCoefficientsPerChannel )
        {
            System.Diagnostics.Debug.Assert(numberCoefficientsPerChannel == prtCompBuffer.NumberCoefficients);

            int numberCoefficients  = prtCompBuffer.NumberCoefficients;
            int order               = this.order;
            int numberChannels      = prtCompBuffer.NumberChannels;
            int numberClusters      = prtCompBuffer.NumberClusters;
            int numberPcaVectors    = prtCompBuffer.NumberPcaVectors;

            //
            // With compressed PRT, a single diffuse channel is caluated by:
            //       R[p] = (M[k] dot L') + sum( w[p][j] * (B[k][j] dot L');
            // where the sum runs j between 0 and # of PCA vectors
            //       R[p] = exit radiance at point p
            //       M[k] = mean of cluster k 
            //       L' = source radiance approximated with SH coefficients
            //       w[p][j] = the j'th PCA weight for point p
            //       B[k][j] = the j'th PCA basis vector for cluster k
            //
            // Note: since both (M[k] dot L') and (B[k][j] dot L') can be computed on the CPU, 
            // these values are passed as constants using the array prtConstants.   
            // 
            // So we compute an array of floats, prtConstants, here.
            // This array is the L' dot M[k] and L' dot B[k][j].
            // The source radiance is the lighting environment in terms of spherical
            // harmonic coefficients which can be computed with SphericalHarmonics.Evaluate* or SphericalHarmonics.ProjectCubeMap.
            // M[k] and B[k][j] are also in terms of spherical harmonic basis coefficients 
            // and come from PrtCompressedBuffer.ExtractBasis().
            //
            
            int clusterStride = numberChannels * numberPcaVectors + 4;
            int basisStride = numberCoefficients * numberChannels * (numberPcaVectors + 1);  

            float[] clusterBasesSegment = new float[numberCoefficients];
            for( int cluster = 0; cluster < numberClusters; cluster++ ) 
            {
                // For each cluster, store L' dot M[k] per channel, where M[k] is the mean of cluster k
                Array.Copy(clusterBases, cluster * basisStride + 0 * numberCoefficients, clusterBasesSegment, 0, numberCoefficients);
                prtConstants[cluster * clusterStride + 0] = SphericalHarmonics.Dot(order, clusterBasesSegment, shCoefficientsRed);

                Array.Copy(clusterBases, cluster * basisStride + 1 * numberCoefficients, clusterBasesSegment, 0, numberCoefficients);
                prtConstants[cluster * clusterStride + 1] = SphericalHarmonics.Dot(order, clusterBasesSegment, shCoefficientsGreen);
                
                Array.Copy(clusterBases, cluster * basisStride + 2 * numberCoefficients, clusterBasesSegment, 0, numberCoefficients);
                prtConstants[cluster * clusterStride + 2] = SphericalHarmonics.Dot(order, clusterBasesSegment, shCoefficientsBlue);
                
                prtConstants[cluster * clusterStride + 3] = 0.0f;

                // Then per channel we compute L' dot B[k][j], where B[k][j] is the jth PCA basis vector for cluster k
                int offset1 = cluster * clusterStride + 4;
                for( int pca = 0; pca < numberPcaVectors; pca++ ) 
                {
                    int offset2 = cluster * basisStride + (pca + 1) * numberCoefficients * numberChannels;

                    Array.Copy(clusterBases, offset2 + 0 * numberCoefficients, clusterBasesSegment, 0, numberCoefficients);
                    prtConstants[offset1 + 0 * numberPcaVectors + pca] = SphericalHarmonics.Dot(order, clusterBasesSegment, shCoefficientsRed);

                    Array.Copy(clusterBases, offset2 + 1 * numberCoefficients, clusterBasesSegment, 0, numberCoefficients);
                    prtConstants[offset1 + 1 * numberPcaVectors + pca] = SphericalHarmonics.Dot(order, clusterBasesSegment, shCoefficientsGreen);
                
                    Array.Copy(clusterBases, offset2 + 2 * numberCoefficients, clusterBasesSegment, 0, numberCoefficients);
                    prtConstants[offset1 + 2 * numberPcaVectors + pca] = SphericalHarmonics.Dot(order, clusterBasesSegment, shCoefficientsBlue);
                }
            }

            prtEffect.SetValue("prtConstants", prtConstants);
        }

        public void SetPrtBuffer( PrtBuffer prtBufferLocal, string fileName ) 
        { 
            if(prtBuffer != null)
            {
                prtBuffer.Dispose();
                prtBuffer = null;
            }

            if(prtCompBuffer != null)
            {
                prtCompBuffer.Dispose();
                prtCompBuffer = null;
            }

            prtBuffer = prtBufferLocal;
            order = GetOrderFromNumCoefficients( (int) prtBuffer.NumberCoefficients);

            reloadState.PrtBufferFileName = fileName;
        }

        /// <summary>
        /// Make the mesh have a known decl in order to pass per vertex CPCA 
        /// data to the shader
        /// </summary>
        public Mesh AdjustMeshDecl( Device device, Mesh mesh)
        {
            VertexElement[] declaration =
            {
                new VertexElement(0,  0,  DeclarationType.Float3, DeclarationMethod.Default, DeclarationUsage.Position, 0),
                new VertexElement(0,  12, DeclarationType.Float3, DeclarationMethod.Default, DeclarationUsage.Normal, 0),
                new VertexElement(0,  24, DeclarationType.Float2, DeclarationMethod.Default, DeclarationUsage.TextureCoordinate, 0),
                new VertexElement(0,  32, DeclarationType.Float1, DeclarationMethod.Default, DeclarationUsage.BlendWeight, 0),
                new VertexElement(0,  36, DeclarationType.Float4, DeclarationMethod.Default, DeclarationUsage.BlendWeight, 1),
                new VertexElement(0,  52, DeclarationType.Float4, DeclarationMethod.Default, DeclarationUsage.BlendWeight, 2),
                new VertexElement(0,  68, DeclarationType.Float4, DeclarationMethod.Default, DeclarationUsage.BlendWeight, 3),
                new VertexElement(0,  84, DeclarationType.Float4, DeclarationMethod.Default, DeclarationUsage.BlendWeight, 4),
                new VertexElement(0, 100, DeclarationType.Float4, DeclarationMethod.Default, DeclarationUsage.BlendWeight, 5),
                new VertexElement(0, 116, DeclarationType.Float4, DeclarationMethod.Default, DeclarationUsage.BlendWeight, 6),
                VertexElement.VertexDeclarationEnd
            };
            // To do CPCA, we need to store (NumberPcaVectors + 1) scalers per vertex, so 
            // make the mesh have a known decl to store this data.  Since we can't do 
            // skinning and PRT at once, we use DeclarationUsage.BlendWeight[0] 
            // to DeclarationUsage.BlendWeight[6] to store our per vertex data needed for PRT.
            // Notice that DeclarationUsage.BlendWeight[0] is a float1, and
            // DeclarationUsage.BlendWeight[1]-DeclarationUsage.BlendWeight[6] are float4.  This allows 
            // up to 24 PCA weights and 1 float that gives the vertex shader 
            // an index into the vertex's cluster's data
            Mesh outMesh;
            using(mesh)
            {
                outMesh = mesh.Clone(mesh.Options.Value, declaration, device);

                // Make sure there are normals which are required for lighting
                if( (mesh.VertexFormat & VertexFormats.Normal) == 0)
                    outMesh.ComputeNormals((GraphicsStream) null);

            }
            mesh = null;

            return outMesh;
        }

        public void LoadMesh(Device device, string meshFileName)
        {
            // Release any previous mesh object
            if(mesh != null)
            {
                mesh.Dispose();
                mesh = null;
            }

            if(albedoTextures != null)
            {
                for(int i = 0; i < albedoTextures.Count; i++ )
                {
                    if(albedoTextures[i] != null)
                    {
                        (albedoTextures[i] as Texture).Dispose();
                    }
                }
                albedoTextures.Clear();
            }

            // Load the mesh object
            string mediaFileLongName = Utility.FindMediaFile(meshFileName);
            reloadState.MeshFileName = mediaFileLongName;
            mesh = Mesh.FromFile(mediaFileLongName, MeshFlags.Managed, device, out materials);

            // Change the current directory to the mesh's directory so we can
            // find the textures.
            string mediaFileDirectory = new FileInfo(mediaFileLongName).DirectoryName;
            string cwd = Directory.GetCurrentDirectory();
            Directory.SetCurrentDirectory( mediaFileDirectory );

            // Lock the vertex buffer to get the object's radius & center
            // simply to help position the camera a good distance away from the mesh.
            using(VertexBuffer vertexBuffer = mesh.VertexBuffer)
            {
                using(GraphicsStream vertices = vertexBuffer.Lock(0, 0, LockFlags.None))
                {
                    try
                    {
                        // Get declaration, and the size of the decl
                        VertexElement[] decl = mesh.Declaration;
                        int declSize = VertexInformation.GetDeclarationVertexSize(decl, 0);
                        // Calculate the bounding sphere based on this size
                        objectRadius = Geometry.ComputeBoundingSphere( vertices, 
                            mesh.NumberVertices, declSize, out objectCenter);
                    }
                    finally
                    {
                        vertexBuffer.Unlock();
                    }
                }
            }

            // Make the mesh have a known declaration in order to pass per vertex CPCA data to the shader
            mesh = AdjustMeshDecl( device, mesh);

            int[] adjacency = new int[mesh.NumberFaces * 3];
            int[] adjacencyOut, faceRemap;
            GraphicsStream vertRemap;
            mesh.GenerateAdjacency(1e-6f, adjacency);
            Mesh tempMesh = null;
            using( mesh)
            {
                // Attribute sort the faces & vertices.  This call will sort the verts independent of hardware
                tempMesh = mesh.Optimize(MeshFlags.OptimizeAttributeSort, adjacency, out adjacencyOut, out faceRemap, out vertRemap);
            }
            mesh = tempMesh;

            // Sort just the faces for optimal vertex cache perf on the current HW device.
            // But note that the vertices are not sorted for optimal vertex cache because
            // this would reorder the vertices depending on the HW which would make it difficult 
            // to precompute the PRT results for all HW.  Alternatively you could do the vcache 
            // optimize with vertex reordering after putting the PRT data into the VB for a 
            // more optimal result
            using( mesh)
            {
                // Attribute sort the faces & vertices.  This call will sort the verts independent of hardware
                tempMesh = mesh.Optimize(MeshFlags.OptimizeVertexCache | MeshFlags.OptimizeIgnoreVerts, adjacencyOut );
            }
            mesh = tempMesh;


            for(int i = 0; i < materials.Length; i++ )
            {
                // First attempt to look for texture in the same folder as the input folder.
                // Create the mesh texture from a file
                try
                {
                    mediaFileLongName = Utility.FindMediaFile(materials[i].TextureFilename);
                }
                catch(MediaNotFoundException)
                {
                    albedoTextures.Add(null);
                    continue;
                }

                Texture albedoTexture = ResourceCache.GetGlobalInstance().CreateTextureFromFile(device, mediaFileLongName);
                albedoTextures.Add(albedoTexture);
            }
   
            Directory.SetCurrentDirectory( cwd );
        }

        private int GetOrderFromNumCoefficients( int numberCoefficients )
        {
            int order = 1;

            while(order * order < numberCoefficients) 
                order++;

            return order;
        }

        public void CompressBuffer( CompressionQuality quality, int numberClusters, int numberPcaVectors)
        {
            System.Diagnostics.Debug.Assert(prtBuffer != null);

            if(prtCompBuffer != null)
            {
                prtCompBuffer.Dispose();
                prtCompBuffer = null;
            }
            prtCompBuffer = new PrtCompressedBuffer(quality, numberClusters, numberPcaVectors, null, prtBuffer);
            reloadState.Quality = quality;
            reloadState.NumberClusters = numberClusters;
            reloadState.NumberPcaVectors = numberPcaVectors;
            reloadState.UseReloadState = true;
            reloadState.LoadCompressed = false;
            order = GetOrderFromNumCoefficients( prtBuffer.NumberCoefficients );
        }

        public void ExtractCompressedDataForPrtShader() 
        { 
            // First call PrtCompBuffer::NormalizeData.  This isn't nessacary, 
            // but it makes it easier to use data formats that have little presision.
            // It normalizes the Pca weights so that they are between [-1,1]
            // and modifies the basis vectors accordingly.  
            prtCompBuffer.NormalizeData();

            int numberSamples= prtCompBuffer.NumberSamples;
            int numberCoefficients = prtCompBuffer.NumberCoefficients;
            int numberChannels = prtCompBuffer.NumberChannels;
            int numberClusters = prtCompBuffer.NumberClusters;
            int numberPcaVectors = prtCompBuffer.NumberPcaVectors;

            // With clustered Pca, each vertex is assigned to a cluster.  To figure out 
            // which vertex goes with which cluster, call PrtCompBuffer::ExtractClusterIDs.
            // This will return a cluster ID for every vertex.  Simply pass in an array of UINTs
            // that is the size of the number of vertices (which also equals the number of samples), and 
            // the cluster ID for vertex N will be at puClusterIDs[N].
            int[] clusterIDs = new int[ numberSamples ];
            prtCompBuffer.ExtractClusterIDs( clusterIDs );


            // Now we also need to store the per vertex PCA weights.  Earilier when
            // the mesh was loaded, we changed the vertex decl to make room to store these
            // PCA weights.  In this sample, we will use DeclarationUsage.BlendWeight[1] to 
            // DeclarationUsage.BlendWeight[6].  Using DeclarationUsage.BlendWeight intead of some other 
            // usage was an arbritatey decision.  Since DeclarationUsage.BlendWeight[1-6] were 
            // declared as float4 then we can store up to 6*4 PCA weights per vertex.  They don't
            // have to be declared as float4, but its a reasonable choice.  So for example, 
            // if NumberPcaVectors=16 the function will write data to DeclarationUsage.BlendWeight[1-4]
            prtCompBuffer.ExtractToMesh( numberPcaVectors, DeclarationUsage.BlendWeight, 1, mesh );

            // Extract the cluster bases into a large array of floats.  
            // PrtCompressedBuffer::ExtractBasis will extract the basis 
            // for a single cluster.
            //
            // A single cluster basis is an array of
            // (NumberPcaVectors + 1) * NumberCoefficients * NumberChannels floats
            // The "1+" is for the cluster mean.
            int clusterBasisSize = (numberPcaVectors + 1) * numberCoefficients * numberChannels;
            int bufferSize       = clusterBasisSize * numberClusters; 

            clusterBases = new float[bufferSize];
            float[] clusterBasesSegment = new float[clusterBasisSize];
            for( int cluster = 0; cluster < numberClusters; cluster++ ) 
            {
                // ExtractBasis() extracts the basis for a single cluster at a time.
                prtCompBuffer.ExtractBasis( cluster, clusterBasesSegment);
                clusterBasesSegment.CopyTo(clusterBases, cluster * clusterBasisSize);
            }

            prtConstants = new float[ numberClusters * (4 + numberChannels * numberPcaVectors) ];
        }

        public void OnCreateDevice( Device device)
        {
            if( reloadState.UseReloadState )
            {
                LoadMesh( device, reloadState.MeshFileName );
                if( reloadState.LoadCompressed )
                {
                    LoadCompPrtBufferFromFile( reloadState.PrtBufferFileName );
                }
                else
                {
                    LoadPrtBufferFromFile( reloadState.PrtBufferFileName );
                    CompressBuffer( reloadState.Quality, reloadState.NumberClusters, reloadState.NumberPcaVectors );
                }
                ExtractCompressedDataForPrtShader();
                LoadEffects( device, PrtPerVertex.SampleFramework.DeviceCaps);
            }
        }


        public void OnResetDevice()
        {
            if( prtEffect != null )
                prtEffect.OnResetDevice();

            if( shIrradEnvMapEffect != null)
                shIrradEnvMapEffect.OnResetDevice();

            if( ndotlEffect != null )
                ndotlEffect.OnResetDevice();
        }


        public void OnLostDevice()
        {
            if( prtEffect != null )
                prtEffect.OnLostDevice();

            if( shIrradEnvMapEffect != null)
                shIrradEnvMapEffect.OnLostDevice();

            if( ndotlEffect != null )
                ndotlEffect.OnLostDevice();
        }


        public void OnDestroyDevice()
        {
            if( !reloadState.UseReloadState )
                reloadState = new ReloadState();

            if(mesh != null)
            {
                mesh.Dispose();
                mesh = null;
            }
            if(albedoTextures != null)
            {
                for(int i = 0; i < albedoTextures.Count; i++ )
                {
                    if(albedoTextures[i] != null)
                    {
                        (albedoTextures[i] as Texture).Dispose();
                    }
                }
                albedoTextures.Clear();
            }

            if( prtEffect != null )
            {
                prtEffect.Dispose();
                prtEffect = null;
            }

            if( shIrradEnvMapEffect != null)
            {
                shIrradEnvMapEffect.Dispose();
                shIrradEnvMapEffect = null;
            }

            if( ndotlEffect != null )
            {
                ndotlEffect.Dispose();
                ndotlEffect = null;
            }

            if( prtBuffer != null)
            {
                prtBuffer.Dispose();
                prtBuffer = null;
            }

            if( prtCompBuffer != null)
            {
                prtCompBuffer.Dispose();
                prtCompBuffer = null;
            }
        }
    }
}
