using System;
using System.ComponentModel;
using System.IO;
using System.Windows.Forms;
using HW1;
using OpenTK;
using OpenTK.Graphics.OpenGL;

namespace Mandelbrot
{
    public partial class MandelbrotForm : Form
    {
        private readonly string vertexShaderPath = Path.Combine(Environment.CurrentDirectory, "Assets\\shader.vert");
        private readonly string fragmentShaderPath = Path.Combine(Environment.CurrentDirectory, "Assets\\shader.frag");
        private readonly string palettePath = Path.Combine(Environment.CurrentDirectory, "Assets\\palette.png");

        private const float scaleFactor = 0.05f;

        private int maxIterations = 1000;
        private float depthAffect = 0.5f;
        private Vector2 z0 = Vector2.Zero;

        private Shader shader;
        
        private uint indexBufferId;
        private uint vertexBufferId;

        private int texture;

        private bool isDragging;
        private Vector2 lastDraggingPosition;
        private Vector2 currentDraggingPosition;

        private Matrix4 currentTransform;
        
        public MandelbrotForm()
        {
            InitializeComponent();
        }

        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad(e);
            Application.Idle += OnApplicationIdle;
            OnGLControlResize(glControl, EventArgs.Empty);
        }
        
        private void OnGLControlLoad(object sender, EventArgs e)
        {
            shader = new Shader(vertexShaderPath, fragmentShaderPath);
            InitQuad();
            texture = shader.InitTexture(palettePath);
            currentTransform = Matrix4.Identity;
        }
        
        protected override void OnClosing(CancelEventArgs e)
        {
            Application.Idle -= OnApplicationIdle;
            base.OnClosing(e);
        }
        
        private void OnApplicationIdle(object sender, EventArgs e)
        {
            while (glControl.IsIdle)
            {
                OnGLControlRender(sender, e);
            }
        }

        private void OnGLControlResize(object sender, EventArgs e)
        {
            var control = sender as GLControl;
            
            GL.Viewport(0, 0, control.ClientRectangle.Width, control.ClientRectangle.Height);

            var projection = Matrix4.CreatePerspectiveFieldOfView((float)Math.PI / 4, glControl.Width / (float)glControl.Height, 1.0f, 64.0f);
            GL.MatrixMode(MatrixMode.Projection);
            GL.LoadMatrix(ref projection);
        }

        private void OnGLControlRender(object sender, EventArgs e)
        {
            GL.Clear(ClearBufferMask.ColorBufferBit);
        
            shader.SetUniform("z0", z0);
            shader.SetUniform("maxIterations", maxIterations);
            shader.SetUniform("depthAffect", depthAffect);
            shader.SetUniformTexture1D("palette", 0);

            shader.SetUniform("transform", currentTransform);
            
            shader.Use();
            
            GL.BindBuffer(BufferTarget.ArrayBuffer, vertexBufferId);
            GL.EnableClientState(ArrayCap.VertexArray);
            GL.VertexPointer(2, VertexPointerType.Float, 0, IntPtr.Zero);
            GL.BindBuffer(BufferTarget.ElementArrayBuffer, indexBufferId);
            
            GL.BindTexture(TextureTarget.Texture1D, texture);
            
            GL.DrawElements( PrimitiveType.Quads, 4, DrawElementsType.UnsignedShort, IntPtr.Zero);
            
            GL.DisableClientState(ArrayCap.VertexArray);

            glControl.SwapBuffers();
        }

        private void InitQuad()
        {
            ushort[] indices = { 0, 1, 2, 3 };

            GL.GenBuffers(1, out indexBufferId);
            GL.BindBuffer(BufferTarget.ElementArrayBuffer, indexBufferId);
            GL.BufferData(
                BufferTarget.ElementArrayBuffer,
                (IntPtr)(indices.Length * sizeof(ushort)),
                indices,
                BufferUsageHint.StaticDraw);

            float[] vertexData = 
            {
                -1.0f, -1.0f,
                1.0f, -1.0f,
                1.0f, 1.0f,
                -1.0f, 1.0f 
            };

            GL.GenBuffers(1, out vertexBufferId);
            GL.BindBuffer(BufferTarget.ArrayBuffer, vertexBufferId);
            GL.BufferData(
                BufferTarget.ArrayBuffer,
                (IntPtr)(vertexData.Length * sizeof(float)),
                vertexData,
                BufferUsageHint.StaticDraw);

        }

        private void glControl_MouseDown(object sender, MouseEventArgs e)
        {
            if (!isDragging && e.Button == MouseButtons.Left)
            {
                isDragging = true;
                currentDraggingPosition = MouseToWorld(new Vector2(e.X, e.Y));
            }
        }

        private void glControl_MouseMove(object sender, MouseEventArgs e)
        {
            if (isDragging)
            {
                lastDraggingPosition = currentDraggingPosition;
                currentDraggingPosition = MouseToWorld(new Vector2(e.X, e.Y));
                var delta = lastDraggingPosition - currentDraggingPosition;
                currentTransform = Matrix4.CreateTranslation(delta.X, delta.Y, 0) * currentTransform;
            }
        }

        private Vector2 MouseToWorld(Vector2 mousePosition)
        {
            return new Vector2(2 * mousePosition.X / glControl.Width - 1, 
                2 * (glControl.Height - mousePosition.Y) / glControl.Height - 1);
        }
        
        private void glControl_MouseUp(object sender, MouseEventArgs e)
        {
            isDragging = false;
            lastDraggingPosition = Vector2.Zero;
            currentDraggingPosition = Vector2.Zero;
        }

        private void glControl_MouseWheel(object sender, MouseEventArgs e)
        {
            var fixedPoint = MouseToWorld(new Vector2(e.X, e.Y));
            var scale = scaleFactor * Math.Sign(e.Delta);
            
            currentTransform = Matrix4.CreateTranslation(-fixedPoint.X, -fixedPoint.Y, 0) * 
                               Matrix4.CreateScale(1 - scale) *
                               Matrix4.CreateTranslation(fixedPoint.X, fixedPoint.Y, 0) * currentTransform;
        }

        private void trackBar1_ValueChanged(object sender, EventArgs e)
        {
            var value = (sender as TrackBar).Value;
            label1.Text = value.ToString();
            maxIterations = value;
        }
        
        private void trackBar2_ValueChanged(object sender, EventArgs e)
        {
            var value = (sender as TrackBar).Value;
            label2.Text = $@"{value}%";
            depthAffect = value / 100.0f;
        }
        
        private void trackBar3_ValueChanged(object sender, EventArgs e)
        {
            var value = (sender as TrackBar).Value / 100.0f;
            label3.Text = $@"{value}";
            z0.X = value;
        }

        private void trackBar4_ValueChanged(object sender, EventArgs e)
        {
            var value = (sender as TrackBar).Value / 100.0f;
            label4.Text = $@"{value}";
            z0.Y = value;
        }
    }
}