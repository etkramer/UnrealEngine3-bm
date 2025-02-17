using System;
using Microsoft.DirectX;
using Microsoft.DirectX.Direct3D;
using Microsoft.Samples.DirectX.UtilityToolkit;
using System.Drawing;
using System.Collections;

namespace Simple2DSample
{

    /// <summary>
    /// This structure is used by UniformTileSet to
    /// track the destination data of a single on-screen tile.
    /// </summary>
    public struct TileInstance 
    {
        public int TileIndex;
        public float X;
        public float Y;
        public float Z;
        public Color Color;
    }

    /// <summary>
    /// An example usage of sprite with a tile texture.
    /// This assumes that all source tiles are the same size
    /// and all tiles will be scaled uniformly.
    /// </summary>
    public class UniformTileSet
    {
        Texture tileTex;
        int imgWidth, imgHeight;
        private Rectangle[] tiles;
        private float tileWidth, tileHeight;
        private float destWidth, destHeight;
        

        /// <summary>
        /// 
        /// </summary>
        /// <param name="t">The texture containing the tile images</param>
        /// <param name="width">The width of the source image.</param>
        /// <param name="height">The height of the source image.</param>
        public UniformTileSet(Texture t,int width, int height)
        {
            tileTex = t;
            imgWidth = width;
            imgHeight = height;
        }

        /// <summary>
        /// Creates a set of uniformly sized tiles from the texture image
        /// </summary>
        /// <param name="tileWidth">Width of the tile sources.</param>
        /// <param name="tileHeight">Height of the tile sources.</param>
        /// <param name="leftOffset">Offset in pixels from the left edge of the image at which to start the tile data</param>
        /// <param name="topOffset">Offset in pixels from the top edge of the image at which to start the tile data</param>
        /// <param name="numTilesX">Number of columns in the tile source grid</param>
        /// <param name="numTilesY">Number of rows in the tile source grid</param>
        public void CreateTiles(int width, int height, int leftOffset, int topOffset, int numTilesX, int numTilesY)
        {
            //clamp number of tiles to the avaialble resolution
            int maxXTiles = (imgWidth - leftOffset)  / width;
            if (maxXTiles > numTilesX) numTilesX = maxXTiles;
            int maxYTiles = (imgHeight - topOffset) / height;
            if (maxYTiles > numTilesY) numTilesY = maxXTiles;

            tiles = new Rectangle[numTilesX * numTilesY];
            tileWidth = width;
            tileHeight = height;

            for (int y = 0; y < numTilesY; y++ )
            {
                for (int x = 0; x < numTilesX; x++)
                {
                    int i = x + y * numTilesX;
                    tiles[i] = new Rectangle((int) (x * tileWidth), (int) (y * tileHeight), width, height);
                }
            }
        }

        /// <summary>
        /// Set the destination size of the tiles in pixels.
        /// </summary>
        public void SetTileDestinationSize(float width, float height)
        {
            destWidth = width;
            destHeight = height;
        }

        /// <summary>
        /// This method batches drawing of an array of tile instances using
        /// the Sprite interface.
        /// </summary>
        public void DrawTiles(TileInstance[] instances, Sprite sprite)
        {
            //Apply scaling based on global scale values (set in SetTileDestinationSize())
            //Notice that we are modifying the scale to correct to the next pixel.  D3D raster rules state that the 
            //midpoint of a pixel must be covered to draw to the upper left pixel.  To gaurauntee that 
            //no seams occur, this call will modify the destination size by adding a pixel width and height per scale multiplier (rounded 
            //to the nearest pixel)
            sprite.Transform = Matrix.Scaling( (float) (destWidth +(Math.Round(destWidth / tileWidth))) / tileWidth, (float) (destHeight +(Math.Round(destHeight / tileHeight))) / tileHeight, 1f);
            
            //Since the order of operations is to transform the position first, then apply the transform set in
            //sprite.Transform, the inverse of the matrix is needed to properly pre-position the sprite before the
            //entire batch is scaled approriately
            Matrix tInverse = Matrix.Invert(sprite.Transform);

            for(int i=0; i < instances.Length; i++)
            {
                //we lock the postions to integers for ease of transformation to screen coords
                Vector3 position = new Vector3((int) instances[i].X -.5f, (int) instances[i].Y  -.5f, (int) instances[i].Z);
                //setting the center to 0,0,0 makes the upper left corner of the sprite the "origin" of the sprite for transformation
                //since we aren't rotating, this is acceptable
                Vector3 adjustedPosition = Vector3.TransformCoordinate(position, tInverse);
                
                sprite.Draw(tileTex, tiles[instances[i].TileIndex], Vector3.Empty, adjustedPosition  , instances[i].Color);
            }
        }

    }
    
    /// <summary>
    /// Example of a 32-bit ARGB 'canvas' for drawing.
    /// </summary>
    public class Canvas : IDisposable
    {
        Texture tex;
        SurfaceDescription desc;
        int size = 0;
        
        /// <summary>
        /// Create a new "canvas", which is simply a wrapper for
        /// a managed pool texture to be used for directly drawing
        /// to pixels.
        /// </summary>
        /// <param name="device">The current D3D Device</param>
        /// <param name="targetWidth">The width (in pixels) of the canvas</param>
        /// <param name="targetHeight">The height (in pixels) of the canvas</param>
        public Canvas(Device device, int targetWidth, int targetHeight)
        {
            //Ensure the texture is compatible with the current D3D device
            if(!Manager.CheckDeviceFormat(device.CreationParameters.AdapterOrdinal, device.CreationParameters.DeviceType,
                device.PresentationParameters.BackBufferFormat, Usage.None, ResourceType.Textures, Format.A8R8G8B8))
            {
                throw new ApplicationException("Canvas format is unsupported.");
            }

            //Create a managed pool texture with a single level 
            //(sprites do not support MIP chains, so there is no reason to use the resources)
            tex = new Texture(device, targetWidth, targetHeight, 1, Usage.None, Format.A8R8G8B8, Pool.Managed); 

            //Get the description and size of the canvas.
            desc = tex.GetLevelDescription(0);
            size = desc.Height * desc.Width;
        }

        /// <summary>
        /// Locks the entire canvas for direct access.
        /// </summary>
        /// <param name="readOnly">Specifies that the lock is read-only.</param>
        /// <returns>An array containing the 32-bit pixel values of the canvas.</returns>
        public int[] Lock(bool readOnly)
        { 
            LockFlags flags = LockFlags.None;
            if(readOnly) flags|= LockFlags.ReadOnly;
            return tex.LockRectangle(typeof(int),0,  flags, size) as int[];
        }

        /// <summary>
        /// Locks the specified region for direct access.
        /// </summary>
        /// <param name="rect">The area to lock</param>
        /// <param name="readOnly">Specifies that the lock is read-only</param>
        /// <returns>An array containing the 32-bit pixel values of the canvas</returns>
        public GraphicsStream LockToGraphicsStream(bool readOnly)
        {
            LockFlags flags = LockFlags.None;
            if(readOnly) flags|= LockFlags.ReadOnly;
            return tex.LockRectangle(0, flags);
        }

        /// <summary>
        /// Unlocks the surface and updates the surface if specified.
        /// </summary>
        public void UnLock()
        {
            tex.UnlockRectangle(0);
        }

        /// <summary>
        /// Draws the canvas to the screen.
        /// </summary>
        /// <param name="sprite">The sprite interface used for drawing</param>
        /// <param name="sourceRect">The source rectangle to draw (using Rectangle.Empty will draw the entire source area)</param>
        /// <param name="destRect">The destination (in screen coords) to which the canvas will be drawn</param>
        /// <param name="color">A color to modulate the canvas.</param>
        public void Draw(Sprite sprite, Rectangle sourceRect, Rectangle destRect, Color color)
        {
            if(sourceRect == Rectangle.Empty) sourceRect = new Rectangle(0,0,desc.Width, desc.Height);
            
            Matrix transform = Matrix.Scaling((float)destRect.Width / (float)sourceRect.Width, (float)destRect.Height / (float)sourceRect.Height, 1f);

            sprite.Transform = transform;
            Vector3 pos = new Vector3((float) destRect.X,(float) destRect.Y, 0f);
            sprite.Draw(tex, sourceRect, pos, Vector3.Empty, color);
            
        }
        /// <summary>
        /// Returns the width of the canvas
        /// </summary>
        public int Width
        {
            get 
            {
                return desc.Width;
            }
        }
        /// <summary>
        /// Returns the height of the canvas
        /// </summary>
        public int Height
        {
            get 
            {
                return desc.Height;
            }
        }
        /// <summary>
        /// Returns the size (in pixels) of the canvas
        /// </summary>
        public int Size
        {
            get 
            {
                return size;
            }
        }

        #region IDisposable Members

        public void Dispose()
        {
            tex.Dispose();
        }

        #endregion
    }

    /// <summary>
    /// This class represents a sprite that moves randomly around a predefined space in screen coordinates
    /// </summary>
    public class MovingSprite
    {
        private float direction;
        /// <summary>
        /// In this example, position is calculated from the center of the sprite
        /// </summary>
        private Vector2 position;
        private float screenWidth, screenHeight;
        private float spriteWidth, spriteHeight;
        private Rectangle srcRect;
        private float speed;
        private Random rand;
        private Texture tex;
        private const float PI = (float) Math.PI;

        private float timeSinceTurn;
        private const float timeToTurn = 1f;
        private float aliveTime;


        /// <summary>
        /// This class represents a sprite that moves randomly around a predefined space in screen coordinates
        /// </summary>
        /// <param name="spriteTexture">The texture used for the sprite</param>
        /// <param name="sourceRect">The source rectangle of the sprite to be drawn</param>
        /// <param name="random">An instance of the Random class</param>
        /// <param name="movementSpeed">The speed at which the sprite moves.</param>
        public MovingSprite(Texture spriteTexture, Rectangle sourceRect, Random random, float movementSpeed)
        {
            rand = random;
            speed = movementSpeed;
            srcRect = sourceRect;
            tex = spriteTexture;
            spriteWidth = sourceRect.Width;
            spriteHeight = sourceRect.Height;
        }

        /// <summary>
        /// Sets the size of the screen in which the moving sprite can animate
        /// </summary>
        public void SetScreenSize(int width, int height)
        {
            screenWidth = width;
            screenHeight = height;
            position.X = screenWidth / 2;
            position.Y = screenHeight / 2;
            direction = (float) rand.NextDouble() * 2f * PI;
            direction = 0;
        }

        /// <summary>
        /// This method moves, scales, and rotates the sprite's parameters during frame updates.
        /// </summary>
        /// <param name="time">Elapsed time in seconds of the last frame.</param>
        public void Move(float time)
        {

            timeSinceTurn+= time;
            aliveTime+= time;

            //check to see if it is time to turn
            if(timeSinceTurn > timeToTurn)
            {
                //randomly turn within a 90 degree arc to a new direction
                direction+= (.5f - (float) rand.NextDouble()) * PI * .5f;
                timeSinceTurn-= timeToTurn;
            }

            //calculate a new size for the output sprite.  The sprite will change scale relative to its source x/y scale.
            spriteWidth = Math.Abs((float) Math.Cos(aliveTime* 30f) * (float) srcRect.Width *.333f) + (float) srcRect.Width ;
            spriteHeight = Math.Abs((float) Math.Sin(aliveTime*2.5f) * (float) srcRect.Height *.333f) + (float)srcRect.Height ;

            //Get a delta x/y position based on the speed time time times the magnitude of the unit direction
            float dX = speed * time * (float) Math.Cos(direction);
            float dY = speed * time * (float) Math.Sin(direction);

            //detect collisions (using the center point) with any of the screen adges and move back if the
            //sprite is out of bounds.
            if((dX + position.X) > screenWidth ) { direction = PI;  dX = speed * time * (float) Math.Cos(direction);}
            if((dY + position.Y) > screenHeight ) { direction = 3f * PI / 2f;  dY = speed * time * (float) Math.Sin(direction);}
            if((dY + position.X) < 0 ) { direction = 0; dX = speed * time * (float) Math.Cos(direction);}
            if((dY + position.Y) < 0 ) { direction =  PI / 2f; dY = speed * time * (float) Math.Sin(direction); }

            position.X+= dX;
            position.Y+= dY;

            if(direction > (Math.PI * 2)) direction-= 2* PI;
        }

        /// <summary>
        /// This method uses Draw2D to easily draw the sprite to screen space.
        /// </summary>
        public void Draw(Sprite sprite)
        {
            sprite.Draw2D(tex, srcRect, 
                //The destination size
                new SizeF(spriteWidth, spriteHeight), 
                //The center point (For Draw2D, this value is relative to the source image, not the scaled end image)
                new PointF(srcRect.Width / 2f, srcRect.Height / 2f),  
                //The angle of rotation is simply the direction of the sprite
                direction, 
                //the position is calculated from the center of the sprite
                new PointF(position.X, position.Y),
                Color.White);
        }

        /// <summary>
        /// This draw call example is not used in the sample, but is provided to show
        /// how to emulate the Draw2D call by setting the transformation property.
        /// before the Draw() call.
        /// </summary>
        public void DrawAlternate(Sprite sprite)
        {
            Matrix transformation = 
                //First, translate to the center of the original (untransformed) sprite
                Matrix.Translation(-srcRect.Width / 2f, -srcRect.Height / 2f, 0f)
                //scale the sprite
                * Matrix.Scaling(spriteWidth / srcRect.Width, spriteHeight / srcRect.Height, 1)
                //rotate about the z axis (into the screen) of the centered sprite
                * Matrix.RotationZ(direction)
                //translate to the final position
                *  Matrix.Translation(position.X, position.Y, 0);

            //set the transformation
            sprite.Transform = transformation;
            //Draw with position 0, center 0, and color white (all needed transformation has been completed at this point)
            sprite.Draw(tex, srcRect,  Vector3.Empty, Vector3.Empty, Color.Red);
        }
    }

    /// <summary>
    /// This class represents an example of a multi-frame animation displayed using Sprite.
    /// This class assumes that the art used is specifically formated for use with this class.
    /// The art must be a texture with uniformly sized cells (frames) of animation organized in rows
    /// and columens, in sequential order from left-to-right, top-to-bottom.
    /// </summary>
    public class AnimatedSprite
    {
        Texture tex;
        int currentFrame;
        int numFrames;
        int width, height;
        int startCell;
        int numRows, numColumns;


        /// <summary>
        /// Create an instance of an animated Sprite
        /// </summary>
        /// <param name="t">The texture containing the complete set of frames for the animaion</param>
        /// <param name="cellWidth">The width of a single cell of animation</param>
        /// <param name="cellHeight">The height of a single cell of animation</param>
        /// <param name="rows">Number of rows on the texture that contain animation data</param>
        /// <param name="columns">Number of columns on the texture that contain animation data</param>
        /// <param name="numberOfFrames">Total number of frames on the texture that contain animation data (should be less than or equal to rows * columns)</param>
        /// <param name="startingCell">The first cell with animaion data (counting in left-to-right then top-to-bottom order)</param>
        public AnimatedSprite(Texture t, int cellWidth, int cellHeight, int rows, int columns, int numberOfFrames, int startingCell)
        {
            currentFrame = 0;
            numFrames = numberOfFrames;
            tex = t;
            width = cellWidth;
            height = cellHeight;
            startCell = startingCell;
            numRows = rows ;
            numColumns = columns;
        }

        /// <summary>
        /// Gets or sets the current frame of animation
        /// </summary>
        public int Frame
        {
            get { return currentFrame; }
            set 
            { 
                if(value > numFrames) throw new IndexOutOfRangeException("Specified frame was outside the range of frames for this animation.");
                currentFrame = value; 
            }

        }

        /// <summary>
        /// Advances the frame by one.  Automatically wraps to the beginning of the animation.
        /// </summary>
        public void AdvanceFrame()
        {
            currentFrame++;
            if(currentFrame >= numFrames) currentFrame = 0;
        }

        /// <summary>
        /// Draws the sprite using the current frame of animation for a source image.
        /// </summary>
        /// <param name="sprite"></param>
        /// <param name="upperLeftPosition"></param>
        /// <param name="color"></param>
        public void Draw(Sprite sprite, Vector3 upperLeftPosition, Color color)
        {
            //The current row and column are calculated from the current frame information
            int currentRow = (currentFrame +  startCell) / numColumns;
            int currentColumn = (currentFrame + startCell) % numColumns;
            sprite.Transform = Matrix.Identity;

            //The source rectange is calculated from the current row/column and the size of the animation cells
            Rectangle srcRect = new Rectangle(currentColumn * width, currentRow * height, width, height);

            //The sprite is drawn to the screen
            sprite.Draw(tex, srcRect, Vector3.Empty, upperLeftPosition, color);
        }
    }

    /// <summary>
    /// An example usage of Canvas to create a 2D particle effect.
    /// </summary>
    public class ParticleEffect
    {
        //particle information
        private int[] x;
        private int[] y;
        private double[] direction;
        private double[] angularVelocity;
        private int[] color;
        private int numParticles;
        private double speed;

        //canvas-specific information
        private int maxX, maxY;
        private int particleWidth, particleHeight;
        private Canvas canvas;
        /// <summary>
        /// The canvas width as a private variable as subsequent calls to
        /// width would call through the property (it is NOT cached in debug mode).
        /// For example, on a 640x480 canvas, we'd have to call send message
        /// as many as 2 million times per frame, destroying performance.
        /// </summary>
        private int width;
        
        //randomization varaibles
        private Random rand;
        private byte[] randData;

        /// <summary>
        /// Create a new instance of ParticleEffect.
        /// </summary>
        public ParticleEffect(int sizeX, int sizeY, Canvas particleCanvas, Random random, double particleSpeed, int numberOfParticles)
        {
            canvas = particleCanvas;
            width= canvas.Width;
            maxX = width - sizeX;
            maxY = canvas.Height - sizeY;
            particleWidth = sizeX;
            particleHeight = sizeY;
            rand = random;
            speed = particleSpeed;
            numParticles = numberOfParticles;
            x = new int[numParticles];
            y = new int[numParticles];
            direction = new double[numParticles];
            angularVelocity = new double[numParticles];
            color = new int[numParticles];
            randData = new byte[canvas.Width / 4];
            rand.NextBytes(randData);
            
            
            //Assign colors and starting parameters to particles
            for(int i=0; i<numParticles; i++)
            {
                x[i] = rand.Next(maxX);
                y[i] = rand.Next(maxY);
                
                switch(i % 5)
                {
                    case 0:
                        color[i] = Color.Purple.ToArgb();
                        break;
                    case 1:
                        color[i] = Color.LightGreen.ToArgb();
                        break;
                    case 2:
                        color[i] = Color.Pink.ToArgb();
                        break;
                    case 3:
                        color[i] = Color.CornflowerBlue.ToArgb();
                        break;
                    case 4:
                        color[i] = Color.GhostWhite.ToArgb();
                        break;
                }
                angularVelocity[i] = 0;
                direction[i] = (rand.NextDouble() * Math.PI * 2.0) - Math.PI;
            }
            
        }

        /// <summary>
        /// This function moves the "Particle" instances around the 2D canvas.
        /// </summary>
        /// <param name="i"></param>
        private void MoveParticle(int i)
        {
            
            int dX = (int) (speed * Math.Cos(direction[i]));
            int dY = (int) (speed * Math.Sin(direction[i]));

            angularVelocity[i] += (rand.NextDouble() - .5) * .01;
            if(angularVelocity[i] > .1) angularVelocity[i] = .1;
            if(angularVelocity[i] < -.1) angularVelocity[i] = -.1;
            direction[i] = angularVelocity[i] + direction[i];

            if((dX + x[i]) > maxX ) { dX = -dX;  direction[i] = Math.PI; }
            if((dY + y[i]) > maxY ) { dY = -dY;  direction[i] = 3f * Math.PI / 2f; }
            if((dX + x[i]) < 0 ) { dX = -dX;  direction[i] = 0; }
            if((dY + y[i]) < 0 ) { dY = -dY;  direction[i] =  Math.PI / 2f; }

            x[i]+= dX;
            y[i]+= dY;

            if(direction[i] > (Math.PI * 2)) direction[i]-= 2* Math.PI;
        }

        /// <summary>
        /// This function internally updates the positions of the particles
        /// and updates the pixels on the canvas.
        /// </summary>
        public void Update()
        {
            

            //Lock the texture data to an int array.  This is a slow operation
            //that requires allocation of a new managed array and a full memory copy.
            //It's reccomended only for low-frequency methods.
            int[] data = canvas.Lock(false);

            //This section updates the canvas with a smeary, gravity effect.
            for(int i=0; i<(width); i++)
            {
                data[i]=0;
            }
            int startingPoint = rand.Next(width);
            int count = startingPoint;
            for(int i=canvas.Size-1; i > width; i--)
            {
                    
                //To avoid lots of calls to Random.Next(), random data has been into
                //an array and is used for a random bitmap of boolian data.
                int comp = randData[count / 4] & (0x01 << count % 8);
                if(comp != 0) data[i] = DecrementColor(data[i]);
                else data[i] = DecrementColor(data[i-(width)]);
                count++;
                if(count >= width) count = startingPoint;
            }


            //This section updates each "Particle" and draws them
            //as a random rectangular pattern.
            for(int particle=0; particle < numParticles; particle++)
            {
                MoveParticle(particle);
                for(int pY = y[particle]; pY < (y[particle]+particleHeight); pY++)
                {
                    for(int pX = x[particle]; pX < (x[particle]+particleWidth); pX++)
                    {
                        int i = pX + pY * width;
                        int comp = randData[count / 4] & (0x01 << count % 8);
                        if(comp != 0) data[i] = color[particle];
                        else data[i] = 0;
                        count++;
                        if(count >= width) count = startingPoint;
                        
                    }
                }
            }

            //After a lock, a call to unlock updates indicates 
            //that the texture is ready to be used.
            canvas.UnLock();
        }


        /// <summary>
        /// This function uses an unsafe code block and uses GraphicsStream
        /// to reduce the background copies from the unmanaged data to a
        /// managed array.
        /// </summary>
        public void UpdateUnsafe()
        {

            unsafe 
            {
                //The unsafe version of this function creates a GraphicsStream
                //which is a managed representation of the actual texture memory.  This way there 
                //is no data copy and no need to allocate a large int array on the managed heap.
                GraphicsStream dataStream = canvas.LockToGraphicsStream(false);
                
                //Since we're in an unsafe context, the InternalDataPointer can be treated like
                //a void pointer from C/C++
                int *data = (int*) dataStream.InternalDataPointer;
                for(int i=0; i<(width); i++)
                {
                    data[i]=0;
                }
                int startingPoint = rand.Next(width);
                int count = startingPoint;
                for(int i=canvas.Size-1; i > (width) ; i--)
                {
                    int comp = randData[count / 4] & (0x01 << count % 8);
                    if(comp != 0) data[i] = DecrementColor(data[i]);
                    else data[i] = DecrementColor(data[i-(width)]);
                    count++;
                    if(count >= width) count = startingPoint;
                }
                for(int Particle=0; Particle<numParticles; Particle++)
                {
                    MoveParticle(Particle);
                    for(int pY = y[Particle]; pY < (y[Particle]+particleHeight); pY++)
                    {
                        for(int pX = x[Particle]; pX < (x[Particle]+particleWidth); pX++)
                        {
                            int i = pX + pY * width;
                            int comp = randData[count / 4] & (0x01 << count % 8);
                            if(comp != 0) data[i] = color[Particle];
                            else data[i] = 0;
                            count++;
                            if(count >= width) count = startingPoint;
                        
                        }
                    }
                }
                canvas.UnLock();
            }

        }

        /// <summary>
        /// This algorithm gives the colors in the final image a "fade" effect.
        /// </summary>
        /// <param name="color"></param>
        /// <returns></returns>
        private  int DecrementColor(int color)
        {
            unchecked 
            {
                if((color & 0xFF000000) == 0) return 0;
                int alpha = (int) (color & 0xFF000000);
                int red = color & 0x00FF0000;
                int green = color & 0x0000FF00;
                int blue = color & 0x000000FF;

                if((uint) alpha > (uint) 0x06000000) alpha -= 0x06000000;
                else alpha = 0;
                if(red < 0x00FA0000) red += 0x00050000;
                if(green < 0x0000FA00) green += 0x00000500;
                if(blue < 0x000000DF) blue += 0x00000010;
                
                return (int) (alpha | red | green | blue);
            }
        }
    }


   

    
}