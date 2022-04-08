/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <gl/glew.h>
#include "glwindow.h"
#include <QImage>
#include <QOpenGLContext>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QTimer>
#include <fstream>
#include <QFile>

namespace {

static const char *const vertexShaderSrc {
#include "vertex_shader_1.vert"
};

static const char *const fragmentShaderSrc {
#include "fragment_shader_1.frag"
};

bool compileShader(const char *shaderSource, GLuint stage, GLuint &shader)
{
    shader = glCreateShader(stage);
    const GLchar *sourceCode = static_cast<const GLchar *>(shaderSource);
    glShaderSource(shader, 1, &sourceCode, NULL);
    glCompileShader(shader);

    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    return status;
}

GLuint shaderProgram(const char* vsSource, const char* fsSource)
{
    GLuint vertShader, fragShader;

    GLint result = GL_TRUE;

    result &= compileShader(vsSource, GL_VERTEX_SHADER, vertShader);
    result &= compileShader(fsSource, GL_FRAGMENT_SHADER, fragShader);

    if (!result)
    {
        qDebug() << "Could not compile all  shaders required for this program";
        return GL_FALSE;
    }

    qDebug() << "Linking shader m_program";
    GLuint program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);

    glLinkProgram(program);

    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    glUseProgram(program);

    return program;

}
bool loadBinaryShader(const char *fileName, GLuint stage, GLuint binaryFormat, GLuint &shader)
{
    //std::ifstream shaderFile;
    QFile shaderFile(fileName);
    //shaderFile.open(fileName, std::ios::binary | std::ios::ate);
    if (shaderFile.open(QIODevice::ReadOnly))
    {
        //size_t size = shaderFile.tellg();
        //shaderFile.seekg(0, std::ios::beg);
        //char* bin = new char[size];
        //shaderFile.read(bin, size);
        QByteArray bin = shaderFile.readAll();

        GLint status;
        shader = glCreateShader(stage);									// Create a new shader
        glShaderBinary(1, &shader, binaryFormat, bin.data(), bin.size());			// Load the binary shader file
        glSpecializeShader(shader, "main", 0, nullptr, nullptr);		// Set main entry point (required, no specialization used in this example)
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);				// Check compilation status
        return status;
    }
    else
    {
        qDebug() << "Could not open " << fileName;
        return false;
    }
}

GLuint loadShader(const char* vsFileName, const char* fsFileName)
{
    GLuint vertShader, fragShader;

    GLint result = GL_TRUE;

    result &= loadBinaryShader(vsFileName, GL_VERTEX_SHADER, GL_SHADER_BINARY_FORMAT_SPIR_V, vertShader);
    result &= loadBinaryShader(fsFileName, GL_FRAGMENT_SHADER, GL_SHADER_BINARY_FORMAT_SPIR_V, fragShader);

    if (!result)
    {
        qDebug() << "Could not load all binary shaders required for this program";
        return GL_FALSE;
    }

    qDebug() << "Linking shader m_program";
    GLuint program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);

    glLinkProgram(program);

    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    glUseProgram(program);

    return program;
}

}

GLint get_uniform_location(GLuint m_program, const char *uniformName){

    GLint location = glGetUniformLocation(m_program, uniformName);
    if (location == -1) {
        qDebug() << "Failed to get uniform location for:" << uniformName << "in m_program:" << m_program ;
    }
    return location;
}

GLint get_attribute_location(GLuint m_program, const char *attrName){

    GLint location = glGetAttribLocation(m_program, attrName);
    if (location == -1) {
        qDebug() << "Failed to get attribute location for:" << attrName << "in m_program:" << m_program ;
    }
    return location;
}

GLWindow::GLWindow()
{
    m_world.setToIdentity();
    m_world.translate(0, 0, -1);
    m_world.rotate(180, 1, 0, 0);

    QSequentialAnimationGroup *animGroup = new QSequentialAnimationGroup(this);
    animGroup->setLoopCount(-1);
    QPropertyAnimation *zAnim0 = new QPropertyAnimation(this, QByteArrayLiteral("z"));
    zAnim0->setStartValue(1.5f);
    zAnim0->setEndValue(10.0f);
    zAnim0->setDuration(2000);
    animGroup->addAnimation(zAnim0);
    QPropertyAnimation *zAnim1 = new QPropertyAnimation(this, QByteArrayLiteral("z"));
    zAnim1->setStartValue(10.0f);
    zAnim1->setEndValue(50.0f);
    zAnim1->setDuration(4000);
    zAnim1->setEasingCurve(QEasingCurve::OutElastic);
    animGroup->addAnimation(zAnim1);
    QPropertyAnimation *zAnim2 = new QPropertyAnimation(this, QByteArrayLiteral("z"));
    zAnim2->setStartValue(50.0f);
    zAnim2->setEndValue(1.5f);
    zAnim2->setDuration(2000);
    animGroup->addAnimation(zAnim2);
    animGroup->start();

    QPropertyAnimation* rAnim = new QPropertyAnimation(this, QByteArrayLiteral("r"));
    rAnim->setStartValue(0.0f);
    rAnim->setEndValue(360.0f);
    rAnim->setDuration(2000);
    rAnim->setLoopCount(-1);
    rAnim->start();

    QTimer::singleShot(4000, this, &GLWindow::startSecondStage);
}

GLWindow::~GLWindow()
{
    makeCurrent();
    //delete m_texture;
    // We don't need the m_program anymore.
    glDeleteProgram(m_program);
    if (m_isVaoCreated) {
        glDeleteVertexArrays(1, &m_vao);
    }

    if (m_isBufferAllocated) {
        glDeleteBuffers(1, &m_vBuffer);
    }

    if (m_isTextureAllocated) {
        glDeleteTextures(1, &m_texture);
    }
}

void GLWindow::startSecondStage()
{
    QPropertyAnimation* r2Anim = new QPropertyAnimation(this, QByteArrayLiteral("r2"));
    r2Anim->setStartValue(0.0f);
    r2Anim->setEndValue(360.0f);
    r2Anim->setDuration(20000);
    r2Anim->setLoopCount(-1);
    r2Anim->start();
}

void GLWindow::setZ(float v)
{
    m_eye.setZ(v);
    m_uniformsDirty = true;
    update();
}

void GLWindow::setR(float v)
{
    m_r = v;
    m_uniformsDirty = true;
    update();
}

void GLWindow::setR2(float v)
{
    m_r2 = v;
    m_uniformsDirty = true;
    update();
}

void GLWindow::initializeGL()
{
    glewExperimental = true;
    GLenum glew_init_result = glewInit();
    glGetError(); // Ignore GL_INVALID_ENUM​ error caused by glew
    if (glew_init_result != GLEW_OK) {
         qDebug("Failed to initialize glew");
    }

    QImage img(":/qtlogo.png");
    QImage imgMirr{img.mirrored()};
    Q_ASSERT(!img.isNull());
    //m_texture = new QOpenGLTexture(img.scaled(32, 36).mirrored());
    if (m_isTextureAllocated) {
        glDeleteTextures(1, &m_texture);
    }
    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (imgMirr.hasAlphaChannel()) {
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_RGBA,
                     img.width(),
                     img.height(),
                     0,
                     GL_RGBA,
                     GL_UNSIGNED_BYTE,
                     imgMirr.bits());
    } else {
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_RGB,
                     img.width(),
                     img.height(),
                     0,
                     GL_RGB,
                     GL_UNSIGNED_BYTE,
                     imgMirr.bits());
    }
    glGenerateMipmap(GL_TEXTURE_2D);
    m_isTextureAllocated = true;

    glBindTexture(GL_TEXTURE_2D, 0);

    /// load spir-v binary shaders
    m_program = loadShader(":/shaders/vertex_shader.vert.spv",":/shaders/fragment_shader.frag.spv");

    /// compile shaders from source
    //m_program = shaderProgram(vertexShaderSrc,fragmentShaderSrc);

    m_projMatrixLoc = get_uniform_location(m_program, "projMatrix");
    m_camMatrixLoc = get_uniform_location(m_program, "camMatrix");
    m_worldMatrixLoc = get_uniform_location(m_program, "worldMatrix");
    m_myMatrixLoc = get_uniform_location(m_program, "myMatrix");
    m_lightPosLoc = get_uniform_location(m_program, "lightPos");

    if (m_isVaoCreated){
        glDeleteVertexArrays(1, &m_vao);
    }
    glGenVertexArrays(1, &m_vao);
    m_isVaoCreated = true;

    if (m_isBufferAllocated) {
        glDeleteBuffers(1, &m_vBuffer);
    }
    glGenBuffers(1, &m_vBuffer);
    m_isBufferAllocated = true;

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vBuffer);
    glBufferData(GL_ARRAY_BUFFER, m_logo.count() * sizeof(GLfloat), m_logo.constData(), GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat),
                             nullptr);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat),
                             reinterpret_cast<void *>(3 * sizeof(GLfloat)));

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}

void GLWindow::resizeGL(int w, int h)
{
    m_proj.setToIdentity();
    m_proj.perspective(45.0f, GLfloat(w) / h, 0.01f, 100.0f);
    m_uniformsDirty = true;
}

void GLWindow::paintGL()
{
    glClearColor(0, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(m_vao);

    glUseProgram(m_program);

    glActiveTexture(GL_TEXTURE0); // activate the m_texture unit first before binding m_texture
    glBindTexture(GL_TEXTURE_2D, m_texture);

    if (m_uniformsDirty) {
        m_uniformsDirty = false;
        QMatrix4x4 camera;
        camera.lookAt(m_eye, m_eye + m_target, QVector3D(0, 1, 0));
        glUniformMatrix4fv(m_projMatrixLoc, 1, GL_FALSE,  m_proj.data());
        glUniformMatrix4fv(m_camMatrixLoc, 1, GL_FALSE,  camera.data());
        QMatrix4x4 wm = m_world;
        wm.rotate(m_r, 1, 1, 0);
        glUniformMatrix4fv(m_worldMatrixLoc, 1, GL_FALSE, wm.data());
        QMatrix4x4 mm;
        mm.setToIdentity();
        mm.rotate(-m_r2, 1, 0, 0);
        glUniformMatrix4fv(m_myMatrixLoc, 1, GL_FALSE, mm.data());
        glUniform3f(m_lightPosLoc, 0.f, 0.f, 0.7f);
    }


    glDrawArraysInstanced(GL_TRIANGLES, 0, m_logo.vertexCount(), 32 * 36);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
}
