//========================================================================
// Version information dumper
// Copyright (c) Camilla Berglund <elmindreda@elmindreda.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================
//
// This test is a pale imitation of glxinfo(1), except not really
//
// It dumps GLFW and OpenGL version information
//
//========================================================================

#include <GL/glfw3.h>
#include <GL/glext.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "getopt.h"

#ifdef _MSC_VER
#define strcasecmp(x, y) _stricmp(x, y)
#endif

#define API_OPENGL          "gl"
#define API_OPENGL_ES       "es"

#define PROFILE_NAME_CORE   "core"
#define PROFILE_NAME_COMPAT "compat"

#define STRATEGY_NAME_NONE "none"
#define STRATEGY_NAME_LOSE "lose"

static void usage(void)
{
    printf("Usage: glfwinfo [-h] [-a API] [-m MAJOR] [-n MINOR] [-d] [-l] [-f] [-p PROFILE] [-r STRATEGY]\n");
    printf("available APIs: " API_OPENGL " " API_OPENGL_ES "\n");
    printf("available profiles: " PROFILE_NAME_CORE " " PROFILE_NAME_COMPAT "\n");
    printf("available strategies: " STRATEGY_NAME_NONE " " STRATEGY_NAME_LOSE "\n");
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s in %s\n", glfwErrorString(error), description);
}

static const char* get_client_api_name(int api)
{
    if (api == GLFW_OPENGL_API)
        return "OpenGL";
    else if (api == GLFW_OPENGL_ES_API)
        return "OpenGL ES";

    return "Unknown API";
}

static const char* get_glfw_profile_name(int profile)
{
    if (profile == GLFW_OPENGL_COMPAT_PROFILE)
        return "compatibility";
    else if (profile == GLFW_OPENGL_CORE_PROFILE)
        return "core";

    return "unknown";
}

static const char* get_profile_name(GLint mask)
{
    if (mask & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT)
        return "compatibility";
    if (mask & GL_CONTEXT_CORE_PROFILE_BIT)
        return "core";

    return "unknown";
}

static void list_extensions(int api, int major, int minor)
{
    int i;
    GLint count;
    const GLubyte* extensions;

    printf("%s context supported extensions:\n", get_client_api_name(api));

    if (api == GLFW_OPENGL_API && major > 2)
    {
        PFNGLGETSTRINGIPROC glGetStringi = (PFNGLGETSTRINGIPROC) glfwGetProcAddress("glGetStringi");
        if (!glGetStringi)
            exit(EXIT_FAILURE);

        glGetIntegerv(GL_NUM_EXTENSIONS, &count);

        for (i = 0;  i < count;  i++)
            puts((const char*) glGetStringi(GL_EXTENSIONS, i));
    }
    else
    {
        extensions = glGetString(GL_EXTENSIONS);
        while (*extensions != '\0')
        {
            if (*extensions == ' ')
                putchar('\n');
            else
                putchar(*extensions);

            extensions++;
        }
    }

    putchar('\n');
}

int main(int argc, char** argv)
{
    int ch, api = 0, profile = 0, strategy = 0, major = 1, minor = 0, revision;
    GLboolean debug = GL_FALSE, forward = GL_FALSE, list = GL_FALSE;
    GLint flags, mask;
    GLFWwindow window;

    while ((ch = getopt(argc, argv, "a:dfhlm:n:p:r:")) != -1)
    {
        switch (ch)
        {
            case 'a':
                if (strcasecmp(optarg, API_OPENGL) == 0)
                    api = GLFW_OPENGL_API;
                else if (strcasecmp(optarg, API_OPENGL_ES) == 0)
                    api = GLFW_OPENGL_ES_API;
                else
                {
                    usage();
                    exit(EXIT_FAILURE);
                }
            case 'd':
                debug = GL_TRUE;
                break;
            case 'f':
                forward = GL_TRUE;
                break;
            case 'h':
                usage();
                exit(EXIT_SUCCESS);
            case 'l':
                list = GL_TRUE;
                break;
            case 'm':
                major = atoi(optarg);
                break;
            case 'n':
                minor = atoi(optarg);
                break;
            case 'p':
                if (strcasecmp(optarg, PROFILE_NAME_CORE) == 0)
                    profile = GLFW_OPENGL_CORE_PROFILE;
                else if (strcasecmp(optarg, PROFILE_NAME_COMPAT) == 0)
                    profile = GLFW_OPENGL_COMPAT_PROFILE;
                else
                {
                    usage();
                    exit(EXIT_FAILURE);
                }
                break;
            case 'r':
                if (strcasecmp(optarg, STRATEGY_NAME_NONE) == 0)
                    strategy = GLFW_OPENGL_NO_RESET_NOTIFICATION;
                else if (strcasecmp(optarg, STRATEGY_NAME_LOSE) == 0)
                    strategy = GLFW_OPENGL_LOSE_CONTEXT_ON_RESET;
                else
                {
                    usage();
                    exit(EXIT_FAILURE);
                }
                break;
            default:
                usage();
                exit(EXIT_FAILURE);
        }
    }

    argc -= optind;
    argv += optind;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW: %s\n", glfwErrorString(glfwGetError()));
        exit(EXIT_FAILURE);
    }

    if (major != 1 || minor != 0)
    {
        glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, major);
        glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, minor);
    }

    if (api != 0)
        glfwOpenWindowHint(GLFW_CLIENT_API, api);

    if (debug)
        glfwOpenWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    if (forward)
        glfwOpenWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    if (profile != 0)
        glfwOpenWindowHint(GLFW_OPENGL_PROFILE, profile);

    if (strategy)
        glfwOpenWindowHint(GLFW_OPENGL_ROBUSTNESS, strategy);

    // We assume here that we stand a better chance of success by leaving all
    // possible details of pixel format selection to GLFW

    window = glfwOpenWindow(0, 0, GLFW_WINDOWED, "Version", NULL);
    if (!window)
        exit(EXIT_FAILURE);

    // Report GLFW version

    glfwGetVersion(&major, &minor, &revision);

    printf("GLFW header version: %u.%u.%u\n",
           GLFW_VERSION_MAJOR,
           GLFW_VERSION_MINOR,
           GLFW_VERSION_REVISION);

    printf("GLFW library version: %u.%u.%u\n", major, minor, revision);

    if (major != GLFW_VERSION_MAJOR ||
        minor != GLFW_VERSION_MINOR ||
        revision != GLFW_VERSION_REVISION)
    {
        printf("*** WARNING: GLFW version mismatch! ***\n");
    }

    printf("GLFW library version string: \"%s\"\n", glfwGetVersionString());

    // Report client API version

    api = glfwGetWindowParam(window, GLFW_CLIENT_API);
    major = glfwGetWindowParam(window, GLFW_OPENGL_VERSION_MAJOR);
    minor = glfwGetWindowParam(window, GLFW_OPENGL_VERSION_MINOR);
    revision = glfwGetWindowParam(window, GLFW_OPENGL_REVISION);

    printf("%s context version string: \"%s\"\n",
           get_client_api_name(api),
           glGetString(GL_VERSION));

    printf("%s context version parsed by GLFW: %u.%u.%u\n",
           get_client_api_name(api),
           major, minor, revision);

    // Report client API context properties

    if (api == GLFW_OPENGL_API)
    {
        if (major >= 3)
        {
            glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
            printf("%s context flags:", get_client_api_name(api));

            if (flags & GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT)
                puts(" forward-compatible");
            else
                puts(" none");

            printf("%s forward-compatible flag parsed by GLFW: %s\n",
                   get_client_api_name(api),
                   glfwGetWindowParam(window, GLFW_OPENGL_FORWARD_COMPAT) ? "true" : "false");
        }

        if (major > 3 || (major == 3 && minor >= 2))
        {
            glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &mask);
            printf("%s profile mask: %s (0x%08x)\n",
                   get_client_api_name(api),
                   get_profile_name(mask), mask);

            printf("%s profile parsed by GLFW: %s\n",
                   get_client_api_name(api),
                   get_glfw_profile_name(glfwGetWindowParam(window, GLFW_OPENGL_PROFILE)));
        }
    }

    printf("%s context renderer string: \"%s\"\n",
           get_client_api_name(api),
           glGetString(GL_RENDERER));
    printf("%s context vendor string: \"%s\"\n",
           get_client_api_name(api),
           glGetString(GL_VENDOR));

    if (major > 1)
    {
        printf("%s context shading language version: \"%s\"\n",
               get_client_api_name(api),
               glGetString(GL_SHADING_LANGUAGE_VERSION));
    }

    // Report client API extensions
    if (list)
        list_extensions(api, major, minor);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

