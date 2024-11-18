#pragma once
#include <vector>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>

class Assimp_Model
{
public:
    using Array_Index = std::int32_t;

    enum struct Texture_Type
    {
        unknown,
        diffuse,
        specular,
        ambient,
        opacity,
        height,
        emissive,
        normal,
        shininess,
        displacement,
        reflection,
        lightmap,
        base_color,
        normal_camera,
        emission_color,
        metalness,
        diffuse_roughness,
        ambient_occlusion,
    };

    struct Node
    {
        Array_Index parent{-1};     // index for nodes array
        glm::mat4 transformation{};

        std::string name;
    };

    struct Mesh final
    {
        struct Triangle final
        {
            std::uint32_t a{};
            std::uint32_t b{};
            std::uint32_t c{};
        };

        struct Vertex_info final
        {
            std::vector<glm::vec3> position;
            std::vector<glm::vec3> normal;
            std::vector<glm::vec3> tangent;
            std::vector<glm::vec3> bitangent;
            std::vector<glm::vec2> texcoord;
            std::vector<glm::vec4> color;
        };

        Array_Index parent{-1};     // index for nodes array
        Array_Index material{-1};   // index for materials array

        std::vector<Triangle> topology;
        Vertex_info vertex_info;
        std::string name;
    };

    struct Material final
    {
        glm::vec4 diffuse_color{};
        glm::vec4 specular_color{};
        glm::vec4 ambient_color{};
        glm::vec4 transparent_color{};
        glm::vec4 emissive_color{};

        float shininess{};
        float opacity{};

        std::unordered_map<Texture_Type, std::string> material_textures;

        std::string name;
    };


    std::vector<Node> nodes;        // guaranteed to be in level-order
    std::vector<Mesh> meshes;
    std::vector<Material> materials;
    std::set<std::string> textures;
};

auto load_model(std::string path) -> Assimp_Model;
