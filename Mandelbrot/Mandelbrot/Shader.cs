using System;
using System.Drawing;
using System.IO;
using System.Text;
using OpenTK;
using OpenTK.Graphics.OpenGL;
using PixelFormat = OpenTK.Graphics.OpenGL.PixelFormat;

namespace HW1
{
    public class Shader
    {
        public int ProgramId { get; }

        public Shader(string vertexPath, string fragmentPath)
        {
            string vertexShaderSource;
            using (var reader = new StreamReader(vertexPath, Encoding.UTF8))
            {
                vertexShaderSource = reader.ReadToEnd();
            }

            string fragmentShaderSource;
            using (var reader = new StreamReader(fragmentPath, Encoding.UTF8))
            {
                fragmentShaderSource = reader.ReadToEnd();
            }
            
            var vertexShader = GL.CreateShader(ShaderType.VertexShader);
            GL.ShaderSource(vertexShader, vertexShaderSource);

            var fragmentShader = GL.CreateShader(ShaderType.FragmentShader);
            GL.ShaderSource(fragmentShader, fragmentShaderSource);
            
            GL.CompileShader(vertexShader);
            
            var infoLogVert = GL.GetShaderInfoLog(vertexShader);
            if (infoLogVert != string.Empty)
            {
                Console.WriteLine(infoLogVert);
            }

            GL.CompileShader(fragmentShader);

            var infoLogFrag = GL.GetShaderInfoLog(fragmentShader);

            if (infoLogFrag != string.Empty)
            {
                Console.WriteLine(infoLogFrag);
            }
            
            ProgramId = GL.CreateProgram();

            GL.AttachShader(ProgramId, vertexShader);
            GL.AttachShader(ProgramId, fragmentShader);

            GL.LinkProgram(ProgramId);
            
            GL.DetachShader(ProgramId, vertexShader);
            GL.DetachShader(ProgramId, fragmentShader);
            GL.DeleteShader(fragmentShader);
            GL.DeleteShader(vertexShader);
        }
        
        public void Use()
        {
            GL.UseProgram(ProgramId);
        }

        ~Shader()
        {
            GL.DeleteProgram(ProgramId);
        }

        public void SetUniform(string name, Vector2 value)
        {
            GL.Uniform2(GL.GetUniformLocation(ProgramId, name), value);
        }

        public int InitTexture(string filename)
        {
            var data = LoadTexture1D(filename, out var width);
            GL.CreateTextures(TextureTarget.Texture1D, 1, out int texture);
            GL.TextureStorage1D(texture, 1, SizedInternalFormat.Rgba32f, width);
            GL.TextureSubImage1D(texture, 0, 0, width, PixelFormat.Rgba, PixelType.Float, data);
            return texture;
        }

        public void SetUniformTexture1D(string name, int index)
        {
            GL.Uniform1(GL.GetUniformLocation(ProgramId, name), index);
        }

        public void SetUniform(string name, int value)
        {
            GL.Uniform1(GL.GetUniformLocation(ProgramId, name), value);
        }
        
        public void SetUniform(string name, float value)
        {
            GL.Uniform1(GL.GetUniformLocation(ProgramId, name), value);
        }
        
        public void SetUniform(string name, Matrix4 value)
        {
            var matrixArray = new float[4 * 4];
            for (var i = 0; i < 4; i++)
            {
                for (var j = 0; j < 4; j++)
                {
                    matrixArray[i * 4 + j] = value[i, j];
                }
            }
            GL.UniformMatrix4(GL.GetUniformLocation(ProgramId, name), 1, false, matrixArray);
        }

        private static float[] LoadTexture1D(string filename, out int width)
        {
            using var bmp = (Bitmap) Image.FromFile(filename);
            width = bmp.Width;
            
            var pixels = new float[width * 4];
            var index = 0;
            
            for (var x = 0; x < width; x++)
            {
                var pixel = bmp.GetPixel(x, 0);
                pixels[index++] = pixel.R / 255f;
                pixels[index++] = pixel.G / 255f;
                pixels[index++] = pixel.B / 255f;
                pixels[index++] = pixel.A / 255f;
            }

            return pixels;
        }
    }
}