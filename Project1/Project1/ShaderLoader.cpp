#include "GL/glew.h"
#include <GLFW/glfw3.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iostream>

GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path) {
    // 1. Tworzenie shaderów
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // 2. Wczytywanie kodu Vertex Shadera z pliku
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if (VertexShaderStream.is_open()) {
        std::stringstream sstr;
        sstr << VertexShaderStream.rdbuf();
        VertexShaderCode = sstr.str();
        VertexShaderStream.close();
    }
    else {
        std::cerr << "Nie udalo sie otworzyc pliku: " << vertex_file_path << "! Sprawdz sciezke." << std::endl;
        return 0;
    }

    // 3. Wczytywanie kodu Fragment Shadera z pliku
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if (FragmentShaderStream.is_open()) {
        std::stringstream sstr;
        sstr << FragmentShaderStream.rdbuf();
        FragmentShaderCode = sstr.str();
        FragmentShaderStream.close();
    }
    else {
        std::cerr << "Nie udalo sie otworzyc pliku: " << fragment_file_path << "! Sprawdz sciezke." << std::endl;
        return 0;
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;

    // 4. Kompilacja Vertex Shadera
    std::cout << "Kompilacja shadera: " << vertex_file_path << std::endl;
    char const* VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
    glCompileShader(VertexShaderID);

    // Sprawdzanie błędów kompilacji Vertex Shadera
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
        glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
        std::cerr << "Blad w Vertex Shaderze:\n" << &VertexShaderErrorMessage[0] << std::endl;
    }

    // 5. Kompilacja Fragment Shadera
    std::cout << "Kompilacja shadera: " << fragment_file_path << std::endl;
    char const* FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
    glCompileShader(FragmentShaderID);

    // Sprawdzanie błędów kompilacji Fragment Shadera
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
        std::cerr << "Blad w Fragment Shaderze:\n" << &FragmentShaderErrorMessage[0] << std::endl;
    }

    // 6. Linkowanie programu shaderów
    std::cout << "Linkowanie programu..." << std::endl;
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Sprawdzanie błędów linkowania
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
        glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        std::cerr << "Blad linkowania programu:\n" << &ProgramErrorMessage[0] << std::endl;
    }

    // 7. Czyszczenie (shadery są już wgrane do programu, pojedyncze obiekty można usunąć)
    glDetachShader(ProgramID, VertexShaderID);
    glDetachShader(ProgramID, FragmentShaderID);
    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}