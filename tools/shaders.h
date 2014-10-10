typedef struct {
    GLuint prog;
    GLint matrix, k, samp;
} SHADER;

/* loads a shader from a fragment and vertex shader
    return value
        0 on failure
        1 on success (dest_size shaders are loaded)
    notes
        binds 2 standard attributes to 0 and 1: pos, tex
        loads 3 standard uniforms: matrix, k, samp
        sets samp to 0
 */
_Bool ngc_shader_load(SHADER *dest, const char *vertex_shader, const char *fragment_shader);
