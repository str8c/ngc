#include "../main.h"

static void debug_infolog(GLuint shader, const char *data)
{
    #ifdef DEBUG
    GLint infologsize;
    char* infolog;

    debug("shader loading failed:\n%s\n", data);
    infologsize = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infologsize);
    if(infologsize) {
        infolog = malloc(infologsize);
        glGetShaderInfoLog(shader, infologsize, NULL, (void*)infolog);
        debug("Infolog: %s\n", infolog);
        free(infolog);
    }
    #endif
}

static bool compile_shader(GLuint shader, const char *data)
{
    GLint status;

    glShaderSource(shader, 1, &data, NULL);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if(!status) {
        debug_infolog(shader, data);
        return 0;
    }

    return 1;
}

bool ngc_shader_load(SHADER *dest, const char *vertex_shader, const char *fragment_shader)
{
    GLuint prog, vertshader, fragshader;
    GLint status;

    vertshader = glCreateShader(GL_VERTEX_SHADER);
    if(!vertshader) {
        debug("glCreateShader() failed (vert)\n");
        return 0;
    }

    if(!compile_shader(vertshader, vertex_shader)) {
        //cleanup
        return 0;
    }

    fragshader = glCreateShader(GL_FRAGMENT_SHADER);
    if(!fragshader) {
        //cleanup
        return 0;
    }

    if(!compile_shader(fragshader, fragment_shader)) {
        //cleanup
        return 0;
    }

    prog = glCreateProgram();
    glAttachShader(prog, vertshader);
    glAttachShader(prog, fragshader);
    glBindAttribLocation(prog, 0, "pos");
    glBindAttribLocation(prog, 1, "tex");

    glLinkProgram(prog);
    glGetProgramiv(prog, GL_LINK_STATUS, &status);
    if(!status) {
        debug_infolog(prog, "glLinkProgram");
        return 0;
    }

    glUseProgram(prog);

    dest->prog = prog;
    dest->matrix = glGetUniformLocation(prog, "matrix");
    dest->k = glGetUniformLocation(prog, "k");
    dest->samp = glGetUniformLocation(prog, "samp");
    return 1;
}
