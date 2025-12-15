#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "stb_image_write.h"

struct Vertex {
    glm::vec3 position{};
    glm::vec3 normal{};
    glm::vec2 uv{};
};

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
};

struct Material {
    glm::vec3 diffuse{0.8f};
    glm::vec3 specular{0.4f};
    float shininess{32.0f};
};

struct Transform {
    glm::vec3 translation{0.0f};
    glm::vec3 rotation{0.0f};
    glm::vec3 scale{1.0f};
};

static Mesh g_mesh;
static Material g_material;
static Transform g_transform;
static glm::vec3 g_lightPos(2.0f, 2.0f, 2.0f);
static float g_lightIntensity = 1.2f;
static bool g_useChecker = true;

static double g_lastX = 0.0, g_lastY = 0.0;
static bool g_rotating = false;
static bool g_panning = false;
static float g_yaw = -30.0f, g_pitch = -25.0f;
static float g_distance = 6.0f;
static glm::vec3 g_target(0.0f);
static float g_scrollDelta = 0.0f;

static unsigned int g_checkerTex = 0;

static void makeCheckerTexture() {
    unsigned char pixels[4 * 4 * 3];
    for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 4; ++x) {
            int idx = (y * 4 + x) * 3;
            bool black = ((x + y) % 2) == 0;
            pixels[idx + 0] = black ? 30 : 230;
            pixels[idx + 1] = black ? 30 : 230;
            pixels[idx + 2] = black ? 30 : 230;
        }
    }
    glGenTextures(1, &g_checkerTex);
    glBindTexture(GL_TEXTURE_2D, g_checkerTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 4, 4, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    glGenerateMipmap(GL_TEXTURE_2D);
}

static void computeNormals(Mesh &mesh) {
    for (auto &v : mesh.vertices) v.normal = glm::vec3(0.0f);
    for (size_t i = 0; i + 2 < mesh.indices.size(); i += 3) {
        Vertex &a = mesh.vertices[mesh.indices[i]];
        Vertex &b = mesh.vertices[mesh.indices[i + 1]];
        Vertex &c = mesh.vertices[mesh.indices[i + 2]];
        glm::vec3 n = glm::normalize(glm::cross(b.position - a.position, c.position - a.position));
        a.normal += n; b.normal += n; c.normal += n;
    }
    for (auto &v : mesh.vertices) v.normal = glm::normalize(v.normal);
}

static Mesh makeCube() {
    Mesh m;
    glm::vec3 positions[] = {
        {-1,-1, 1},{1,-1, 1},{1,1,1},{-1,1,1}, // front
        {-1,-1,-1},{-1,1,-1},{1,1,-1},{1,-1,-1}, // back
        {-1,1,-1},{-1,1,1},{1,1,1},{1,1,-1}, // top
        {-1,-1,-1},{1,-1,-1},{1,-1,1},{-1,-1,1}, // bottom
        {1,-1,-1},{1,1,-1},{1,1,1},{1,-1,1}, // right
        {-1,-1,-1},{-1,-1,1},{-1,1,1},{-1,1,-1} // left
    };
    glm::vec3 normals[] = {{0,0,1},{0,0,-1},{0,1,0},{0,-1,0},{1,0,0},{-1,0,0}};
    glm::vec2 uvs[] = {{0,0},{1,0},{1,1},{0,1}};
    for (int face = 0; face < 6; ++face) {
        int offset = face * 4;
        glm::vec3 n = normals[face];
        unsigned int base = m.vertices.size();
        for (int i = 0; i < 4; ++i) {
            Vertex v; v.position = positions[offset + i]; v.normal = n; v.uv = uvs[i];
            m.vertices.push_back(v);
        }
        m.indices.insert(m.indices.end(), {base, base+1, base+2, base, base+2, base+3});
    }
    return m;
}

static Mesh makeSphere(int slices=24, int stacks=16) {
    Mesh m;
    for (int i = 0; i <= stacks; ++i) {
        float v = (float)i / stacks;
        float phi = glm::pi<float>() * v;
        for (int j = 0; j <= slices; ++j) {
            float u = (float)j / slices;
            float theta = glm::two_pi<float>() * u;
            glm::vec3 p(std::sin(phi)*std::cos(theta), std::cos(phi), std::sin(phi)*std::sin(theta));
            Vertex vert; vert.position = p; vert.normal = p; vert.uv = {u, v};
            m.vertices.push_back(vert);
        }
    }
    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            int row1 = i * (slices + 1);
            int row2 = (i + 1) * (slices + 1);
            unsigned int a = row1 + j;
            unsigned int b = row1 + j + 1;
            unsigned int c = row2 + j + 1;
            unsigned int d = row2 + j;
            m.indices.insert(m.indices.end(), {a,b,c, a,c,d});
        }
    }
    return m;
}

static Mesh makeCylinder(int sides=24, bool capped=true) {
    Mesh m;
    for (int y = 0; y <= 1; ++y) {
        float vy = y==0?-1.0f:1.0f;
        for (int i = 0; i <= sides; ++i) {
            float u = (float)i / sides;
            float theta = glm::two_pi<float>() * u;
            glm::vec3 p(std::cos(theta), vy, std::sin(theta));
            Vertex v; v.position = p; v.normal = glm::normalize(glm::vec3(p.x, 0, p.z)); v.uv = {u, (vy+1)/2};
            m.vertices.push_back(v);
        }
    }
    int ring = sides + 1;
    for (int i = 0; i < sides; ++i) {
        unsigned int a = i; unsigned int b = i+1; unsigned int c = ring + i + 1; unsigned int d = ring + i;
        m.indices.insert(m.indices.end(), {a,b,c, a,c,d});
    }
    if (capped) {
        unsigned int baseTop = m.vertices.size();
        m.vertices.push_back({{0,1,0},{0,1,0},{0.5f,0.5f}});
        for (int i = 0; i <= sides; ++i) {
            float u = (float)i / sides; float theta = glm::two_pi<float>() * u;
            glm::vec3 p(std::cos(theta),1,std::sin(theta));
            Vertex v; v.position = p; v.normal = {0,1,0}; v.uv = {0.5f+0.5f*p.x, 0.5f+0.5f*p.z};
            m.vertices.push_back(v);
        }
        for (int i = 0; i < sides; ++i) {
            m.indices.insert(m.indices.end(), {baseTop, baseTop + i + 1, baseTop + i + 2});
        }

        unsigned int baseBottom = m.vertices.size();
        m.vertices.push_back({{0,-1,0},{0,-1,0},{0.5f,0.5f}});
        for (int i = 0; i <= sides; ++i) {
            float u = (float)i / sides; float theta = glm::two_pi<float>() * u;
            glm::vec3 p(std::cos(theta),-1,std::sin(theta));
            Vertex v; v.position = p; v.normal = {0,-1,0}; v.uv = {0.5f+0.5f*p.x, 0.5f+0.5f*p.z};
            m.vertices.push_back(v);
        }
        for (int i = 0; i < sides; ++i) {
            m.indices.insert(m.indices.end(), {baseBottom, baseBottom + i + 2, baseBottom + i + 1});
        }
    }
    return m;
}

static Mesh makeCone(int sides=24) {
    Mesh m;
    glm::vec3 tip(0,1,0);
    for (int i = 0; i <= sides; ++i) {
        float u = (float)i / sides; float theta = glm::two_pi<float>() * u;
        glm::vec3 p(std::cos(theta),-1,std::sin(theta));
        glm::vec3 edge = glm::normalize(glm::vec3(p.x, 0.5f, p.z));
        Vertex v; v.position = p; v.normal = edge; v.uv = {u,0};
        m.vertices.push_back(v);
    }
    for (int i = 0; i < sides; ++i) {
        unsigned int a = i; unsigned int b = i+1; unsigned int c = m.vertices.size();
        glm::vec3 n = glm::normalize(glm::vec3(m.vertices[a].position.x,0.5f,m.vertices[a].position.z));
        m.vertices.push_back({tip, n, {m.vertices[a].uv.x,1}});
        m.indices.insert(m.indices.end(), {a,b,c});
    }
    unsigned int center = m.vertices.size();
    m.vertices.push_back({{0,-1,0},{0,-1,0},{0.5f,0.5f}});
    for (int i = 0; i < sides; ++i) {
        glm::vec3 p1 = m.vertices[i].position; glm::vec3 p2 = m.vertices[i+1].position;
        unsigned int base = m.vertices.size();
        m.vertices.push_back({p1,{0,-1,0},{0.5f+0.5f*p1.x,0.5f+0.5f*p1.z}});
        m.vertices.push_back({p2,{0,-1,0},{0.5f+0.5f*p2.x,0.5f+0.5f*p2.z}});
        m.indices.insert(m.indices.end(), {center, base+1, base});
    }
    computeNormals(m);
    return m;
}

static Mesh makePrism(int sides=6, float topScale=1.0f) {
    Mesh m;
    float angleStep = glm::two_pi<float>() / sides;
    std::vector<glm::vec3> bottom, top;
    for (int i = 0; i < sides; ++i) {
        float theta = angleStep * i;
        bottom.push_back({std::cos(theta), -1.0f, std::sin(theta)});
        top.push_back({std::cos(theta) * topScale, 1.0f, std::sin(theta) * topScale});
    }
    for (int i = 0; i < sides; ++i) {
        int next = (i + 1) % sides;
        glm::vec3 p0 = bottom[i]; glm::vec3 p1 = bottom[next];
        glm::vec3 p2 = top[next]; glm::vec3 p3 = top[i];
        glm::vec3 normal = glm::normalize(glm::cross(p1 - p0, p3 - p0));
        unsigned int base = m.vertices.size();
        m.vertices.push_back({p0,normal,{0,0}});
        m.vertices.push_back({p1,normal,{1,0}});
        m.vertices.push_back({p2,normal,{1,1}});
        m.vertices.push_back({p3,normal,{0,1}});
        m.indices.insert(m.indices.end(), {base, base+1, base+2, base, base+2, base+3});
    }
    unsigned int centerTop = m.vertices.size();
    m.vertices.push_back({{0,1,0},{0,1,0},{0.5f,0.5f}});
    for (int i = 0; i < sides; ++i) {
        int next = (i+1)%sides; glm::vec3 p1 = top[i]; glm::vec3 p2 = top[next];
        unsigned int base = m.vertices.size();
        m.vertices.push_back({p1,{0,1,0},{0.5f+0.5f*p1.x,0.5f+0.5f*p1.z}});
        m.vertices.push_back({p2,{0,1,0},{0.5f+0.5f*p2.x,0.5f+0.5f*p2.z}});
        m.indices.insert(m.indices.end(), {centerTop, base, base+1});
    }
    unsigned int centerBottom = m.vertices.size();
    m.vertices.push_back({{0,-1,0},{0,-1,0},{0.5f,0.5f}});
    for (int i = 0; i < sides; ++i) {
        int next = (i+1)%sides; glm::vec3 p1 = bottom[i]; glm::vec3 p2 = bottom[next];
        unsigned int base = m.vertices.size();
        m.vertices.push_back({p1,{0,-1,0},{0.5f+0.5f*p1.x,0.5f+0.5f*p1.z}});
        m.vertices.push_back({p2,{0,-1,0},{0.5f+0.5f*p2.x,0.5f+0.5f*p2.z}});
        m.indices.insert(m.indices.end(), {centerBottom, base+1, base});
    }
    return m;
}

static void exportOBJ(const Mesh &mesh, const std::string &path) {
    std::ofstream out(path);
    for (const auto &v : mesh.vertices) {
        out << "v " << v.position.x << ' ' << v.position.y << ' ' << v.position.z << "\n";
    }
    for (const auto &v : mesh.vertices) {
        out << "vt " << v.uv.x << ' ' << v.uv.y << "\n";
    }
    for (const auto &v : mesh.vertices) {
        out << "vn " << v.normal.x << ' ' << v.normal.y << ' ' << v.normal.z << "\n";
    }
    for (size_t i = 0; i + 2 < mesh.indices.size(); i += 3) {
        out << "f";
        for (int j = 0; j < 3; ++j) {
            unsigned int idx = mesh.indices[i + j] + 1;
            out << ' ' << idx << '/' << idx << '/' << idx;
        }
        out << "\n";
    }
    printf("Saved OBJ to %s\n", path.c_str());
}

static bool importOBJ(const std::string &path, Mesh &mesh) {
    std::ifstream in(path);
    if (!in.is_open()) return false;
    std::vector<glm::vec3> positions; std::vector<glm::vec3> normals; std::vector<glm::vec2> uvs;
    std::string line;
    while (std::getline(in, line)) {
        std::istringstream ss(line);
        std::string tag; ss >> tag;
        if (tag == "v") {
            glm::vec3 p; ss >> p.x >> p.y >> p.z; positions.push_back(p);
        } else if (tag == "vn") {
            glm::vec3 n; ss >> n.x >> n.y >> n.z; normals.push_back(glm::normalize(n));
        } else if (tag == "vt") {
            glm::vec2 uv; ss >> uv.x >> uv.y; uvs.push_back(uv);
        } else if (tag == "f") {
            for (int i = 0; i < 3; ++i) {
                std::string vert; ss >> vert;
                unsigned int pi=0, ti=0, ni=0;
                sscanf(vert.c_str(), "%u/%u/%u", &pi, &ti, &ni);
                Vertex v{};
                if (pi>0 && pi<=positions.size()) v.position = positions[pi-1];
                if (ni>0 && ni<=normals.size()) v.normal = normals[ni-1];
                if (ti>0 && ti<=uvs.size()) v.uv = uvs[ti-1];
                mesh.vertices.push_back(v);
                mesh.indices.push_back(mesh.vertices.size()-1);
            }
        }
    }
    if (mesh.vertices.empty()) return false;
    if (normals.empty()) computeNormals(mesh);
    printf("Loaded OBJ from %s\n", path.c_str());
    return true;
}

static void resetTransform() {
    g_transform = {};
    g_transform.scale = glm::vec3(1.0f);
}

static glm::mat4 buildModelMatrix() {
    glm::mat4 model(1.0f);
    model = glm::translate(model, g_transform.translation);
    model = glm::rotate(model, glm::radians(g_transform.rotation.x), glm::vec3(1,0,0));
    model = glm::rotate(model, glm::radians(g_transform.rotation.y), glm::vec3(0,1,0));
    model = glm::rotate(model, glm::radians(g_transform.rotation.z), glm::vec3(0,0,1));
    model = glm::scale(model, g_transform.scale);
    return model;
}

static void applyCamera(GLuint width, GLuint height) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, double(width)/double(height), 0.1, 100.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glm::vec3 dir;
    dir.x = std::cos(glm::radians(g_pitch)) * std::cos(glm::radians(g_yaw));
    dir.y = std::sin(glm::radians(g_pitch));
    dir.z = std::cos(glm::radians(g_pitch)) * std::sin(glm::radians(g_yaw));
    glm::vec3 eye = g_target - dir * g_distance;

    gluLookAt(eye.x, eye.y, eye.z, g_target.x, g_target.y, g_target.z, 0.0,1.0,0.0);
}

static void drawMesh(const Mesh &mesh) {
    glBegin(GL_TRIANGLES);
    for (auto idx : mesh.indices) {
        const Vertex &v = mesh.vertices[idx];
        glNormal3fv(glm::value_ptr(v.normal));
        glTexCoord2fv(glm::value_ptr(v.uv));
        glVertex3fv(glm::value_ptr(v.position));
    }
    glEnd();
}

static void saveScreenshot(int width, int height) {
    std::vector<unsigned char> pixels(width * height * 3);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    for (int y = 0; y < height/2; ++y) {
        for (int x = 0; x < width*3; ++x) {
            std::swap(pixels[y*width*3 + x], pixels[(height-1-y)*width*3 + x]);
        }
    }
    std::vector<unsigned char> buffer;
    auto writer = [](void *context, void *data, int size) {
        auto *buf = reinterpret_cast<std::vector<unsigned char>*>(context);
        unsigned char *bytes = static_cast<unsigned char*>(data);
        buf->insert(buf->end(), bytes, bytes + size);
    };
    stbi_write_png_to_func(writer, &buffer, width, height, 3, pixels.data(), width*3);
    const char* path = "screenshot.png";
    FILE *f = fopen(path, "wb");
    fwrite(buffer.data(), 1, buffer.size(), f);
    fclose(f);
    printf("Saved screenshot to %s\n", path);
}

static void printHelp() {
    puts("Simple OpenGL Modeling Demo\n"
         "Keys: 1 Cube, 2 Sphere, 3 Cylinder, 4 Cone, 5 Prism, 6 Frustum Prism\n"
         "      Shift+L Load OBJ (scene.obj), Shift+O Export OBJ (export.obj)\n"
         "      Arrow/PgUp/PgDn translate, R/T/Y rotate, +/- scale\n"
         "      H/B specular +/- , M/N diffuse +/- , </> shininess +/-\n"
         "      Light move: I/K (Y), J/L (X), U/O (Z). [ ] intensity\n"
         "      Mouse drag left: orbit, right: pan, scroll: zoom, F: frame\n"
         "      P toggle checker texture, V saves screenshot.png\n");
}

static void updateLight() {
    GLfloat lightPos[] = {g_lightPos.x, g_lightPos.y, g_lightPos.z, 1.0f};
    GLfloat diffuse[] = {g_lightIntensity, g_lightIntensity, g_lightIntensity, 1.0f};
    GLfloat ambient[] = {0.15f, 0.15f, 0.15f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, diffuse);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
}

static void applyMaterial() {
    GLfloat diff[] = {g_material.diffuse.r, g_material.diffuse.g, g_material.diffuse.b, 1.0f};
    GLfloat spec[] = {g_material.specular.r, g_material.specular.g, g_material.specular.b, 1.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diff);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, g_material.shininess);
}

static void scrollCallback(GLFWwindow*, double, double yoffset) {
    g_scrollDelta += (float)yoffset;
}

int main() {
    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Simple 3D Modeling", nullptr, nullptr);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetScrollCallback(window, scrollCallback);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);
    glEnable(GL_TEXTURE_2D);

    makeCheckerTexture();
    g_mesh = makeCube();
    resetTransform();
    printHelp();

    while (!glfwWindowShouldClose(window)) {
        int width, height; glfwGetFramebufferSize(window, &width, &height);
        glViewport(0,0,width,height);
        glClearColor(0.1f,0.1f,0.12f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        applyCamera(width, height);
        updateLight();
        applyMaterial();

        if (g_useChecker) glBindTexture(GL_TEXTURE_2D, g_checkerTex);
        else glBindTexture(GL_TEXTURE_2D, 0);

        glPushMatrix();
        glm::mat4 model = buildModelMatrix();
        glMultMatrixf(glm::value_ptr(model));
        drawMesh(g_mesh);
        glPopMatrix();

        glfwSwapBuffers(window);
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, GLFW_TRUE);

        // camera controls
        double x, y; glfwGetCursorPos(window, &x, &y);
        bool left = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
        bool right = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
        if (left && !g_rotating) { g_rotating = true; g_lastX = x; g_lastY = y; }
        if (right && !g_panning) { g_panning = true; g_lastX = x; g_lastY = y; }
        if (!left) g_rotating = false; if (!right) g_panning = false;
        if (g_rotating) {
            float dx = float(x - g_lastX); float dy = float(y - g_lastY);
            g_yaw -= dx * 0.25f; g_pitch -= dy * 0.25f;
            g_pitch = std::clamp(g_pitch, -89.0f, 89.0f);
            g_lastX = x; g_lastY = y;
        }
        if (g_panning) {
            float dx = float(x - g_lastX) * 0.01f; float dy = float(y - g_lastY) * 0.01f;
            g_target += glm::vec3(-dx, dy, 0.0f);
            g_lastX = x; g_lastY = y;
        }
        if (g_scrollDelta != 0.0f) {
            g_distance = std::max(1.5f, g_distance - g_scrollDelta);
            g_scrollDelta = 0.0f;
        }

        // transforms
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) g_transform.translation.x -= 0.02f;
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) g_transform.translation.x += 0.02f;
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) g_transform.translation.y += 0.02f;
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) g_transform.translation.y -= 0.02f;
        if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS) g_transform.translation.z -= 0.02f;
        if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS) g_transform.translation.z += 0.02f;

        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) g_transform.rotation.x += 0.5f;
        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) g_transform.rotation.y += 0.5f;
        if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) g_transform.rotation.z += 0.5f;

        if (glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS)
            g_transform.scale *= 1.005f;
        if (glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS)
            g_transform.scale *= 0.995f;

        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) { g_target = {0,0,0}; g_distance = 6.0f; }

        // light movement
        if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) g_lightPos.y += 0.02f;
        if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) g_lightPos.y -= 0.02f;
        if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) g_lightPos.x -= 0.02f;
        if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) g_lightPos.x += 0.02f;
        if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) g_lightPos.z -= 0.02f;
        if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) g_lightPos.z += 0.02f;
        if (glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS) g_lightIntensity = std::max(0.1f, g_lightIntensity - 0.01f);
        if (glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS) g_lightIntensity = std::min(3.0f, g_lightIntensity + 0.01f);

        // material editing
        if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) g_material.diffuse = glm::clamp(g_material.diffuse + glm::vec3(0.001f), 0.0f, 1.0f);
        if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) g_material.diffuse = glm::clamp(g_material.diffuse - glm::vec3(0.001f), 0.0f, 1.0f);
        if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) g_material.specular = glm::clamp(g_material.specular + glm::vec3(0.001f), 0.0f, 1.0f);
        if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) g_material.specular = glm::clamp(g_material.specular - glm::vec3(0.001f), 0.0f, 1.0f);
        if (glfwGetKey(window, GLFW_KEY_COMMA) == GLFW_PRESS) g_material.shininess = std::max(2.0f, g_material.shininess - 0.5f);
        if (glfwGetKey(window, GLFW_KEY_PERIOD) == GLFW_PRESS) g_material.shininess = std::min(128.0f, g_material.shininess + 0.5f);

        // mesh switching
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) { g_mesh = makeCube(); resetTransform(); }
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) { g_mesh = makeSphere(); resetTransform(); }
        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) { g_mesh = makeCylinder(); resetTransform(); }
        if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) { g_mesh = makeCone(); resetTransform(); }
        if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) { g_mesh = makePrism(6,1.0f); resetTransform(); }
        if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) { g_mesh = makePrism(6,0.6f); resetTransform(); }

        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) g_useChecker = !g_useChecker;

        if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS && (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)==GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT)==GLFW_PRESS)) {
            exportOBJ(g_mesh, "export.obj");
        }
        if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS && (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)==GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT)==GLFW_PRESS)) {
            Mesh imported;
            if (importOBJ("scene.obj", imported)) { g_mesh = imported; resetTransform(); }
            else puts("Failed to load scene.obj");
        }
        if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) saveScreenshot(width, height);
    }

    glfwTerminate();
    return 0;
}
