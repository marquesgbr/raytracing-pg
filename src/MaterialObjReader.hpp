#ifndef OBJREADERHEADER
#define OBJREADERHEADER

/*
Classe leitora de arquivos .obj. Onde o arquivo contém os vários pontos, normais e faces do objeto. No projeto
trabalhamos com faces triangulares, ou seja, uma face consiste em 3 pontos.

No arquivo .obj, temos:
    - v = pontos
    - vn = normais
    - vt = texturas
    - f = faces

Nessa classe podem ser obtidas as seguintes informações (por meio dos Getters):
    - Pontos
    - Normais
    - Lista de faces com seus respectivos pontos
    - Informações de cor, brilho, opacidade, etc.

Obs: -  Para fins de abstração, as normais de cada ponto são ignoradas e assumimos apenas uma normal para cada face.
     -  As texturas também são ignoradas.

Caso sintam necessidade, podem editar a classe para obter mais informações.
*/

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include "Colormap.hpp"

struct Face
{
    int verticeIndice[3];
    int normalIndice[3];
    Vector ka;
    Vector kd;
    Vector ks;
    Vector ke;
    double ns;
    double ni;
    double d;

    Face()
    {
        for (int i = 0; i < 3; ++i)
        {
            verticeIndice[i] = 0;
            normalIndice[i] = 0;
        }
        ka = Vector();
        kd = Vector();
        ks = Vector();
        ke = Vector();
        ns = 0.0;
        ni = 0.0;
        d = 0.0;
    }
};

class ObjReader
{

private:
    std::ifstream file;                         // Arquivo .obj
    std::vector<Point> vertices;                // Lista de pontos
    std::vector<Vector> normals;                // Lista de normais
    std::vector<Face> faces;                    // Lista de indices de faces
    std::vector<std::vector<Point>> facePoints; // Lista de pontos das faces
    MaterialProperties curMaterial;             // Material atual
    colormap cmap;                              // Objeto de leitura de arquivos .mtl
    Point center;

public:
    ObjReader(std::string filename)
    {

        // Abre o arquivo
        file.open(filename);
        if (!file.is_open())
        {
            std::cerr << "Erro ao abrir o arquivo: " << filename << std::endl;
            return;
        }

        // Leitura do arquivo
        std::string line, mtlfile, colorname, filename_mtl;
        while (std::getline(file, line))
        {
            std::istringstream iss(line);
            std::string prefix;
            iss >> prefix;

            if (prefix == "mtllib")
            {
                iss >> filename_mtl;
                std::string filename_mtl_path = filename.replace(filename.length() - 3, 3, "mtl");
                cmap = colormap(filename_mtl_path);
            }
            else if (prefix == "usemtl")
            {
                iss >> colorname;
                curMaterial = cmap.getMaterialProperties(colorname);
            }
            else if (prefix == "v")
            {
                double x, y, z;
                iss >> x >> y >> z;
                vertices.emplace_back(x, y, z);
            }
            else if (prefix == "vn")
            {
                double x, y, z;
                iss >> x >> y >> z;
                normals.emplace_back(x, y, z);
            }
            else if (prefix == "f")
            {
                Face face;
                char slash;
                for (int i = 0; i < 3; ++i)
                {
                    int _;
                    iss >> face.verticeIndice[i] >> slash >> _ >> slash >> face.normalIndice[i];
                    face.ka = curMaterial.ka;
                    face.kd = curMaterial.kd;
                    face.ks = curMaterial.ks;
                    face.ke = curMaterial.ke;
                    face.ns = curMaterial.ns;
                    face.ni = curMaterial.ni;
                    face.d = curMaterial.d;
                    face.verticeIndice[i]--;
                    face.normalIndice[i]--;
                }
                faces.push_back(face);
            }
        }
        for (const auto &face : faces)
        {
            std::vector<Point> points = {
                vertices[face.verticeIndice[0]],
                vertices[face.verticeIndice[1]],
                vertices[face.verticeIndice[2]]};
            facePoints.push_back(points);
        }

        file.close();

        center = getCenter();
    }

    // Getters

    // Método para retornar as coordenadas dos pontos das faces
    std::vector<std::vector<Point>> getFacePoints()
    {
        return facePoints;
    }

    /*
    Retorna uma lista com um struct faces do objeto. Cada face contém:
        - Índices dos pontos
        - Índices das normais
        - Cores (ka, kd, ks, ke)
        - Brilho (ns)
        - Índice de refração (ni)
        - Opacidade (d)
    */
    std::vector<Face> getFaces()
    {
        return faces;
    }

    // Método para retornar a cor do material (Coeficiente de difusão)
    Vector getKd()
    {
        return curMaterial.kd;
    }

    // Método para retornar a cor do ambiente
    Vector getKa()
    {
        return curMaterial.ka;
    }

    // Método para retornar o coeficiente especular (Refletência)
    Vector getKe()
    {
        return curMaterial.ke;
    }

    // Método para retornar o coeficiente de brilho
    double getNs()
    {
        return curMaterial.ns;
    }

    // Método para retornar o índice de refração
    double getNi()
    {
        return curMaterial.ni;
    }

    // Método para retornar o coeficiente de especularidade
    Vector getKs()
    {
        return curMaterial.ks;
    }

    // Método para retornar o indice de opacidade
    double getD()
    {
        return curMaterial.d;
    }

    // Método para retornar as coordenadas dos pontos
    std::vector<Point> getVertices()
    {
        return vertices;
    }

    std::vector<Vector> getNormals()
    {
        return normals;
    }

    // Emite um output no terminal para cada face, com seus respectivos pontos (x, y, z)
    void print_faces()
    {
        int i = 0;
        for (const auto &face : facePoints)
        {
            i++;
            std::cout << "Face " << i << ": ";
            for (const auto &Point : face)
            {
                std::cout << "(" << Point.getX() << ", " << Point.getY() << ", " << Point.getZ() << ")";
            }
            std::cout << std::endl;
        }
    }

    void attachMaterials(std::vector<Material> &objects)
    {
        for (const auto &face : faces)
        {
            objects.emplace_back(facetoTriang(face),
                                 face.kd * 255, face.ka, face.kd, face.ks, face.ke, 1 - face.d, face.ns, face.ni);
        }
    }

    Triangle* facetoTriang(Face const &face)
    {
        return new Triangle(
            vertices[face.verticeIndice[0]],
            vertices[face.verticeIndice[1]],
            vertices[face.verticeIndice[2]],
            normals[face.normalIndice[0]]);
    }

    Point getCenter()
    {
        Point p(0, 0, 0);
        for (auto &v : vertices)
        {
            p = p + v;
        }
        return p / vertices.size();
    }

    void applyTransform(Matrix &transformMatrix)
    {

        Matrix translateOrigin = Matrix::translation(-1 * center);
        Matrix translateBack = Matrix::translation(center);

        transformMatrix = translateBack * transformMatrix * translateOrigin;

        for (auto &v : vertices)
        {
            v = transformMatrix * v;
        }

        center = getCenter();
    }
};

#endif