using System;
using System.Runtime.InteropServices;

using Microsoft.DirectX;
using Microsoft.DirectX.Direct3D;
using Microsoft.Samples.DirectX.UtilityToolkit;

namespace PrtPerVertexSample
{
    /// <summary>
    /// Encapsulation of skybox geometry and textures.
    /// </summary>
    public class Skybox
    {
        public struct SkyboxVertexFormat
        {
            public Vector4 Position;
            public Vector3 Texture;
        };

        #region Constants
        private VertexElement[] skyboxDeclaration =
        {
            new VertexElement(0, 0,  DeclarationType.Float4, DeclarationMethod.Default, DeclarationUsage.Position, 0),
            VertexElement.VertexDeclarationEnd
        };
        #endregion

        #region Instance Data
        private CubeTexture environmentMap = null;
        private CubeTexture environmentMapSH = null;
        private Effect effect = null;
        private VertexBuffer vertexBuffer = null;
        private VertexDeclaration vertexDeclaration = null;
        private Device device = null;   
        private float size = 1.0f;
        private bool drawSH = false;
        #endregion

        #region Properties
        public CubeTexture EnvironmentMap { get { return environmentMap; } }
        public bool DrawSH { get { return drawSH; } set { drawSH = value; } }
        #endregion

        public void OnCreateDevice( Device device, float size, CubeTexture environmentMap, string effectFileName )
        {
            this.device = device;
            this.size = size;
            this.environmentMap = environmentMap;

            // Define DEBUG_VS and/or DEBUG_PS to debug vertex and/or pixel shaders with the shader debugger.  
            // Debugging vertex shaders requires either REF or software vertex processing, and debugging 
            // pixel shaders requires REF.  The "ShaderFlags.Force[Pixel/Vertex]ShaderSoftwareNoOptimizations" flag improves the debug 
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

            // Read the D3DX effect file
            string fileNameWithPath = Utility.FindMediaFile(effectFileName);

            // If this fails, there should be debug output as to 
            // they the .fx file failed to compile
            effect = Effect.FromFile(device, fileNameWithPath , null, null, shaderFlags, null);

            // Create vertex declaration
            vertexDeclaration = new VertexDeclaration(device, skyboxDeclaration);
        }

        public void InitSH(CubeTexture shTexture)
        {
            environmentMapSH = shTexture;
        }

        public void OnCreateDevice( Device device, float size, string cubeMapFileName, string effectFileName )
        {
            string mediaFileLongName = Utility.FindMediaFile(cubeMapFileName);
            
            CubeTexture environmentMap = ResourceCache.GetGlobalInstance().CreateCubeTextureFromFileEx(device, mediaFileLongName, 
                D3DX.Default, 1, Usage.None, Format.A16B16G16R16F, Pool.Managed, Filter.None, Filter.None, 0);

            OnCreateDevice( device, size, environmentMap, effectFileName );
        }

        public void OnResetDevice( SurfaceDescription backBufferSurfaceDesc )
        {
            if( effect != null )
                effect.OnResetDevice();

            vertexBuffer = new VertexBuffer(typeof(SkyboxVertexFormat), 4, device, Usage.WriteOnly, VertexFormats.None, Pool.Default);
            // Fill the vertex buffer
            SkyboxVertexFormat[] vertex = (SkyboxVertexFormat[]) vertexBuffer.Lock(0, LockFlags.None);
            try
            {
                // Map texels to pixels 
                float highW = -1.0f - (1.0f / (float)backBufferSurfaceDesc.Width);
                float highH = -1.0f - (1.0f / (float)backBufferSurfaceDesc.Height);
                float lowW  =  1.0f + (1.0f / (float)backBufferSurfaceDesc.Width);
                float lowH  =  1.0f + (1.0f / (float)backBufferSurfaceDesc.Height);

                vertex[0].Position = new Vector4(lowW,  lowH,  1.0f, 1.0f);
                vertex[1].Position = new Vector4(lowW,  highH, 1.0f, 1.0f);
                vertex[2].Position = new Vector4(highW, lowH,  1.0f, 1.0f);
                vertex[3].Position = new Vector4(highW, highH, 1.0f, 1.0f);
            }
            finally
            {
                vertexBuffer.Unlock();
                vertex = null;
            }
        }

        public void Render( Matrix worldViewProjection, float alpha, float scale )
        {
            Matrix invertedWorldViewProjection = Matrix.Invert(worldViewProjection);
            effect.SetValue( "invertedWorldViewProjection", invertedWorldViewProjection );

            if ((scale == 0.0f) || (alpha == 0.0f)) return; // do nothing if no intensity...

            // Draw the skybox
            int pass, numberPasses;
            effect.Technique = "Skybox";
            effect.SetValue( "alpha", alpha );
            effect.SetValue( "scale", alpha * scale );

            if(drawSH)
            {
                effect.SetValue( "environmentTexture", environmentMapSH );
            } 
            else 
            {
                effect.SetValue( "environmentTexture", environmentMap );
            }

            device.SetStreamSource( 0, vertexBuffer, 0, Marshal.SizeOf(typeof(SkyboxVertexFormat)) );
            device.VertexDeclaration = vertexDeclaration;

            numberPasses = effect.Begin(0);
            for( pass = 0; pass < numberPasses; pass++ )
            {
                effect.BeginPass( pass );
                device.DrawPrimitives( PrimitiveType.TriangleStrip, 0, 2 );
                effect.EndPass();
            }
            effect.End();
        }

        public void OnLostDevice()
        {
            if( effect != null)
                effect.OnLostDevice();

            if(vertexBuffer != null)
            {
                vertexBuffer.Dispose();
                vertexBuffer = null;
            }
        }

        public void OnDestroyDevice()
        {
            if(environmentMap != null)
            {
                environmentMap.Dispose();
                environmentMap = null;
            }

            if(environmentMapSH != null)
            {
                environmentMapSH.Dispose();
                environmentMapSH = null;
            }

            if(effect != null)
            {
                effect.Dispose();
                effect = null;
            }

            if(vertexDeclaration != null)
            {
                vertexDeclaration.Dispose();
                vertexDeclaration = null;
            }
        }
    }
}
