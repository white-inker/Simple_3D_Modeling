//
// Created by ChatGPT on 26-01-08.
//

#ifndef SCENEEXPORTER_H
#define SCENEEXPORTER_H

#include <string>

class Scene;

// 导出整个场景为 OBJ 文件（不含材质文件）
bool exportSceneToOBJ(const Scene& scene, const std::string& outputPath);

#endif //SCENEEXPORTER_H
