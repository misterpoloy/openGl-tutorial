#include <GL/glew.h> // This library has to go first! #JP
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream> // File stream to parse shader
#include <string> // getLine is used to read from line
#include <sstream> // std::streamString

struct ShaderProgramSource {
  std::string VertexSource;
  std::string FragmentSource;
};

// Get the sahders from file
static ShaderProgramSource parseShader(const std::string& filePath) {
  std::ifstream stream(filePath);
  
  enum class ShaderType {
    NONE = -1, VERTEX = 0, FRAGMENT = 1
  };

  std::string line;
  ShaderType type = ShaderType::NONE;
  std::stringstream ss[2];

  while (getline(stream, line)) {
    if (line.find("#shader") != std::string::npos) {
      if (line.find("vertex") != std::string::npos) {
        type = ShaderType::VERTEX;
      }
      else if(line.find("fragment") != std::string::npos) {
        type = ShaderType::FRAGMENT;
      }
    } else {
      ss[(int)type] << line << '\n';
    }
  }
  return { ss[0].str(), ss[1].str() };
}

// Shader creation
static unsigned int CompileShader(unsigned int type, const std::string& source) {
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str(); // Remdinder: Source needs to exist
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id); //We can query this
    
    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS,  &result);
    if (result == GL_FALSE) {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        // Return the massage as an char array
        char* message = (char*)alloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length,  message);
        std::cout << "Failed to compile shader!" << std::endl;
        std::cout << message << std::endl;
        glDeleteShader(id);
        return 0;
    }
    
    return id;
}

static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader) {
    unsigned int program = glCreateProgram();
    // Create Shader
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);
    // Attach shaders to the program
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program); // id of the program;
    glValidateProgram(program);
    
    glDeleteShader(vs);
    glDeleteShader(fs);
    
    return program;
}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;
    
    /**
        IMPORTANT:
        MacOS uses Legacy Profile as default for all created OpenGL context. Therefor by default only OpenGL up to 2.1 and GLSL up to 1.20 is supported.
        https://stackoverflow.com/questions/20931528/shader-cant-be-compiled
     */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    /* Inicialize Glew #JP, THIS SHOULD BE AFTER GLFW CONTEXT */
    if(glewInit() != GLEW_OK)
        std::cout << "Error!" << std::endl;
    
    float position[6] {
        -0.5f, -0.5f,
        0.0f, 0.5f,
        0.5f, -0.5f,
    };
    
    // STEP: 0 - Vertex Buffers
    unsigned int buffer;
    /**
     IMPORTANT:
     Modern OpenGL (Core version, example 4.2) requires that we use a VAO (vertex array object) that stores how the data of our array is organized and in which buffer (VBO - vertex buffer object) we store the vertices that are in our array. If we don't create and link a VAO with that information, OpenGL most likely won't draw anything.
     */
    GLuint vertexArrayID;
    glGenVertexArrays(1, &vertexArrayID);
    glBindVertexArray(vertexArrayID);

    glGenBuffers(1, &buffer); // Create one buffer and asign id
    glBindBuffer(GL_ARRAY_BUFFER, buffer); // Bind buffer of type array with our id, like select layer in Photoshop
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), position , GL_STATIC_DRAW);
    
    // Print current version
    std::cout << "Supported GLSL version is: " << (char *)glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    // STEP: 1 - Vertex Attributes and Layouts
    // Enable created Vertex
    glEnableVertexAttribArray(0);
    // Create Vertex Attribute Pointer
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);
    
//    std::string vertexShader =
//        "#version 330 core\n"
//        "\n"
//        "layout(location = 0) in vec4 position;\n" /// layout(location = 0), so int can now from glVertexAttribPointer(0, ...)
//        "\n"
//        "void main()\n"
//        "{\n"
//        "   gl_Position = position;\n"
//        "}\n";
//
//    std::string fragmentShader =
//        "#version 330 core\n"
//        "\n"
//        "layout(location = 0) out vec4 color;\n" /// layout(location = 0), so int can now from glVertexAttribPointer(0, ...)
//        "\n"
//        "void main()\n"
//        "{\n"
//        "   color = vec4(1.0, 0.0, 0.0, 1.0);\n" // flotas betwen from 0 and 1, 0 is white an 1 is red R G B
//        "}\n";
  
    // https://stackoverflow.com/questions/23438393/new-to-xcode-cant-open-files-in-c/23448835#23448835
    ShaderProgramSource source = parseShader("res/shaders/Basic.shader");
    
    unsigned int shader = CreateShader(source.VertexSource, source.FragmentSource);
    glUseProgram(shader);
      
    // glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    // Shader, vertex shader and fragment shader also know as Pixel Shaders
    // Draw call, vertex shader to fragment shader
    // Vertex shaders occurs once per vertex
    // Fragment shader can occur multiple times, the necessary to fill our content.

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);
        
        /* #JP DRAW */
        glDrawArrays(GL_TRIANGLES, 0, 3);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();

    // IMPORTANT to free the memory again
    glDeleteProgram(shader);

    return 0;
}
