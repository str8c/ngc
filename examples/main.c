#include "../main.h"

/* build with
gcc main.c ../main.c -DSHADERS -DXLIB -lGL -lX11 -pthread -Wall
*/

SHADER shader;

typedef struct
{
    int16_t x, y;
    uint16_t tx, ty;
}VERTEX2D;

typedef struct
{
    VERTEX2D vertex[4];
}QUAD2D;

static const char
vertex_shader[] =
    "uniform vec4 matrix;"
    "attribute vec2 pos;"
    "attribute vec2 tex;"
    "varying vec2 x;"
    "void main(){"
        "x = tex / 32768.0;"
        "gl_Position = vec4((pos + matrix.xy) * matrix.zw, 0.0, 1.0);"
    "}",
fragment_shader[] =
#ifdef OPENGL_ES
    "precision mediump float;"
#endif
    "uniform sampler2D samp;"
    "uniform vec4 k;"
    "varying vec2 x;"
    "void main(){"
        "gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);/*texture2D(samp, x) * k;*/"
    "}";

void do_preinit(void)
{

}

_Bool do_init(uint16_t width, uint16_t height)
{
    if(!ngc_shader_load(&shader, vertex_shader, fragment_shader)) {
        return 0;
    }

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    return 1;
}

void do_cleanup(void)
{

}

void do_frame(void)
{
    ngc_redraw();
}

void do_draw(void)
{
    static uint8_t b;
    QUAD2D quad = {{
        {0, 0, 0, 0},
        {0, 1, 0, 0},
        {1, 1, 0, 0},
        {1, 0, 0, 0},
    }};

    float color[4] = {1.0, 1.0, 1.0, 1.0};
    float matrix[4] = {0.0, 0.0, 1.0, 1.0};

    glUseProgram(0);

    glColor3f(1.0, 1.0, (float)b / 255.0);
    glBegin(GL_QUADS);
    glVertex2f(-1.0, -1.0);
    glVertex2f(-1.0, 1.0);
    glVertex2f(1.0, 1.0);
    glVertex2f(1.0, -1.0);
    glEnd();

    glUseProgram(shader.prog);
    glUniform4fv(shader.k, 1, color);
    glUniform4fv(shader.matrix, 1, matrix);
    //glUniform2fv(shader.matrix, 1, texscale);

    glVertexAttribPointer(0, 2, GL_SHORT, GL_FALSE, sizeof(VERTEX2D), &quad);
    glVertexAttribPointer(1, 2, GL_UNSIGNED_SHORT, GL_FALSE, sizeof(VERTEX2D), &quad.vertex[0].tx);
    glDrawArrays(GL_QUADS, 0, 4);

    debug("%u\n", b++);
}
