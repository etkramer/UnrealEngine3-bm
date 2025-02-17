//--------------------------------------------------------------------------------------
// File: Script3.cs
//
// Scripting code for Managed Scripting sample
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
using System;

/// <summary>
/// Handles scripts for the ShadowVolume sample
/// </summary>
class ScriptClass
{
    // Constants 
    const float MaxMovementSpeed = 3.5f; // Maximum movement speed
    const float RotationSpeed = 0.01f; // Constant rotation speed
    const float MaxX = 2.0f;
    const float MaxY = 2.0f;
    const float MaxZ = 2.0f;
    const float MinX = -2.0f;
    const float MinY = 0.0f;
    const float MinZ = -2.0f;

    // Variables
    static float MovementSpeedX = 0.85f;
    static float MovementSpeedY = 0.85f;
    static float MovementSpeedZ = 0.85f;
    static double LastUpdateTime;
    static int XDirection = 1;
    static int YDirection = 1;
    static int ZDirection = 1;
    static float storedX = MinX;
    static float storedY = MinY;
    static float storedZ = MinZ;
    
    // Rotation variables
    static bool isRotationX = false;
    static bool isRotationY = false;
    static bool isRotationZ = false;
    static float rotationXSpeed = 0.0f;
    static float rotationYSpeed = 0.0f;
    static float rotationZSpeed = 0.0f;

    // Rotation variables
    static float rotationX = 0.0f;
    static float rotationY = 0.0f;
    static float rotationZ = 0.0f;

    static Random rndGenerator = new Random();
    /// <summary>
    /// Update the position of the player
    /// </summary>
    public static void UpdatePosition(double appTime, ref float x, ref float y, ref float z)
    {
        // Make sure some time has elapsed
        float elapsedTime = (float)(appTime - LastUpdateTime);
        LastUpdateTime = appTime;
        if (elapsedTime <= 0.0f)
            return; // Nothing to do
        
        // Increase the X param some
        storedX += (MovementSpeedX * elapsedTime) * XDirection;
        // Now Y
        storedY += (MovementSpeedY * elapsedTime) * YDirection;
        // Finally Z
        storedZ += (MovementSpeedZ * elapsedTime) * ZDirection;

        // Do Param checks
        if (storedX >= MaxX)
        {
            MovementSpeedX = MaxMovementSpeed * (float)rndGenerator.NextDouble();
            storedX = MaxX;
            XDirection *= -1;
            isRotationY = true;
            rotationYSpeed += (RotationSpeed * XDirection);
        }
        else if (storedX <= MinX)
        {
            MovementSpeedX = MaxMovementSpeed * (float)rndGenerator.NextDouble();
            storedX = MinX;
            XDirection *= -1;
            isRotationY = true;
            rotationYSpeed += (RotationSpeed * XDirection);
        }
        if (storedY >= MaxY)
        {
            MovementSpeedY = MaxMovementSpeed * (float)rndGenerator.NextDouble();
            storedY = MaxY;
            YDirection *= -1;
            isRotationZ = true;
            rotationZSpeed += (RotationSpeed * YDirection);
        }
        else if (storedY <= MinY)
        {
            MovementSpeedY = MaxMovementSpeed * (float)rndGenerator.NextDouble();
            storedY = MinY;
            YDirection *= -1;
            isRotationZ = true;
            rotationZSpeed += (RotationSpeed * YDirection);
        }
        if (storedZ >= MaxZ)
        {
            MovementSpeedZ = MaxMovementSpeed * (float)rndGenerator.NextDouble();
            storedZ = MaxZ;
            ZDirection *= -1;
            isRotationX = true;
            rotationXSpeed += (RotationSpeed * ZDirection);
        }
        else if (storedZ <= MinZ)
        {
            MovementSpeedZ = MaxMovementSpeed * (float)rndGenerator.NextDouble();
            storedZ = MinZ;
            ZDirection *= -1;
            isRotationX = true;
            rotationXSpeed += (RotationSpeed * ZDirection);
        }

        // Save the variables now
        x = storedX;
        y = storedY;
        z = storedZ;

        // Update rotation if need be
        if (isRotationX)
            rotationX += rotationXSpeed;
        if (isRotationY)
            rotationY += rotationYSpeed;
        if (isRotationZ)
            rotationZ += rotationZSpeed;

    }

    /// <summary>
    /// Update the players rotation on the X axis
    /// </summary>
    public static void UpdateRotationX(double appTime, ref float a)
    {
        a = rotationX;
    }

    /// <summary>
    /// Update the players rotation on the Y axis
    /// </summary>
    public static void UpdateRotationY(double appTime, ref float a)
    {
        a = rotationY;
    }

    /// <summary>
    /// Update the players rotation on the Z axis
    /// </summary>
    public static void UpdateRotationZ(double appTime, ref float a)
    {
        a = rotationZ;
    }
}
