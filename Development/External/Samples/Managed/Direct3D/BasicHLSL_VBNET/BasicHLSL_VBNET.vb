'-----------------------------------------------------------------------------
' File: BasicHLSL_VBNET.vb
'
' A simple Visual Basic.NET HLSL sample
'
' Copyright (c) Microsoft Corporation. All rights reserved.
'-----------------------------------------------------------------------------

Imports System
Imports System.Runtime.InteropServices
Imports Microsoft.DirectX
Imports Microsoft.DirectX.Direct3D
Imports Microsoft.Samples.DirectX.UtilityToolkit


Public Class BasicHLSL
    Implements IDeviceCreation, IFrameworkCallback

#Region "Creation"

    '/ <summary>Create a new instance of the class</summary>
    Public Sub New(ByVal f As Framework)
        ' Store framework
        sampleFramework = f
        ' Create dialogs
        hud = New Dialog(sampleFramework)
        sampleUi = New Dialog(sampleFramework)
    End Sub 'New
#End Region

    ' constants
    Private Shared WhiteColor As New ColorValue(1.0F, 1.0F, 1.0F, 1.0F)
    Private Shared YellowColor As New ColorValue(1.0F, 1.0F, 0.0F, 1.0F)

    ' Variables
    Private sampleFramework As Framework = Nothing ' Framework for samples
    Private statsFont As Font = Nothing ' Font for drawing text
    Private textSprite As Sprite = Nothing ' Sprite for batching text calls
    Private effect As effect = Nothing ' D3DX Effect Interface
    Private camera As New ModelViewerCamera   ' A model viewing camera
    Private mesh As mesh = Nothing ' The mesh object
    Private meshTexture As Texture = Nothing ' The meshes texture
    Private isHelpShowing As Boolean = True ' If true, renders the UI help text
    Private isUsingPreshader As Boolean ' If false, the NoPreshader flag is used when compiling the shader
    Private hud As Dialog ' dialog for standard controls
    Private sampleUi As Dialog ' dialog for sample specific controls
    Private worldFix As Matrix

    ' Light stuff
    Private lightControl(MaxNumberLights) As DirectionWidget
    Private Const MaxNumberLights As Integer = 3
    Private lightScale As Single
    Private numberActiveLights As Integer
    Private activeLight As Integer

    ' HUD Ui Control constants
    Private Const ToggleFullscreen As Integer = 1
    Private Const ToggleReference As Integer = 3
    Private Const ChangeDevice As Integer = 4
    Private Const EnablePreshader As Integer = 5
    Private Const NumberLights As Integer = 6
    Private Const NumberLightsStatic As Integer = 7
    Private Const ActiveLightControl As Integer = 8
    Private Const LightScaleControl As Integer = 9
    Private Const LightScaleStatic As Integer = 10


    '/ <summary>
    '/ Called during device initialization, this code checks the device for some 
    '/ minimum set of capabilities, and rejects those that don't pass by returning false.
    '/ </summary>
    Public Function IsDeviceAcceptable(ByVal caps As Caps, ByVal adapterFormat As Format, ByVal backBufferFormat As Format, ByVal windowed As Boolean) As Boolean Implements IDeviceCreation.IsDeviceAcceptable
        ' No fallback defined by this app, so reject any device that 
        ' doesn't support at least ps1.1

        If System.Version.op_LessThan(caps.PixelShaderVersion, New Version(1, 1)) Then
            Return False
        End If
        ' Skip back buffer formats that don't support alpha blending
        If Not Manager.CheckDeviceFormat(caps.AdapterOrdinal, caps.DeviceType, adapterFormat, Usage.QueryPostPixelShaderBlending, ResourceType.Textures, backBufferFormat) Then
            Return False
        End If
        Return True
    End Function 'IsDeviceAcceptable


    '/ <summary>
    '/ This callback function is called immediately before a device is created to allow the 
    '/ application to modify the device settings. The supplied settings parameter 
    '/ contains the settings that the framework has selected for the new device, and the 
    '/ application can make any desired changes directly to this structure.  Note however that 
    '/ the sample framework will not correct invalid device settings so care must be taken 
    '/ to return valid device settings, otherwise creating the Device will fail.  
    '/ </summary>
    Public Sub ModifyDeviceSettings(ByVal settings As DeviceSettings, ByVal caps As Caps) Implements IDeviceCreation.ModifyDeviceSettings
        ' If device doesn't support HW T&L or doesn't support 1.1 vertex shaders in HW 
        ' then switch to SWVP.
        If Not caps.DeviceCaps.SupportsHardwareTransformAndLight OrElse System.Version.op_LessThan(caps.VertexShaderVersion, New Version(1, 1)) Then
            settings.BehaviorFlags = CreateFlags.SoftwareVertexProcessing
        Else
            settings.BehaviorFlags = CreateFlags.HardwareVertexProcessing
        End If

        ' This application is designed to work on a pure device by not using 
        ' any get methods, so create a pure device if supported and using HWVP.
        If caps.DeviceCaps.SupportsPureDevice AndAlso (settings.BehaviorFlags And CreateFlags.HardwareVertexProcessing) <> 0 Then
            settings.BehaviorFlags = settings.BehaviorFlags Or CreateFlags.PureDevice
        End If

        If settings.DeviceType = DeviceType.Reference Then
            Utility.DisplaySwitchingToRefWarning(sampleFramework, "BasicHLSL")
        End If
    End Sub 'ModifyDeviceSettings

    '/ <summary>
    '/ This event will be fired immediately after the Direct3D device has been 
    '/ created, which will happen during application initialization and windowed/full screen 
    '/ toggles. This is the best location to create Pool.Managed resources since these 
    '/ resources need to be reloaded whenever the device is destroyed. Resources created  
    '/ here should be released in the Disposing event. 
    '/ </summary>
    Private Sub OnCreateDevice(ByVal sender As Object, ByVal e As DeviceEventArgs)
        ' Initialize the stats font
        statsFont = ResourceCache.GetGlobalInstance().CreateFont(e.Device, 15, 0, FontWeight.Bold, 1, False, CharacterSet.Default, Precision.Default, FontQuality.Default, PitchAndFamily.FamilyDoNotCare Or PitchAndFamily.DefaultPitch, "Arial")

        ' Load the mesh
        mesh = LoadMesh(e.Device, "tiny\tiny.x")

        ' Calculate a bounding sphere
        Dim radius As Single = 0.0F
        Dim data As GraphicsStream = mesh.LockVertexBuffer(LockFlags.None)
        Try
            Dim center As Vector3
            radius = Geometry.ComputeBoundingSphere(data, mesh.NumberVertices, mesh.VertexFormat, center)

            worldFix = Matrix.Translation(Vector3.Multiply(center, -1))
            worldFix.Multiply(Matrix.RotationY(System.Convert.ToSingle(Math.PI)))
            worldFix.Multiply(Matrix.RotationX(System.Convert.ToSingle(Math.PI) / 2.0F))

            ' Setup direction widget
            DirectionWidget.OnCreateDevice(e.Device)
            Dim i As Integer
            For i = 0 To MaxNumberLights - 1
                lightControl(i).Radius = radius
            Next i
            ' Finally unlock the vertex buffer
            mesh.UnlockVertexBuffer()
        Finally
            data.Dispose()
        End Try

        Dim shaderFlags As ShaderFlags = shaderFlags.NotCloneable
        ' Preshaders are parts of the shader that the effect system pulls out of the 
        ' shader and runs on the host CPU. They should be used if you are GPU limited. 
        ' The ShaderFlags.NoPreShader flag disables preshaders.
        If Not isUsingPreshader Then
            shaderFlags = shaderFlags Or shaderFlags.NoPreShader
        End If
        ' Read the D3DX effect file
        Dim path As String = Utility.FindMediaFile("BasicHLSL_VBNET.fx")
        effect = ResourceCache.GetGlobalInstance().CreateEffectFromFile(e.Device, path, Nothing, Nothing, shaderFlags, Nothing)

        ' Create the mesh texture from a file
        path = Utility.FindMediaFile("tiny\tiny_skin.dds")
        meshTexture = ResourceCache.GetGlobalInstance().CreateTextureFromFile(e.Device, path)

        ' Set effect variables as needed
        effect.SetValue(EffectHandle.FromString("g_MaterialAmbientColor"), New ColorValue(0.35F, 0.35F, 0.35F, 0))
        effect.SetValue(EffectHandle.FromString("g_MaterialDiffuseColor"), WhiteColor)
        effect.SetValue(EffectHandle.FromString("g_MeshTexture"), meshTexture)

        ' Setup the camera's view parameters
        camera.SetViewParameters(New Vector3(0.0F, 0.0F, -15.0F), Vector3.Empty)
        camera.SetRadius(radius * 3.0F, radius * 0.5F, radius * 10.0F)
    End Sub 'OnCreateDevice


    '/ <summary>
    '/ This event will be fired immediately after the Direct3D device has been 
    '/ reset, which will happen after a lost device scenario. This is the best location to 
    '/ create Pool.Default resources since these resources need to be reloaded whenever 
    '/ the device is lost. Resources created here should be released in the OnLostDevice 
    '/ event. 
    '/ </summary>
    Private Sub OnResetDevice(ByVal sender As Object, ByVal e As DeviceEventArgs)
        Dim desc As SurfaceDescription = e.BackBufferDescription
        ' Create a sprite to help batch calls when drawing many lines of text
        textSprite = New Sprite(e.Device)

        ' Reset items
        Dim i As Integer
        For i = 0 To MaxNumberLights - 1
            lightControl(i).OnResetDevice(desc)
        Next i
        ' Setup the camera's projection parameters
        Dim aspectRatio As Single = System.Convert.ToSingle(desc.Width) / System.Convert.ToSingle(desc.Height)
        camera.SetProjectionParameters(System.Convert.ToSingle(Math.PI) / 4, aspectRatio, 100.0F, 5000.0F)
        camera.SetWindow(desc.Width, desc.Height)
        camera.SetButtonMasks(MouseButtonMask.Left, MouseButtonMask.Wheel, MouseButtonMask.Middle)

        ' Setup UI locations
        hud.SetLocation(desc.Width - 170, 0)
        hud.SetSize(170, 170)
        sampleUi.SetLocation(desc.Width - 170, desc.Height - 300)
        sampleUi.SetSize(170, 300)
    End Sub 'OnResetDevice


    '/ <summary>
    '/ This event function will be called fired after the Direct3D device has 
    '/ entered a lost state and before Device.Reset() is called. Resources created
    '/ in the OnResetDevice callback should be released here, which generally includes all 
    '/ Pool.Default resources. See the "Lost Devices" section of the documentation for 
    '/ information about lost devices.
    '/ </summary>
    Private Sub OnLostDevice(ByVal sender As Object, ByVal e As EventArgs)
        If Not (textSprite Is Nothing) Then
            textSprite.Dispose()
            textSprite = Nothing
        End If

        ' Update the direction widget
        DirectionWidget.OnLostDevice()
    End Sub 'OnLostDevice


    '/ <summary>
    '/ This event will be fired immediately after the Direct3D device has 
    '/ been destroyed, which generally happens as a result of application termination or 
    '/ windowed/full screen toggles. Resources created in the OnCreateDevice event 
    '/ should be released here, which generally includes all Pool.Managed resources. 
    '/ </summary>
    Private Sub OnDestroyDevice(ByVal sender As Object, ByVal e As EventArgs)
        ' Update the direction widget
        DirectionWidget.OnDestroyDevice()
        If Not (mesh Is Nothing) Then
            mesh.Dispose()
        End If
    End Sub 'OnDestroyDevice

    '/ <summary>
    '/ This callback function will be called once at the beginning of every frame. This is the
    '/ best location for your application to handle updates to the scene, but is not 
    '/ intended to contain actual rendering calls, which should instead be placed in the 
    '/ OnFrameRender callback.  
    '/ </summary>
    Public Sub OnFrameMove(ByVal device As Device, ByVal appTime As Double, ByVal elapsedTime As Single) Implements IFrameworkCallback.OnFrameMove
        ' Update the camera's position based on user input 
        camera.FrameMove(elapsedTime)
    End Sub 'OnFrameMove


    '/ <summary>
    '/ This callback function will be called at the end of every frame to perform all the 
    '/ rendering calls for the scene, and it will also be called if the window needs to be 
    '/ repainted. After this function has returned, the sample framework will call 
    '/ Device.Present to display the contents of the next buffer in the swap chain
    '/ </summary>
    Public Sub OnFrameRender(ByVal device As Device, ByVal appTime As Double, ByVal elapsedTime As Single) Implements IFrameworkCallback.OnFrameRender
        Dim beginSceneCalled As Boolean = False

        ' Clear the render target and the zbuffer 
        device.Clear(ClearFlags.ZBuffer Or ClearFlags.Target, &H8C003F3F, 1.0F, 0)
        Try
            device.BeginScene()
            beginSceneCalled = True

            Dim lightDir(MaxNumberLights - 1) As Vector3
            Dim lightDiffuse(MaxNumberLights - 1) As ColorValue

            ' Render the arrows so the user can visually see the light direction
            Dim i As Integer
            For i = 0 To numberActiveLights - 1
                Dim arrowColor As ColorValue
                If (i = activeLight) Then
                    arrowColor = YellowColor
                Else
                    arrowColor = WhiteColor
                End If
                lightControl(i).OnRender(arrowColor, camera.ViewMatrix, camera.ProjectionMatrix, camera.EyeLocation)
                ' Get the light direction and color
                lightDir(i) = lightControl(i).LightDirection
                lightDiffuse(i) = ColorOperator.Scale(WhiteColor, lightScale)
            Next i

            Dim worldMatrix As Matrix = Matrix.Multiply(worldFix, camera.WorldMatrix)
            ' Update the effects now
            Dim lightDirHandle As GCHandle = GCHandle.Alloc(lightDir, GCHandleType.Pinned)
            Dim lightDirectionData = New GraphicsStream(Marshal.UnsafeAddrOfPinnedArrayElement(lightDir, 0), Marshal.SizeOf(GetType(Vector3)) * MaxNumberLights, True, True)
            effect.SetValue(EffectHandle.FromString("g_LightDir"), lightDirectionData)
            effect.SetValue(EffectHandle.FromString("g_LightDiffuse"), lightDiffuse)
            lightDirHandle.Free()
            ' Update the effect's variables.  Instead of using strings, it would 
            ' be more efficient to cache a handle to the parameter by calling 
            ' Effect.GetParameter
            effect.SetValue(EffectHandle.FromString("worldViewProjection"), _
                Matrix.Multiply(Matrix.Multiply(worldMatrix, camera.ViewMatrix), camera.ProjectionMatrix))
            effect.SetValue(EffectHandle.FromString("worldMatrix"), worldMatrix)
            effect.SetValue(EffectHandle.FromString("appTime"), System.Convert.ToSingle(appTime))

            effect.SetValue(EffectHandle.FromString("g_MaterialDiffuseColor"), WhiteColor)
            effect.SetValue(EffectHandle.FromString("g_nNumLights"), numberActiveLights)

            ' Render the scene with this technique as defined in the .fx file
            Select Case numberActiveLights
                Case 1
                    effect.Technique = EffectHandle.FromString("RenderSceneWithTexture1Light")
                Case 2
                    effect.Technique = EffectHandle.FromString("RenderSceneWithTexture2Light")
                Case 3
                    effect.Technique = EffectHandle.FromString("RenderSceneWithTexture3Light")
            End Select

            ' Apply the technique contained in the effect 
            Dim passes As Integer = effect.Begin(0)
            Dim pass As Integer
            For pass = 0 To passes - 1
                effect.BeginPass(pass)

                ' The effect interface queues up the changes and performs them 
                ' with the CommitChanges call. You do not need to call CommitChanges if 
                ' you are not setting any parameters between the BeginPass and EndPass.
                ' effect.CommitChanges() );
                ' Render the mesh with the applied technique
                mesh.DrawSubset(0)
                effect.EndPass()
            Next pass
            effect.End()

            ' Show frame rate and help, etc
            RenderText(appTime)

            ' Show UI
            hud.OnRender(elapsedTime)
            sampleUi.OnRender(elapsedTime)
        Finally
            If beginSceneCalled Then
                device.EndScene()
            End If
        End Try
    End Sub 'OnFrameRender

    '/ <summary>
    '/ Render the help and statistics text. This function uses the Font object for 
    '/ efficient text rendering.
    '/ </summary>
    Private Sub RenderText(ByVal appTime As Double)
        Dim txtHelper As New TextHelper(statsFont, textSprite, 15)

        ' Output statistics
        txtHelper.Begin()
        txtHelper.SetInsertionPoint(2, 0)
        txtHelper.SetForegroundColor(&HFFFFFF00)
        txtHelper.DrawTextLine(sampleFramework.FrameStats)
        txtHelper.DrawTextLine(sampleFramework.DeviceStats)

        txtHelper.SetForegroundColor(&HFFFFFFFF)
        txtHelper.DrawTextLine("appTime: {0} sin(appTime): {1}", appTime.ToString("f2", System.Globalization.CultureInfo.CurrentUICulture), Math.Sin(appTime).ToString("f4", System.Globalization.CultureInfo.CurrentUICulture))

        ' Draw help
        If isHelpShowing Then
            txtHelper.SetInsertionPoint(10, sampleFramework.BackBufferSurfaceDescription.Height - 15 * 6)
            txtHelper.SetForegroundColor(System.Drawing.Color.DarkOrange)
            txtHelper.DrawTextLine("Controls (F1 to hide):")

            txtHelper.SetInsertionPoint(20, sampleFramework.BackBufferSurfaceDescription.Height - 15 * 5)
            txtHelper.DrawTextLine("Rotate model: Left mouse button")
            txtHelper.DrawTextLine("Rotate light: Right mouse button")
            txtHelper.DrawTextLine("Rotate camera: Middle mouse button")
            txtHelper.DrawTextLine("Zoom camera: Mouse wheel scroll")

            txtHelper.SetInsertionPoint(250, sampleFramework.BackBufferSurfaceDescription.Height - 15 * 5)
            txtHelper.DrawTextLine("Hide help: F1")
            txtHelper.DrawTextLine("Quit: Esc")
        Else
            txtHelper.SetForegroundColor(&HFFFFFFFF)
            txtHelper.DrawTextLine("Press F1 for help")
        End If

        txtHelper.End()
    End Sub 'RenderText


    '/ <summary>
    '/ As a convenience, the sample framework inspects the incoming windows messages for
    '/ keystroke messages and decodes the message parameters to pass relevant keyboard
    '/ messages to the application.  The framework does not remove the underlying keystroke 
    '/ messages, which are still passed to the application's MsgProc callback.
    '/ </summary>
    Private Sub OnKeyEvent(ByVal sender As Object, ByVal e As System.Windows.Forms.KeyEventArgs)
        Select Case e.KeyCode
            Case System.Windows.Forms.Keys.F1
                isHelpShowing = Not isHelpShowing
        End Select
    End Sub 'OnKeyEvent


    '/ <summary>
    '/ Before handling window messages, the sample framework passes incoming windows 
    '/ messages to the application through this callback function. If the application sets 
    '/ noFurtherProcessing to true, the sample framework will not process the message
    '/ </summary>
    Public Function OnMsgProc(ByVal hWnd As IntPtr, ByVal msg As NativeMethods.WindowMessage, ByVal wParam As IntPtr, ByVal lParam As IntPtr, ByRef noFurtherProcessing As Boolean) As IntPtr
        ' Give the dialog a chance to handle the message first
        noFurtherProcessing = hud.MessageProc(hWnd, msg, wParam, lParam)
        If noFurtherProcessing Then
            Return IntPtr.Zero
        End If
        noFurtherProcessing = sampleUi.MessageProc(hWnd, msg, wParam, lParam)
        If noFurtherProcessing Then
            Return IntPtr.Zero
        End If
        ' Give the light control a chance now
        lightControl(activeLight).HandleMessages(hWnd, msg, wParam, lParam)

        ' Pass all remaining windows messages to camera so it can respond to user input
        camera.HandleMessages(hWnd, msg, wParam, lParam)

        Return IntPtr.Zero
    End Function 'OnMsgProc


    '/ <summary>
    '/ This function loads the mesh and ensures the mesh has normals; it also optimizes the 
    '/ mesh for the graphics card's vertex cache, which improves performance by organizing 
    '/ the internal triangle list for less cache misses.
    '/ </summary>
    Private Function LoadMesh(ByVal device As Device, ByVal filename As String) As mesh
        ' Load the mesh with D3DX and get back a Mesh.  For this
        ' sample we'll ignore the X file's embedded materials since we know 
        ' exactly the model we're loading.  See the mesh samples such as
        ' "OptimizedMesh" for a more generic mesh loading example.
        Dim path As String = Utility.FindMediaFile(filename)
        Dim m As Mesh = mesh.FromFile(path, MeshFlags.Managed, device)

        ' Make sure there are normals which are required for lighting
        If (m.VertexFormat And VertexFormats.Normal) = 0 Then
            ' Clone the mesh
            Dim tempMesh As Mesh = m.Clone(m.Options.Value, m.VertexFormat Or VertexFormats.Normal, device)
            ' Compute the normals
            tempMesh.ComputeNormals()
            ' Dispose of the original mesh
            m.Dispose()
            ' use the newly cloned mesh
            m = tempMesh
        End If

        ' Optimize the mesh for this graphics card's vertex cache 
        ' so when rendering the mesh's triangle list the vertices will 
        ' cache hit more often so it won't have to re-execute the vertex shader 
        ' on those vertices so it will improve perf.     
        Dim adj As Integer() = m.ConvertPointRepsToAdjacency(CType(Nothing, GraphicsStream))
        m.OptimizeInPlace(MeshFlags.OptimizeVertexCache, adj)

        ' return the mesh
        Return m
    End Function 'LoadMesh


    '/ <summary>Initializes the application</summary>
    Public Sub InitializeApplication()
        isUsingPreshader = True

        Dim i As Integer
        For i = 0 To MaxNumberLights - 1
            lightControl(i) = New DirectionWidget
            lightControl(i).LightDirection = New Vector3(System.Convert.ToSingle(Math.Sin(System.Convert.ToSingle(Math.PI) * 2 * i / MaxNumberLights - System.Convert.ToSingle(Math.PI) / 6)), 0, -System.Convert.ToSingle(Math.Cos(System.Convert.ToSingle(Math.PI) * 2 * i / MaxNumberLights - System.Convert.ToSingle(Math.PI) / 6)))
        Next i

        activeLight = 0
        numberActiveLights = 1
        lightScale = 1.0F

        Dim y As Integer = 10
        ' Initialize the dialogs
        Dim fullScreen As Button = hud.AddButton(ToggleFullscreen, "Toggle full screen", 35, y, 125, 22) : y += 24
        Dim toggleRef As Button = hud.AddButton(ToggleReference, "Toggle reference (F3)", 35, y, 125, 22) : y += 24
        Dim changeDeviceButton As Button = hud.AddButton(ChangeDevice, "Change Device (F2)", 35, y, 125, 22) : y += 24
        ' Hook the button events for when these items are clicked
        AddHandler fullScreen.Click, AddressOf OnFullscreenClicked
        AddHandler toggleRef.Click, AddressOf OnRefClicked
        AddHandler changeDeviceButton.Click, AddressOf OnChangeDevicClicked

        ' Now add the sample specific UI
        y = 10
        sampleUi.AddStatic(NumberLightsStatic, String.Format("# Lights: {0}", numberActiveLights), 35, y, 125, 22) : y += 24
        Dim lightSlider As Slider = sampleUi.AddSlider(NumberLights, 50, y, 100, 22, 1, MaxNumberLights, numberActiveLights, False) : y += 24

        y += 24
        sampleUi.AddStatic(LightScaleStatic, String.Format("Light scale: {0}", lightScale.ToString("f2", System.Globalization.CultureInfo.CurrentUICulture)), 35, y, 125, 22) : y += 24
        Dim scaleSlider As Slider = sampleUi.AddSlider(LightScaleControl, 50, y, 100, 22, 0, 20, lightScale * 10.0F, False) : y += 24

        y += 24
        Dim activeLightControlButton As Button = sampleUi.AddButton(ActiveLightControl, "Change active light (K)", 35, y, 125, 22, System.Windows.Forms.Keys.K, False) : y += 24
        Dim preShader As Checkbox = sampleUi.AddCheckBox(EnablePreshader, "Enable preshaders", 35, y, 125, 22, isUsingPreshader)

        ' Hook the events
        AddHandler preShader.Changed, AddressOf OnPreshaderClick
        AddHandler lightSlider.ValueChanged, AddressOf OnNumberLightsChanged
        AddHandler activeLightControlButton.Click, AddressOf OnActiveLightClick
        AddHandler scaleSlider.ValueChanged, AddressOf OnLightScaleChanged
    End Sub 'InitializeApplication


    '/ <summary>Called when the light scale has changed</summary>
    Private Sub OnLightScaleChanged(ByVal sender As Object, ByVal e As EventArgs)
        Dim sl As Slider = sender
        lightScale = System.Convert.ToSingle(sl.Value * 0.1F)

        Dim [text] As StaticText = sampleUi.GetStaticText(LightScaleStatic)
        [text].SetText(String.Format("Light scale: {0}", lightScale.ToString("f2", System.Globalization.CultureInfo.CurrentUICulture)))
    End Sub 'OnLightScaleChanged


    '/ <summary>Called when active light button has been clicked</summary>
    Private Sub OnActiveLightClick(ByVal sender As Object, ByVal e As EventArgs)
        If Not lightControl(activeLight).IsBeingDragged Then
            activeLight += 1
            activeLight = activeLight Mod numberActiveLights
        End If
    End Sub 'OnActiveLightClick


    '/ <summary>Called when the number of lights has changed</summary>
    Private Sub OnNumberLightsChanged(ByVal sender As Object, ByVal e As EventArgs)
        If Not lightControl(activeLight).IsBeingDragged Then
            Dim [text] As StaticText = sampleUi.GetStaticText(NumberLightsStatic)
            Dim lights As Slider = sampleUi.GetSlider(NumberLights)

            numberActiveLights = lights.Value

            [text].SetText(String.Format("# Lights: {0}", numberActiveLights))

            activeLight = activeLight Mod numberActiveLights
        End If
    End Sub 'OnNumberLightsChanged


    '/ <summary>Called when the pre-shader button is changed</summary>
    Private Sub OnPreshaderClick(ByVal sender As Object, ByVal e As EventArgs)
        Dim cb As Checkbox = sender
        isUsingPreshader = cb.IsChecked

        If Not (sampleFramework.Device Is Nothing) Then
            ' Destroy and recreate the objects to update the preshader stuff
            OnLostDevice(Nothing, EventArgs.Empty)
            OnDestroyDevice(Nothing, EventArgs.Empty)
            OnCreateDevice(Nothing, New DeviceEventArgs(sampleFramework.Device, sampleFramework.BackBufferSurfaceDescription))
            OnResetDevice(Nothing, New DeviceEventArgs(sampleFramework.Device, sampleFramework.BackBufferSurfaceDescription))
        End If
    End Sub 'OnPreshaderClick


    '/ <summary>Called when the change device button is clicked</summary>
    Private Sub OnChangeDevicClicked(ByVal sender As Object, ByVal e As EventArgs)
        sampleFramework.ShowSettingsDialog(Not sampleFramework.IsD3DSettingsDialogShowing)
    End Sub 'OnChangeDevicClicked


    '/ <summary>Called when the full screen button is clicked</summary>
    Private Sub OnFullscreenClicked(ByVal sender As Object, ByVal e As EventArgs)
        sampleFramework.ToggleFullscreen()
    End Sub 'OnFullscreenClicked


    '/ <summary>Called when the ref button is clicked</summary>
    Private Sub OnRefClicked(ByVal sender As Object, ByVal e As EventArgs)
        sampleFramework.ToggleReference()
    End Sub 'OnRefClicked


    '/ <summary>
    '/ Entry point to the program. Initializes everything and goes into a message processing 
    '/ loop. Idle time is used to render the scene.
    '/ </summary>
    Shared Function Main() As Integer
        System.Windows.Forms.Application.EnableVisualStyles()
        Dim sampleFramework As New Framework
        Try
            Dim sample As New BasicHLSL(sampleFramework)
            ' Set the callback functions. These functions allow the sample framework to notify
            ' the application about device changes, user input, and windows messages.  The 
            ' callbacks are optional so you need only set callbacks for events you're interested 
            ' in. However, if you don't handle the device reset/lost callbacks then the sample 
            ' framework won't be able to reset your device since the application must first 
            ' release all device resources before resetting.  Likewise, if you don't handle the 
            ' device created/destroyed callbacks then the sample framework won't be able to 
            ' recreate your device resources.
            AddHandler sampleFramework.Disposing, AddressOf sample.OnDestroyDevice
            AddHandler sampleFramework.DeviceLost, AddressOf sample.OnLostDevice
            AddHandler sampleFramework.DeviceCreated, AddressOf sample.OnCreateDevice
            AddHandler sampleFramework.DeviceReset, AddressOf sample.OnResetDevice

            sampleFramework.SetWndProcCallback(AddressOf sample.OnMsgProc)

            sampleFramework.SetCallbackInterface(sample)
            Try

                ' Show the cursor and clip it when in full screen
                sampleFramework.SetCursorSettings(True, True)

                ' Initialize
                sample.InitializeApplication()

                ' Initialize the sample framework and create the desired window and Direct3D 
                ' device for the application. Calling each of these functions is optional, but they
                ' allow you to set several options which control the behavior of the sampleFramework.
                sampleFramework.Initialize(True, True, True) ' Parse the command line, handle the default hotkeys, and show msgboxes
                sampleFramework.CreateWindow("BasicHLSL")
                AddHandler sampleFramework.Window.KeyDown, AddressOf sample.OnKeyEvent
                sampleFramework.CreateDevice(0, True, Framework.DefaultSizeWidth, Framework.DefaultSizeHeight, sample)

                ' Pass control to the sample framework for handling the message pump and 
                ' dispatching render calls. The sample framework will call your FrameMove 
                ' and FrameRender callback when there is idle time between handling window messages.
                sampleFramework.MainLoop()

#If (DEBUG = 1) Then
            Catch e As Exception
                ' In debug mode show this error (maybe - depending on settings)
                sampleFramework.DisplayErrorMessage(e)
#Else
            Catch
#End If
                ' Ignore any exceptions here, they would have been handled by other areas
                If (sampleFramework.ExitCode = 0) Then
                    Return 1
                Else
                    Return sampleFramework.ExitCode
                End If
            End Try
            ' Perform any application-level cleanup here. Direct3D device resources are released within the
            ' appropriate callback functions and therefore don't require any cleanup code here.
            Return sampleFramework.ExitCode
        Finally
            sampleFramework.Dispose()
        End Try
    End Function 'Main
End Class 'BasicHLSL