````
IndoorSceneRenderer/
│
├── CMakeLists.txt          # 构建配置（或 .vcxproj）
├── README.md               # 项目说明（课程答辩神器）
│
├── assets/                 # 资源文件（不放代码）
│   ├── models/             # 模型文件（OBJ）
│   │   ├── chair.obj
│   │   ├── table.obj
│   │   └── room.obj
│   │
│   ├── textures/           # 纹理
│   │   ├── wall_diffuse.jpg
│   │   ├── floor.png
│   │   └── wood.jpg
│   │
│   └── shaders/            # GLSL 着色器
│       ├── basic.vert
│       ├── phong.frag
│       ├── shadow.vert     # （可选）
│       └── shadow.frag
│
├── include/                # 头文件
│   ├── core/
│   │   ├── Application.h
│   │   ├── Renderer.h
│   │   └── Window.h
│   │
│   ├── scene/
│   │   ├── Scene.h
│   │   ├── SceneNode.h
│   │   └── Transform.h
│   │
│   ├── geometry/
│   │   ├── Mesh.h
│   │   ├── Primitive.h
│   │   ├── Cube.h
│   │   ├── Sphere.h
│   │   ├── Cylinder.h
│   │   └── Cone.h
│   │
│   ├── graphics/
│   │   ├── Shader.h
│   │   ├── Texture.h
│   │   ├── Material.h
│   │   └── Light.h
│   │
│   ├── camera/
│   │   ├── Camera.h
│   │   └── OrbitCamera.h
│   │
│   ├── io/
│   │   ├── ModelLoader.h   # OBJ 导入
│   │   ├── ModelExporter.h # OBJ 导出（可选）
│   │   └── Screenshot.h
│   │
│   ├── interaction/
│   │   ├── Input.h
│   │   └── Controller.h
│   │
│   └── utils/
│       ├── MathUtils.h
│       ├── FileUtils.h
│       └── Timer.h
│
├── src/                    # 源文件
│   ├── main.cpp
│   │
│   ├── core/
│   ├── scene/
│   ├── geometry/
│   ├── graphics/
│   ├── camera/
│   ├── io/
│   ├── interaction/
│   └── utils/
│
├── third_party/            # 第三方库
│   ├── glad/
│   ├── glfw/
│   ├── glm/
│   ├── stb/
│   └── tinyobjloader/
│
└── screenshots/            # 截图输出

