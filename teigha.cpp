#include <cstdio>
#include <iostream>

#include <GL/glew.h>

#include <OdaCommon.h>
#include <OdPlatform.h>

#include <AbstractViewPE.h>
#include <ColorMapping.h>
#include <DbDatabase.h>
#include <DbGsManager.h>
#include <GiContextForDbDatabase.h>
#include <OdAlloc.h>
#include <RxDynamicModule.h>
#include <RxObject.h>
#include <RxVariantValue.h>

#include <ExHostAppServices.h>
#include <ExSystemServices.h>

#include <glfw3.h>
#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_GLX
#include <glfw3native.h>

#include <shader.hpp>

const int WIDTH = 640;
const int HEIGHT = 480;

GLFWwindow* window;

// Services class
class MyServices : public ExSystemServices, public ExHostAppServices
{
  protected:
    ODRX_USING_HEAP_OPERATORS(ExSystemServices);
};

int main(int /*argc*/, char *argv[])
{
    // Initialise GLFW
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_RESIZABLE,GL_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE );

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(WIDTH, HEIGHT, "Teigha", NULL, NULL);
    if( window == NULL )
    {
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible.\n" );
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // Cull triangles which normal is not towards the camera
    glEnable(GL_CULL_FACE);

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Create and compile our GLSL program from the shaders
    GLuint programID = LoadShaders( "SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader" );

    static const GLfloat g_vertex_buffer_data[] = {
        -0.1f, -0.1f, 0.0f,
         0.1f, -0.1f, 0.0f,
         0.0f,  0.1f, 0.0f,
    };

    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

    OdStaticRxObject<MyServices> svcs;
    odInitialize(&svcs);

    try
    {
      // Load database
      OdDbDatabasePtr pDb;
      pDb = svcs.readFile(argv[1], false, false, Oda::kShareDenyNo);
      OdGiContextForDbDatabasePtr pDwgContext = OdGiContextForDbDatabase::createObject();

      // Load OpenGL vectorizer
      OdGsModulePtr pGs = ::odrxDynamicLinker()->loadModule(OdWinOpenGLModuleName);
      OdGsDevicePtr pDevice = pGs->createDevice();

      // Begin OpenGL initialization
      OdRxDictionaryPtr pProperties = pDevice->properties();
      Display* x11Display = glfwGetX11Display();
      pProperties->putAt(OD_T("XDisplay"), OdRxVariantValue((OdIntPtr)x11Display));
      Window x11Window = glfwGetX11Window(window);
      pProperties->putAt(OD_T("XWindow"), OdRxVariantValue((OdIntPtr)x11Window));
      GLXContext glxContext = glfwGetGLXContext(window);
      pProperties->putAt(OD_T("GLXContext"), OdRxVariantValue((OdIntPtr)glxContext));

      // Initialize Gs device
      pDwgContext->setDatabase(pDb);
      pDevice = OdDbGsManager::setupActiveLayoutViews(pDevice, pDwgContext);

      // Resize
      OdGsDCRect screenRect(OdGsDCPoint(0, HEIGHT), OdGsDCPoint(WIDTH, 0));
      pDevice->onSize(screenRect);

      // Zoom to extents
      OdAbstractViewPEPtr(pDevice->viewAt(0))->zoomExtents(pDevice->viewAt(0));

      do
      {
          // Clear the screen
          glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

          pDevice->update();

          glBegin(GL_LINES);
              GLfloat x = 3;
              glVertex3f(-50, 0, (GLfloat)x);
              glVertex3f( 50, 0, (GLfloat)x);
              glVertex3f((GLfloat)x, 0, -50);
              glVertex3f((GLfloat)x, 0,  50);
          glEnd();

          // Use our shader
          glUseProgram(programID);

          // 1rst attribute buffer : vertices
          glEnableVertexAttribArray(0);

          glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
          glVertexAttribPointer(
                  0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                  3,                  // size
                  GL_FLOAT,           // type
                  GL_FALSE,           // normalized?
                  0,                  // stride
                  (void*)0            // array buffer offset
          );

          // Draw the triangle !
          glDrawArrays(GL_TRIANGLES, 0, 3); // 3 indices starting at 0 -> 1 triangle

          glDisableVertexAttribArray(0);

          // Swap buffers
          glfwSwapBuffers(window);
          glfwPollEvents();

      } // Check if the ESC key was pressed or the window was closed
      while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
            glfwWindowShouldClose(window) == 0 );
    }
    catch (const OdError& e)
    {
      std::cout << "\nException caught: " << e.description().c_str() << "\n";
    }
    catch (...)
    {
      std::cout << "\nUnknown exception caught\n";
    }

    odUninitialize();

    glDeleteBuffers(1, &vertexbuffer);
    glDeleteVertexArrays(1, &VertexArrayID);
    glDeleteProgram(programID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}

