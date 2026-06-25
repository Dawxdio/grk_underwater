#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#include <string>
#include <vector>

// Prosta struktura wierzchołka, dostosuj ją pod swój shader PBR
struct Vertex {
    float x, y, z;    // Pozycja
    float nx, ny, nz; // Normalna
    float tx, ty;     // Koordynaty tekstury (UV)
};

// Funkcja wczytująca plik .obj i zwracająca wektor wierzchołków
bool loadOBJ(const std::string& path, std::vector<Vertex>& out_vertices);

#endif