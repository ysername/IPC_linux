/*
 * @copyright Copyright (c) 2016 Altia, Inc.
 *
 * Permission to use, copy, and modify this software for any purpose is
 * hereby granted, provided that this copyright notice appears in all
 * copies and that the entity using, copying, or modifying this software
 * has purchased an Altia Deep Screen license.  Permission to sell or
 * distribute this source code is denied.
 *
 * Altia makes no representations about the suitability of this software for
 * any purpose.  It is provided "as is" without express or implied warranty.
 *
 * ALTIA INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
 * IN NO EVENT SHALL ALTIA INC. BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * @author  Mike Yeakley
 * @date    9/9/16
 * @brief   Updates for Yazaki FF HUD DualLite HW
 *
 * @author  Mike Yeakley
 * @date    4/28/16
 * @brief   Original version.
 */

/******************************************************************************
 * Includes
 ******************************************************************************/
#include <INTEGRITY.h>
#include <stdio.h>                  // printf()
#include <unistd.h>                 // WaitForFileSystemInitialization()

#include <gc_vdk.h>                 // Vivante VDK
#include <device/fbdriver.h>        // Vivante VDK
#include <device/fbdriver_imx6.h>   // Vivante VDK

#include "GLES2/gl2.h"              // OpenGL ES 2
#include "GLES2/gl2ext.h"           // OpenGL ES 2
#include "EGL/egl.h"                // OpenGL ES 2, EGL

#include "OpenGLES2/egl_Wrapper.h"
extern const char *defVShader;
extern const char *defFShader;
extern void esMatrixMultiply(ESMatrix *result, ESMatrix *srcA, ESMatrix *srcB);

/******************************************************************************
 * Definitions
 ******************************************************************************/
/*
 * Error checking
 */
#if defined(DISPLAY)
#error DISPLAY defined already!  Figure out from where and undefine it to prevent conflicts!
#endif  // #if defined(DISPLAY)

#if defined(DISPLAY_HEIGHT)
#error DISPLAY_HEIGHT defined already!  Figure out from where and undefine it to prevent conflicts!
#endif  // #if defined(DISPLAY_HEIGHT)

#if defined(DISPLAY_WIDTH)
#error DISPLAY_WIDTH defined already!  Figure out from where and undefine it to prevent conflicts!
#endif  // #if defined(DISPLAY_HEIGHT)


/*
 * Set up various config params based on platform / display type
 */
//#define PLATFORM_TYPE_SABRE_LVDS        1
#define PLATFORM_TYPE_YAZAKI_RGB        1

#define TURN_ON_ANIMATED_OBJECTS        1


#if defined(PLATFORM_TYPE_SABRE_LVDS)
#define DISPLAY             "imx6_lvds_ipu1"
#define DISPLAY_WIDTH       1024
#define DISPLAY_HEIGHT      768

#elif defined(PLATFORM_TYPE_YAZAKI_RGB)
/*
 * :NOTE:  Yazaki display size limited to 864x480 per imx6-yazaki-rgblcd.c
 */
#define DISPLAY             "imx6_rgb_ipu1"
#define DISPLAY_WIDTH       854                 // Was 864
#define DISPLAY_HEIGHT      480

#else
#error PLATFORM_TYPE_xxx not defined anywhere!  Figure out the proper definition and adjust the source file / build settings!

#endif  // #if defined(PLATFORM_TYPE_SABRE_LVDS)


/*
 * Basic object dimensions for simple rectangles that get drawn and animated across
 * the screen
 */
#define OBJ_HEIGHT          (100)
#define OBJ_WIDTH           (100)


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Altia macros
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define F2VT(x)             ((GLfloat)(x))
#define VT2F(x)             ((GLfloat)(x))
#define F2FX(x)             ((GLfloat)(x))
#define FX2F(x)             ((GLfloat)(x))

/* Variable names used in the shaders */
#define SHDR_POSITION_NAME  "a_position"
#define SHDR_TEXCOORD_NAME  "a_texCoord"
#define SHDR_COLOR_NAME     "a_color"
#define SHDR_TEXTURE_NAME   "u_texture"
#define SHDR_PMVMATRIX_NAME "u_pmvMatrix"
#define SHDR_OBJXFORM_NAME  "u_objXForm"
#define SHDR_TEXTYPE_NAME   "u_texType" /* 1-Alpha only, 2-RGBA */
#define SHDR_TEXTURE_NAME2  "u_texture2"
#define SHDR_VERTEX_OPER    "u_vertexOperator"
#define SHDR_VERTEX_OFFSET  "u_vertexOffset"
#define SHDR_ALPHA_OPER     "u_alphaOperator"
#define SHDR_ALPHA_OFFSET   "u_alphaOffset"

#define SHDR_POSITION_IDX  0
#define SHDR_TEXCOORD_IDX  1
#define SHDR_COLOR_IDX     2

/* Macros to add offset to the x-y positions of rectangle objects */
#ifndef EGL_RECT_X_OFFSET
#define EGL_RECT_X_OFFSET 0
#endif

#ifndef EGL_RECT_Y_OFFSET
#define EGL_RECT_Y_OFFSET 0
#endif

#ifndef ALTIA_RECTANGLE_TRIANGLE_TYPE
#define ALTIA_RECTANGLE_TRIANGLE_TYPE   GL_TRIANGLE_STRIP
#endif
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*
 * Check for OpenGL ES 2.0 errors
 */
#define CHECK_GL_ERROR()                checkGlError(__FILE__,__LINE__)
#define CHECK_EGL_ERROR()               checkEglError(__FILE__,__LINE__)


/******************************************************************************
 * Function prototypes
 ******************************************************************************/
extern void VivanteInit();


/******************************************************************************
 * Global variables
 ******************************************************************************/
const int demo_ver = 1;
const int demo_rev = 2;


/******************************************************************************
 * Private types
 ******************************************************************************/
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Altia types
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#if 0
/* Replaces some functions missing from OpenGL ES 2.0 */
typedef struct
{
    GLfloat m[4][4];
} ESMatrix;

typedef GLfloat        EGL_FLOAT;
typedef GLfloat        EGL_POS;
#endif

/*
 * Similar to EGL_LAYER_T in egl_Wrapper.h
 */
typedef struct SLayerInfo_t
{
    NativeDisplayType nativeDisplay;
    NativeWindowType nativeWindow;
    EGLDisplay eglDisplay;
    EGLConfig eglConfig;
    EGLContext eglContext;
    EGLSurface eglSurface;
    EGLint matchingConfig;
    GLuint vShader;
    GLuint fShader;
    GLuint shaderProgObj;
    ESMatrix pmvMatrix;
} SLayerInfo_t;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


typedef enum EHwLayer_t
{
    HW_LAYER_BG = 0,
    HW_LAYER_FG,
    HW_LAYER_MAX
} EHwLayer_t;


/**
 * @brief  RGB color info
 */
typedef struct SColorInfo_t
{
    GLfloat red;
    GLfloat green;
    GLfloat blue;
} SColorInfo_t;


/**
 * @brief  X,Y 2D Cartesian coordinate info
 */
typedef struct SCoord_t
{
    GLfloat x;
    GLfloat y;
} SCoord_t;


/**
 * @brief Rectangle info
 * @description  X,Y vertices coordinates map to screen style coordinates ie origin
 * aka the begin parameter, is in top left corner.
 */
typedef struct SRectInfo_t
{
    SCoord_t begin;
    SCoord_t end;
} SRectInfo_t;


/******************************************************************************
 * Private variables
 ******************************************************************************/
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Altia private globals
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/*
 * Altia EGL config attributes
 */
static EGLint configAttributes[] =
{
#ifdef ANTIALIAS_2D
    EGL_SAMPLE_BUFFERS, 1, /* 1 to turn on AA */
    EGL_SAMPLES,        4, /* 4x Anti-aliasing */
#else
    EGL_SAMPLE_BUFFERS, 0, /* 0 to turn off AA */
    EGL_SAMPLES,        0, /* No Anti-aliasing */
#endif
    EGL_RED_SIZE,       8,
    EGL_GREEN_SIZE,     8,
    EGL_BLUE_SIZE,      8,
    EGL_ALPHA_SIZE,     EGL_DONT_CARE,
    EGL_DEPTH_SIZE,     16,
    EGL_SURFACE_TYPE,   EGL_WINDOW_BIT,
    EGL_NONE,
};


static const EGLint attribListContext[] =
{
    EGL_CONTEXT_CLIENT_VERSION, 2,
    EGL_NONE
};


#if 0
/*
 * Altia shaders
 */
const char *defVShader = "\
    attribute mediump vec4 a_color; \
    attribute highp vec4 a_position;\
    attribute highp vec2 a_texCoord;\
    varying vec4 v_color;\
    varying vec2 v_texCoord;\
    uniform highp mat4 u_pmvMatrix;\
    uniform highp mat4 u_objXForm;\
    void main() {\
        v_color = a_color;\
        v_texCoord = a_texCoord;\
        gl_Position = u_pmvMatrix *u_objXForm *a_position;\
    }";

const char *defFShader = "\
    precision mediump float;\
    varying vec4 v_color;\
    varying vec2 v_texCoord;\
    uniform int u_texType;\
    uniform sampler2D u_texture;\
    void main() {\
        if(u_texType == 1)\
            gl_FragColor = vec4(v_color.rgb,\
                                texture2D(u_texture, v_texCoord).a *v_color.a);\
        else if(u_texType == 2) {\
            gl_FragColor = texture2D(u_texture, v_texCoord);\
            gl_FragColor.a *= v_color.a;\
        } else\
            gl_FragColor = v_color;\
    }";
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#endif

static SLayerInfo_t sg_layerInfo[HW_LAYER_MAX];

// upper left corner of display
static SRectInfo_t sg_bgRectInfo =
{
    { (GLfloat) (OBJ_WIDTH * 1), (GLfloat) (OBJ_HEIGHT * 1) },
    { (GLfloat) (OBJ_WIDTH * 2), (GLfloat) (OBJ_HEIGHT * 2) }
};

// lower right corner of display
static SRectInfo_t sg_fgRectInfo =
{
    { (GLfloat) (DISPLAY_WIDTH - (OBJ_WIDTH * 2)),  (GLfloat) (DISPLAY_HEIGHT - (OBJ_HEIGHT * 2)) },
    { (GLfloat) (DISPLAY_WIDTH - (OBJ_WIDTH * 1)),  (GLfloat) (DISPLAY_HEIGHT - (OBJ_HEIGHT * 1)) }
};

// upper left corner of display
static SRectInfo_t sg_errorRectInfo =
{
    { 0.0f,                         0.0f },
    { (0.0f + (GLfloat) OBJ_WIDTH), (0.0f + (GLfloat) OBJ_HEIGHT) }
};

static SColorInfo_t sg_blackColor = { 0.0f, 0.0f, 0.0f };
static SColorInfo_t sg_redColor   = { 1.0f, 0.0f, 0.0f };
static SColorInfo_t sg_greenColor = { 0.0f, 1.0f, 0.0f };
static SColorInfo_t sg_blueColor  = { 0.0f, 0.0f, 1.0f };


/******************************************************************************
 * Private functions
 ******************************************************************************/
/**
 * @brief  Check OpenGL ES 2.0 errors.
 * @see  https://blog.nobel-joergensen.com/2013/01/29/debugging-opengl-using-glgeterror/
 */
static void checkGlError(const char * file, int line)
{
    char * errorString;
    int err = glGetError();

    while (err != GL_NO_ERROR)
    {
        switch (err)
        {
            case GL_INVALID_OPERATION:              { errorString = "INVALID_OPERATION";                break; }
            case GL_INVALID_ENUM:                   { errorString = "INVALID_ENUM";                     break; }
            case GL_INVALID_VALUE:                  { errorString = "INVALID_VALUE";                    break; }
            case GL_OUT_OF_MEMORY:                  { errorString = "OUT_OF_MEMORY";                    break; }
            case GL_INVALID_FRAMEBUFFER_OPERATION:  { errorString = "INVALID_FRAMEBUFFER_OPERATION";    break; }
            default:                                { errorString = "UNKNOWN ERROR";                    break; }
        }

        printf("ERROR:  %s: %d:  OpenGL error - 0x%08X :  %s\n", file, line, err, errorString);

        err = glGetError();
    }
}


static void checkEglError(const char * file, int line)
{
    char * errorString;
    int err = eglGetError();

    /*
     * :NOTE:  https://www.khronos.org/registry/egl/sdk/docs/man/html/eglGetError.xhtml
     */
    while (err != EGL_SUCCESS)
    {
        switch (err)
        {
            case EGL_NOT_INITIALIZED:               { errorString = "NOT_INITIALIZED";              break; }
            case EGL_BAD_ACCESS:                    { errorString = "BAD_ACCESS";                   break; }
            case EGL_BAD_ALLOC:                     { errorString = "BAD_ALLOC";                    break; }
            case EGL_BAD_ATTRIBUTE:                 { errorString = "BAD_ATTRIBUTE";                break; }
            case EGL_BAD_CONFIG:                    { errorString = "BAD_CONFIG";                   break; }
            case EGL_BAD_CONTEXT:                   { errorString = "BAD_CONTEXT";                  break; }
            case EGL_BAD_CURRENT_SURFACE:           { errorString = "BAD_CURRENT_SURFACE";          break; }
            case EGL_BAD_DISPLAY:                   { errorString = "BAD_DISPLAY";                  break; }
            case EGL_BAD_MATCH:                     { errorString = "BAD_MATCH";                    break; }
            case EGL_BAD_NATIVE_PIXMAP:             { errorString = "BAD_NATIVE_PIXMAP";            break; }
            case EGL_BAD_NATIVE_WINDOW:             { errorString = "BAD_NATIVE_WINDOW";            break; }
            case EGL_BAD_PARAMETER:                 { errorString = "BAD_PARAMETER";                break; }
            case EGL_BAD_SURFACE:                   { errorString = "BAD_SURFACE";                  break; }
            case EGL_CONTEXT_LOST:                  { errorString = "CONTEXT_LOST";                 break; }
            default:                                { errorString = "UNKNOWN ERROR";                break; }
        }

        printf("ERROR:  %s: %d:  EGL error - 0x%08X :  %s\n", file, line, err, errorString);

        err = eglGetError();
    }
}


/*
 * Altia:  dispShaderErr()
 */
static void dispShaderErr(GLuint shader, char * txt)
{
    GLint len, chars;
    char * log;
    char * msg;

    /* Get length and error */
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
    log = (char *) malloc(len * sizeof(txt[0]));
    glGetShaderInfoLog(shader, len, &chars, log);

    /* Display error */
    msg = (char *) malloc((chars + 256) * sizeof(msg[0]));
    printf("ERROR:  shaders:\n  %s\n  %s\n", txt, log);

    free(msg);
    free(log);
}


/*
 * Altia:  loadShader()
 */
static GLuint loadShader(GLenum type, const char **code)
{
    GLint compiled;
    GLuint shader;

    /* Create, load, and compile */
    shader = glCreateShader(type);
    glShaderSource(shader, 1, code, NULL);
    glCompileShader(shader);

    /* Check for compile errors */
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if(!compiled)
    {
        char txt[256];
        sprintf(txt, "Failed to compile %s shader.", type == GL_VERTEX_SHADER ? "vertex" : "fragment");
        dispShaderErr(shader, txt);
        glDeleteShader(shader);
        return (GLuint)0;
    }

    return shader;
}

#if 0
/*
 * Altia:  esOrtho()
 * 3D Matrix multiply.
 */
void esMatrixMultiply(ESMatrix *result, ESMatrix *srcA, ESMatrix *srcB)
{
    ESMatrix tmp;
    int i;
    for(i=0; i<4; i++)
    {
        tmp.m[i][0] = (srcA->m[i][0] *srcB->m[0][0]) +
                      (srcA->m[i][1] *srcB->m[1][0]) +
                      (srcA->m[i][2] *srcB->m[2][0]) +
                      (srcA->m[i][3] *srcB->m[3][0]);

        tmp.m[i][1] = (srcA->m[i][0] *srcB->m[0][1]) +
                      (srcA->m[i][1] *srcB->m[1][1]) +
                      (srcA->m[i][2] *srcB->m[2][1]) +
                      (srcA->m[i][3] *srcB->m[3][1]);

        tmp.m[i][2] = (srcA->m[i][0] *srcB->m[0][2]) +
                      (srcA->m[i][1] *srcB->m[1][2]) +
                      (srcA->m[i][2] *srcB->m[2][2]) +
                      (srcA->m[i][3] *srcB->m[3][2]);

        tmp.m[i][3] = (srcA->m[i][0] *srcB->m[0][3]) +
                      (srcA->m[i][1] *srcB->m[1][3]) +
                      (srcA->m[i][2] *srcB->m[2][3]) +
                      (srcA->m[i][3] *srcB->m[3][3]);
    }
    memcpy(result, &tmp, sizeof(ESMatrix));
}
#endif

/*
 * Altia:  esOrtho()
 * Returns 3D identity matrix.
 */
static void esMatrixLoadIdentity(ESMatrix *result)
{
    memset(result, 0x0, sizeof(ESMatrix));
    result->m[0][0] =
    result->m[1][1] =
    result->m[2][2] =
    result->m[3][3] = 1.0f;
}


/*
 * Altia:  esOrtho()
 * Returns orthographic projection matrix.
 */
static void esOrtho(ESMatrix *result, EGL_FLOAT left, EGL_FLOAT right,
             EGL_FLOAT bottom, EGL_FLOAT top,
             EGL_FLOAT nearZ, EGL_FLOAT farZ)
{
    EGL_FLOAT deltaX = right -left;
    EGL_FLOAT deltaY = top -bottom;
    EGL_FLOAT deltaZ = farZ -nearZ;
    ESMatrix ortho;

    if((deltaX == F2FX(0)) || (deltaY == F2FX(0)) || (deltaZ == F2FX(0)))
        return;

    esMatrixLoadIdentity(result);
    esMatrixLoadIdentity(&ortho);

    ortho.m[0][0] = 2.0f /deltaX;
    ortho.m[3][0] = -(right +left) /deltaX;
    ortho.m[1][1] = 2.0f /deltaY;
    ortho.m[3][1] = -(top +bottom) /deltaY;
    ortho.m[2][2] = -2.0f /deltaZ;
    ortho.m[3][2] = -(nearZ +farZ) /deltaZ;
    esMatrixMultiply(result, &ortho, result);
}


static int initLayer(EHwLayer_t hwLayer)
{
    EGLint matchingConfigs;
    GLint linked;
    ESMatrix objXForm;
    unsigned int i = hwLayer;
    int displayWidth;
    int displayHeight;

    /*
     * Altia:  driver_create_layer()
     *
     * EGL / OpenGL ES 2 init (using Vivante API)
     * This matches Altia environment.  We do this instead of using gh_FB_get_driver_by_name().
     */
    sg_layerInfo[i].nativeDisplay = fbGetDisplayByName((char *) DISPLAY, i, NULL);
    if (0 == sg_layerInfo[i].nativeDisplay)
    {
        printf("ERROR:  fbGetDisplayByName(%s, %d) failed.\n", DISPLAY, i);
        return (-1);
    }

    printf("fbGetDisplayByName():  %s:  layer:  %d\n", DISPLAY, i);
    printf("  nativeDisplay(%d):  0x%08X\n", i, sg_layerInfo[i].nativeDisplay);

    /*
     * Retrieve display dimensions directly from the display driver
     */
    fbGetDisplayGeometry(sg_layerInfo[i].nativeDisplay, &displayWidth, &displayHeight);
    printf("    resolution (pixels):  %d x %d\n", displayWidth, displayHeight);

    /*
     * Error check:  do the native display IDs match one another for each layer?
     *
     * :NOTE: MY, 5/4/16 -- Confirmed the native display IDs are different across layers.
     */
//    if (sg_layerInfo[HW_LAYER_BG].nativeDisplay != sg_layerInfo[HW_LAYER_FG].nativeDisplay)
//    {
//        printf("ERROR:  sg_layerInfo[HW_LAYER_BG].nativeDisplay != sg_layerInfo[HW_LAYER_FG].nativeDisplay\n");
//        return (-2);
//    }

    /*
     * EGL / OpenGL ES 2 window creation  (using Vivante API)
     * This matches Altia environment.  Create the windows on the specified native
     * display @ offset 0,0.
     */
    sg_layerInfo[i].nativeWindow = fbCreateWindow(sg_layerInfo[i].nativeDisplay, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    if (0 == sg_layerInfo[i].nativeWindow)
    {
        printf("ERROR:  fbCreateWindow() failed for:  (%d, %d),  size: %d x %d\n", i, sg_layerInfo[i].nativeDisplay, DISPLAY_WIDTH, DISPLAY_HEIGHT);
        return (-3);
    }

    printf("  fbCreateWindow():  success\n");

    /*
     * :NOTE: MY, Per Altia code comments, this re-setting of the window size is
     * necessary to work around an Integrity BSP issue wrt foreground layer size
     * init & config when it was created.
     *
     * :NOTE: MY, 5/4/16 -- Ran a test on Altia gen'ed code w/ all fbSetDisplayProperty() APIs that were returning error codes commented out; no change in erroneous behavior.
     */
//    {
//        FBRect rect;
//
//        rect.dx = 0;
//        rect.dy = 0;
//        rect.Width = DISPLAY_WIDTH;
//        rect.Height = DISPLAY_HEIGHT;
//
//        if (fbSetDisplayProperty(sg_layerInfo[i].nativeDisplay, FB_PROPERTY_WINDOW, (Address) &rect))
//        {
//            printf("ERROR:  fbSetDisplayProperty() failed for:  (%d, %d),  size: %d x %d\n", i, sg_layerInfo[i].nativeDisplay, DISPLAY_WIDTH, DISPLAY_HEIGHT);
//            return (-4);
//        }
//    }
//
//    printf("  fbSetDisplayProperty(..., FB_PROPERTY_WINDOW, ...):  success\n");
//    printf("  fbSetDisplayProperty(..., FB_PROPERTY_WINDOW, ...):  SKIPPED\n");

    /*
     * Standard EGL init sequence
     *
     * :NOTE:
     * Since the display IDs have been verified to be the same by the time we get
     * to the EGL init sequence, we just use the hard-coded background layer ID.
     * This mimics the Altia code closely.
     *
     * For eglInitialize(), we don't care about Open GL ES version numbers
     */
    sg_layerInfo[i].eglDisplay = eglGetDisplay(sg_layerInfo[i].nativeDisplay);
    if (NULL == sg_layerInfo[i].eglDisplay)
    {
        CHECK_EGL_ERROR();
        printf("ERROR:  eglGetDisplay(%d) failed!\n", i);
        return (-5);
    }

    if (0 == eglInitialize(sg_layerInfo[i].eglDisplay, NULL, NULL))
    {
        CHECK_EGL_ERROR();
        printf("ERROR:  eglInitialize(%d) failed!\n", i);
        return (-6);
    }

    if (0 == eglChooseConfig(sg_layerInfo[i].eglDisplay, configAttributes, &sg_layerInfo[i].eglConfig, 1, &matchingConfigs))
    {
        CHECK_EGL_ERROR();
        printf("ERROR:  eglChooseConfig(%d) failed!\n", i);
        return (-7);
    }

    printf("  eglGetDisplay(), eglInitialize(), eglChooseConfig():  success\n");

//	/* TRY THIS CODE TO FIX ERROR */
//    sg_layerInfo[i].nativeWindow = fbCreateWindow(sg_layerInfo[i].nativeDisplay, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);
//    if (sg_layerInfo[i].nativeWindow == NULL)
//    {
//        printf("%s", "fbCreateWindow() failed.\n");
//        return (-8);
//    }
//	printf("  fbCreateWindow(): success\n");

    /*
     * Hide the foreground layer
     * Show the background layer since it's not allowed to be invisible
     * Both layers are shown later after the full init sequence is done
     *
     * :NOTE: MY, 5/4/16 -- fbSetDisplayProperty() returns an error code here; skipping this
     */
//    if (fbSetDisplayProperty(sg_layerInfo[HW_LAYER_FG].nativeDisplay, FB_PROPERTY_VISIBLE, 0))
//    {
//        printf("ERROR:  fbSetDisplayProperty(sg_layerInfo[HW_LAYER_FG].nativeDisplay, FB_PROPERTY_VISIBLE, 0)\n");
//        return (-8);
//    }
//
//    if (fbSetDisplayProperty(sg_layerInfo[HW_LAYER_BG].nativeDisplay, FB_PROPERTY_VISIBLE, 1))
//    {
//        printf("ERROR:  fbSetDisplayProperty(sg_layerInfo[HW_LAYER_BG].nativeDisplay, FB_PROPERTY_VISIBLE, 1)\n");
//        return (-8);
//    }
//
//    printf("%s:  fbSetDisplayProperty(..., FB_PROPERTY_VISIBLE, ...) calls:  success\n", DISPLAY);
//    printf("  fbSetDisplayProperty(..., FB_PROPERTY_VISIBLE, ...):  SKIPPED\n");

    /*
     * Get an EGL surface to draw on
     */
    sg_layerInfo[i].eglSurface = eglCreateWindowSurface(sg_layerInfo[i].eglDisplay, sg_layerInfo[i].eglConfig, sg_layerInfo[i].nativeWindow, NULL);
    if (EGL_NO_SURFACE == sg_layerInfo[i].eglSurface)
    {
        CHECK_EGL_ERROR();
        printf("ERROR:  eglCreateWindowSurface(%d) failed:  EGL_NO_SURFACE\n", i);
        return (-9);
    }

    printf("  eglCreateWindowSurface():  success\n");

    /*
     * Create EGL contexts
     */
    sg_layerInfo[i].eglContext = eglCreateContext(sg_layerInfo[i].eglDisplay, sg_layerInfo[i].eglConfig, EGL_NO_CONTEXT, attribListContext);

    if (EGL_NO_CONTEXT == sg_layerInfo[i].eglContext)
    {
        CHECK_EGL_ERROR();
        printf("ERROR:  eglCreateContext(%d) failed!\n", i);
        return (-9);
    }

    printf("  eglCreateContext():  success\n");

    /*
     * Activate the context for rendering
     */
    eglMakeCurrent(sg_layerInfo[i].eglDisplay, sg_layerInfo[i].eglSurface, sg_layerInfo[i].eglSurface, sg_layerInfo[i].eglContext);
    eglSwapInterval(sg_layerInfo[i].eglDisplay, 0);
    printf("  eglContext(%d) = 0x%08X\n", i, sg_layerInfo[i].eglContext);

    eglBindAPI(EGL_OPENGL_ES_API);

    CHECK_EGL_ERROR();

    /*
     * Init OpenGL ES 2 vertex and fragment shaders
     */
    sg_layerInfo[i].vShader = loadShader(GL_VERTEX_SHADER, &defVShader);
    sg_layerInfo[i].fShader = loadShader(GL_FRAGMENT_SHADER, &defFShader);

    if ((0 == sg_layerInfo[i].vShader) || (0 == sg_layerInfo[i].fShader))
    {
        printf("ERROR:  %s:  loadShader(%d) = %d (vertex), %d (fragment)\n", DISPLAY, i, sg_layerInfo[i].vShader, sg_layerInfo[i].fShader);
        return (-10);
    }

    /*
     * Create shader program object and attach shaders to it
     */
    sg_layerInfo[i].shaderProgObj = glCreateProgram();
    glAttachShader(sg_layerInfo[i].shaderProgObj, sg_layerInfo[i].vShader);
    glAttachShader(sg_layerInfo[i].shaderProgObj, sg_layerInfo[i].fShader);

    /*
     * Bind attributes variables to their index locations
     */
    glBindAttribLocation(sg_layerInfo[i].shaderProgObj, SHDR_TEXCOORD_IDX, SHDR_TEXCOORD_NAME);
    glBindAttribLocation(sg_layerInfo[i].shaderProgObj, SHDR_POSITION_IDX, SHDR_POSITION_NAME);
    glBindAttribLocation(sg_layerInfo[i].shaderProgObj, SHDR_COLOR_IDX,    SHDR_COLOR_NAME);

    /*
     * Link the shaders and check for errors
     */
    glLinkProgram(sg_layerInfo[i].shaderProgObj);
    glGetProgramiv(sg_layerInfo[i].shaderProgObj, GL_LINK_STATUS, &linked);
    if (0 == linked)
    {
        printf("ERROR:  shader link failure(%d)\n", i);
        glDeleteShader(sg_layerInfo[i].vShader);
        glDeleteShader(sg_layerInfo[i].fShader);
        glDeleteProgram(sg_layerInfo[i].shaderProgObj);
        return (-11);
    }

    /*
     * Finally use the program object
     */
    glUseProgram(sg_layerInfo[i].shaderProgObj);

    CHECK_GL_ERROR();

    printf("  shaders init:  success\n");
    printf("  shaderProgObj[%d]:  0x%08X\n", i, sg_layerInfo[i].shaderProgObj);

    /*
     * Setup the OpenGL context
     */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glViewport(0, 0, (GLsizei) DISPLAY_WIDTH, (GLsizei) DISPLAY_HEIGHT);
    glDepthRangef(F2VT(0.0f), F2VT(1.0f));

    /*
     * Set the PMVMatrix value to orthographic
     */
    esOrtho(&(sg_layerInfo[i].pmvMatrix),
            F2FX(0),                    /* left */
            F2FX(DISPLAY_WIDTH),        /* right */
            F2FX(DISPLAY_HEIGHT -1),    /* bottom */
            F2FX(-1),                   /* top */
            F2FX(-1),                   /* nearZ */
            F2FX(1) );                  /* farZ */
    glUniformMatrix4fv( glGetUniformLocation(sg_layerInfo[i].shaderProgObj,SHDR_PMVMATRIX_NAME),
                        1,
                        GL_FALSE,
                        (const GLfloat*) sg_layerInfo[i].pmvMatrix.m );

    /*
     * Set the object transform matrix to identity
     */
    esMatrixLoadIdentity(&objXForm);
    glUniformMatrix4fv( glGetUniformLocation(sg_layerInfo[i].shaderProgObj, SHDR_OBJXFORM_NAME),
                        1,
                        GL_FALSE,
                        (const GLfloat*) objXForm.m );

    glScissor(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    glEnable(GL_SCISSOR_TEST);

    eglSwapBuffers(sg_layerInfo[i].eglDisplay, sg_layerInfo[i].eglSurface);

    printf("  Open GL ES 2 context init:  success\n");

    /*
     * Altia:  driver_enable_layer()
     *
     * Enable the foreground layer
     *
     * :NOTE: MY, 5/4/16 -- fbSetDisplayProperty() returns an error code here; skipping this
     */
//    if (fbSetDisplayProperty(sg_layerInfo[HW_LAYER_FG].nativeDisplay, FB_PROPERTY_VISIBLE, 1))
//    {
//        printf("ERROR:  fbSetDisplayProperty(sg_layerInfo[HW_LAYER_FG].nativeDisplay, FB_PROPERTY_VISIBLE, 1)\n");
//        return (-12);
//    }
//
//    printf("%s:  Foreground layer now visible\n", DISPLAY);
//    printf("  Foreground layer now visible:  SKIPPED\n");

    CHECK_GL_ERROR();

    return (0);
}


static int initEverything(void)
{
    int retVal = 0;
    EHwLayer_t hwLayer;

    /*
     * The Altia target primarily uses a mixture of Vivante APIs, EGL, and OpenGL
     * ES 2.0 APIs.  It typically avoids the GHS FB API due to a number of bugs
     * that were preventing it from working properly on the i.MX 6 SABRE platform.
     */

    /*
     * Altia:  driver_open()
     *
     * i.MX 6 Vivante EGL initialization
     */

    VivanteInit();

    for (hwLayer = HW_LAYER_BG; hwLayer < HW_LAYER_MAX; hwLayer++)
    {
        retVal = initLayer(hwLayer);
        if (retVal)
        {
            printf("ERROR:  initLayer(%d) failed:  %d\n", hwLayer, retVal);
            break;
        }
    }

    return (retVal);
}


static int drawRectangleWithAlpha(SRectInfo_t * pRectInfo, SColorInfo_t * pColorInfo, GLfloat alpha)
{
    int retVal = 0;
    EGL_POS data[8];

    CHECK_GL_ERROR();

    /*
     * Altia:  egl_Rectangle()
     * Construct a strip of 2 triangles to create the filled rectangle
     */
    data[0] = (GLfloat) pRectInfo->begin.x  + EGL_RECT_X_OFFSET;
    data[1] = (GLfloat) pRectInfo->end.y    + EGL_RECT_Y_OFFSET;

    data[2] = (GLfloat) pRectInfo->end.x    + EGL_RECT_X_OFFSET;
    data[3] = (GLfloat) pRectInfo->end.y    + EGL_RECT_Y_OFFSET;

    data[4] = (GLfloat) pRectInfo->begin.x  + EGL_RECT_X_OFFSET;
    data[5] = (GLfloat) pRectInfo->begin.y  + EGL_RECT_Y_OFFSET;

    data[6] = (GLfloat) pRectInfo->end.x    + EGL_RECT_X_OFFSET;
    data[7] = (GLfloat) pRectInfo->begin.y  + EGL_RECT_Y_OFFSET;

    /*
     * Set the shader a_color and a_position and draw the triangles
     * glVertexAttrib4f(SHDR_COLOR_IDX, FX2F(red), FX2F(green), FX2F(blue), alpha);
     */
    glVertexAttrib4f(SHDR_COLOR_IDX, pColorInfo->red, pColorInfo->green, pColorInfo->blue, alpha);
    glVertexAttribPointer(SHDR_POSITION_IDX, 2, GL_FLOAT, GL_FALSE, 0, data);
    glEnableVertexAttribArray(SHDR_POSITION_IDX);
    glDrawArrays(ALTIA_RECTANGLE_TRIANGLE_TYPE, 0, 4);

    CHECK_GL_ERROR();

    return (retVal);
}


static int drawRectangle(SRectInfo_t * pRectInfo, SColorInfo_t * pColorInfo)
{
    return (drawRectangleWithAlpha(pRectInfo, pColorInfo, 1.0f));
}


static int clearFrame(EHwLayer_t layer)
{
    int retVal = 0;
    SRectInfo_t clearScreenRectInfo;
    SColorInfo_t * pColorInfo;

    /*
     * :NOTE: Frame clearing
     * Keep things simple and just clear out a larger rectangle than all the potential
     * movements of the rectangle on the fgnd / bgnd
     */
    switch (layer)
    {
        case HW_LAYER_BG:
        {
            clearScreenRectInfo = sg_bgRectInfo;
            clearScreenRectInfo.begin.x = (GLfloat) 0.0f;
            clearScreenRectInfo.end.x = (GLfloat) DISPLAY_WIDTH;
            pColorInfo = &sg_blackColor;
            break;
        }

        case HW_LAYER_FG:
        {
            clearScreenRectInfo = sg_fgRectInfo;
            clearScreenRectInfo.begin.x = (GLfloat) 0.0f;
            clearScreenRectInfo.end.x = (GLfloat) DISPLAY_WIDTH;
            pColorInfo = &sg_blackColor;
            break;
        }

        default:
        {
            /*
             * Clear the whole screen and set it to red if a bogus layer param was
             * passed in
             */
            printf("WARNING:  unexpected layer case:  %d\n", layer);
            clearScreenRectInfo.begin.x = 0.0f;
            clearScreenRectInfo.begin.y = 0.0f;
            clearScreenRectInfo.end.x = (GLfloat) DISPLAY_WIDTH;
            clearScreenRectInfo.end.y = (GLfloat) DISPLAY_HEIGHT;
            pColorInfo = &sg_redColor;
            break;
        }
    }

    retVal = drawRectangleWithAlpha(&clearScreenRectInfo, pColorInfo, 1.0f);

    return (retVal);
}


static void calcTranslationValues(int loopCount, SCoord_t * pTranslationValues)
{
    pTranslationValues->x = (GLfloat) (loopCount % (DISPLAY_WIDTH - (OBJ_WIDTH * 3)));
    pTranslationValues->y = 0.0f;
}


static int preDraw(EHwLayer_t layer)
{
    int retVal = 0;
    ESMatrix objXForm;

    /*
     * Altia:  driver_startGraphics()
     *
     * Attach the EGL rendering context to the EGL surface
     */
    if(EGL_FALSE == eglMakeCurrent(sg_layerInfo[layer].eglDisplay, sg_layerInfo[layer].eglSurface, sg_layerInfo[layer].eglSurface, sg_layerInfo[layer].eglContext))
    {
        printf("ERROR:  eglMakeCurrent() failed!\n");
        return (-1);
    }

    CHECK_EGL_ERROR();
    CHECK_GL_ERROR();

    glUseProgram(sg_layerInfo[layer].shaderProgObj);

    CHECK_GL_ERROR();

    glUniformMatrix4fv( glGetUniformLocation(sg_layerInfo[layer].shaderProgObj, SHDR_PMVMATRIX_NAME),
                        1,
                        GL_FALSE,
                        (const GLfloat*) sg_layerInfo[layer].pmvMatrix.m );

    CHECK_GL_ERROR();

    /*
     * Set the object transform matrix to identity
     */
    esMatrixLoadIdentity(&objXForm);
    glUniformMatrix4fv( glGetUniformLocation(sg_layerInfo[layer].shaderProgObj, SHDR_OBJXFORM_NAME),
                        1,
                        GL_FALSE,
                        (const GLfloat*) objXForm.m );

    CHECK_GL_ERROR();

    /*
     * Clear the previous frame's data before drawing the next frame
     */
    clearFrame(layer);

    return (retVal);
}


static int postDraw(EHwLayer_t layer)
{
    /*
     * Nothing to do here
     */
    return (0);
}


/**
 * @brief  Swap all the framebuffers at the same time
 */
static int swapBuffers(void)
{
    EHwLayer_t i;

    /*
     * Altia:  driver_swapBuffers()
     *
     * Attach the EGL rendering context to the EGL surface prior to swapping buffers
     */
    for (i = HW_LAYER_BG; i < HW_LAYER_MAX; i++)
    {
        if (EGL_FALSE == eglMakeCurrent(sg_layerInfo[i].eglDisplay, sg_layerInfo[i].eglSurface, sg_layerInfo[i].eglSurface, sg_layerInfo[i].eglContext))
        {
            printf("ERROR:  eglMakeCurrent(%d) failed!\n", i);
            return (-1);
        }

        /*
         * Swap the EGL surface
         */
        if (EGL_FALSE == eglSwapBuffers(sg_layerInfo[i].eglDisplay, sg_layerInfo[i].eglSurface))
        {
            printf("ERROR:  eglSwapBuffers(%d) failed!\n", i);
            return (-2);
        }
    }

    return (0);
}


static void translateRectangle(SRectInfo_t * pDstRectInfo, SRectInfo_t * pSrcRectInfo, GLfloat x, GLfloat y)
{
    pDstRectInfo->begin.x = pSrcRectInfo->begin.x + x;
    pDstRectInfo->begin.y = pSrcRectInfo->begin.y + y;
    pDstRectInfo->end.x = pSrcRectInfo->end.x + x;
    pDstRectInfo->end.y = pSrcRectInfo->end.y + y;
}


/******************************************************************************
 * APIs
 ******************************************************************************/
int StartVivanteDemo(void)
{
    int retVal = 0;
    EHwLayer_t i;
    int loopCount = 0;
    SColorInfo_t * pColorInfo;
    SRectInfo_t tmpRectInfo;
    SRectInfo_t * pRectInfo = &tmpRectInfo;
    SCoord_t translationValues;

    /* Wait for file system to be ready before proceeding */
    WaitForFileSystemInitialization();

	printf("************************************************\n");    
    printf(" Yazaki graphics test application v %d.%d\n", demo_ver, demo_rev);
	printf(" Build date: %s\n", __DATE__ );
    printf(" DISPLAY_WIDTH: %d\n", DISPLAY_WIDTH);
    printf(" DISPLAY_HEIGHT: %d\n", DISPLAY_HEIGHT);
	printf("************************************************\n");

    retVal = initEverything();
    if (retVal)
    {
        printf("ERROR:  initEverything()\n");
        goto mainReturn;
    }

    /*
     * Loop forever drawing rectangles
     */
    while (1)
    {
        /*
         * Sanity check that we're still executing
         */
        if (0 == (loopCount % 100))
        {
            printf("loop count:  %d\n", loopCount);
        }

        for (i = HW_LAYER_BG; i < HW_LAYER_MAX; i++)
        {
            /*
             * Set the current rendering surface to the desired layer
             * Then draw a rectangle on said layer
             * Do any post processing after each draw
             */
            retVal = preDraw(i);
            if (retVal)
            {
                printf("ERROR:  preDraw():  %d\n", retVal);
                break;
            }

            switch (i)
            {
                case HW_LAYER_BG:
                {
#if defined(TURN_ON_ANIMATED_OBJECTS)
                    calcTranslationValues(loopCount, &translationValues);
                    translateRectangle(pRectInfo, &sg_bgRectInfo, translationValues.x, translationValues.y);
#else
                    pRectInfo = &sg_bgRectInfo;
#endif  // #if defined(TURN_ON_ANIMATED_OBJECTS)
                    pColorInfo = &sg_blueColor;
                    break;
                }

                case HW_LAYER_FG:
                {
#if defined(TURN_ON_ANIMATED_OBJECTS)
                    calcTranslationValues(loopCount, &translationValues);
                    translateRectangle(pRectInfo, &sg_fgRectInfo, (translationValues.x * -1.0f), translationValues.y);
#else
                    pRectInfo = &sg_fgRectInfo;
#endif  // #if defined(TURN_ON_ANIMATED_OBJECTS)
                    pColorInfo = &sg_greenColor;
                    break;
                }

                default:
                {
                    printf("WARNING:  unexpected layer case:  %d\n", i);
                    pRectInfo = &sg_errorRectInfo;
                    pColorInfo = &sg_redColor;
                    break;
                }
            }

            retVal = drawRectangle(pRectInfo, pColorInfo);
            if (retVal)
            {
                printf("ERROR:  drawRectangle():  %d\n", retVal);
                break;
            }

            postDraw(i);
        }

        retVal = swapBuffers();
        if (retVal)
        {
            printf("ERROR:  swapBuffers():  %d\n", retVal);
            break;
        }

        loopCount++;
    }  // END -- while(1)

mainReturn:

    return (retVal);
}
