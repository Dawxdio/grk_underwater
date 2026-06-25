#define TINYOBJLOADER_IMPLEMENTATION 
#include "model_loader.h"
#include "tiny_obj_loader.h"

#include <iostream>

bool loadOBJ(const std::string& path, std::vector<Vertex>& out_vertices) {
    std::cout << "\n[DEBUG] Rozpoczynam ladowanie modelu z: " << path << std::endl;

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn;
    std::string err;

    // Ładowanie pliku
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str());

    if (!warn.empty()) {
        std::cout << "[DEBUG] Ostrzezenie biblioteki tinyobj: " << warn << std::endl;
    }
    if (!err.empty()) {
        std::cerr << "[DEBUG] BLAD biblioteki tinyobj: " << err << std::endl;
    }

    if (!ret) {
        std::cerr << "[DEBUG] KRYTYCZNY BLAD: Nie udalo sie otworzyc pliku! Sprawdz czy sciezka jest poprawna." << std::endl;
        return false;
    }

    std::cout << "[DEBUG] Plik otwarty pomyslnie." << std::endl;
    std::cout << "[DEBUG] Liczba wierzcholkow bazowych (attrib.vertices): " << attrib.vertices.size() / 3 << std::endl;
    std::cout << "[DEBUG] Liczba zdefiniowanych ksztaltow (shapes): " << shapes.size() << std::endl;

    size_t total_triangles = 0;

    // Przetwarzanie danych
    for (size_t s = 0; s < shapes.size(); s++) {
        size_t index_offset = 0;
        total_triangles += shapes[s].mesh.num_face_vertices.size();

        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

            for (size_t v = 0; v < fv; v++) {
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                Vertex vertex{};

                vertex.x = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                vertex.y = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                vertex.z = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

                if (idx.normal_index >= 0) {
                    vertex.nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
                    vertex.ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
                    vertex.nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
                }
                else {
                    // Jeśli model nie ma normalnych, przypisz domyślne (w górę)
                    vertex.nx = 0.0f; vertex.ny = 1.0f; vertex.nz = 0.0f;
                }

                if (idx.texcoord_index >= 0) {
                    vertex.tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                    vertex.ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
                }

                out_vertices.push_back(vertex);
            }
            index_offset += fv;
        }
    }

    std::cout << "[DEBUG] Przetwarzanie zakonczone." << std::endl;
    std::cout << "[DEBUG] Laczna liczba wygenerowanych trojkatow: " << total_triangles << std::endl;
    std::cout << "[DEBUG] Rozmiar koncowej tablicy wierzcholkow (out_vertices): " << out_vertices.size() << std::endl << std::endl;

    if (out_vertices.empty()) {
        std::cerr << "[DEBUG] OSTRZEZENIE: Tablica wierzcholkow jest pusta! Model nie ma geometrii." << std::endl;
    }

    return true;
}