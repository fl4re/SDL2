SDL_PROC(void, glBindTexture, (GLenum, GLuint))
SDL_PROC(void, glBlendFunc, (GLenum, GLenum))
SDL_PROC(void, glClear, (GLbitfield))
SDL_PROC(void, glClearColor, (GLclampf, GLclampf, GLclampf, GLclampf))
SDL_PROC(void, glColor4f, (GLfloat, GLfloat, GLfloat, GLfloat))
SDL_PROC(void, glDeleteTextures, (GLsizei, const GLuint *))
SDL_PROC(void, glDisable, (GLenum))
SDL_PROC(void, glDisableClientState, (GLenum array))
SDL_PROC(void, glDrawArrays, (GLenum, GLint, GLsizei))
SDL_PROC(void, glDrawTexiOES, (GLint, GLint, GLint, GLint, GLint))
SDL_PROC(void, glEnable, (GLenum))
SDL_PROC(void, glEnableClientState, (GLenum))
SDL_PROC(void, glFinish, (void))
SDL_PROC(void, glGenTextures, (GLsizei, GLuint *))
SDL_PROC(GLenum, glGetError, (void))
SDL_PROC(void, glGetIntegerv, (GLenum, GLint *))
SDL_PROC(void, glLoadIdentity, (void))
SDL_PROC(void, glMatrixMode, (GLenum))
SDL_PROC(void, glOrthof, (GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat))
SDL_PROC(void, glPixelStorei, (GLenum, GLint))
SDL_PROC(void, glReadPixels, (GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, GLvoid*))
SDL_PROC(void, glTexCoordPointer, (GLint, GLenum, GLsizei, const GLvoid *))
SDL_PROC(void, glTexEnvf, (GLenum, GLenum, GLfloat))
SDL_PROC(void, glTexImage2D, (GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid *))
SDL_PROC(void, glTexParameteri, (GLenum, GLenum, GLint))
SDL_PROC(void, glTexParameteriv, (GLenum, GLenum, const GLint *))
SDL_PROC(void, glTexSubImage2D, (GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *))
SDL_PROC(void, glVertexPointer, (GLint, GLenum, GLsizei, const GLvoid *))
SDL_PROC(void, glViewport, (GLint, GLint, GLsizei, GLsizei))
SDL_PROC(void, glBindFramebufferOES, (GLenum, GLuint))
SDL_PROC(void, glFramebufferTexture2DOES, (GLenum, GLenum, GLenum, GLuint, GLint))
SDL_PROC(GLenum, glCheckFramebufferStatusOES, (GLenum))
SDL_PROC(void, glPushMatrix, (void))
SDL_PROC(void, glTranslatef, (GLfloat, GLfloat, GLfloat))
SDL_PROC(void, glRotatef, (GLfloat, GLfloat, GLfloat, GLfloat))
SDL_PROC(void, glPopMatrix, (void))
SDL_PROC(void, glDeleteFramebuffersOES, (GLsizei, const GLuint*))

/* vi: set ts=4 sw=4 expandtab: */
