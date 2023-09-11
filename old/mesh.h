#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include "vulkan/vulkan.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>
#include "glm/glm.hpp"
#include <array>

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;
	glm::vec3 normals;

	static VkVertexInputBindingDescription getBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription{}; /* Structure specifying vertex input binding description. More details in https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkVertexInputBindingDescription.html */
		bindingDescription.binding = 0; /* Binding number this structure describes. Must be less than VkPhysicalDeviceLimits::maxVertexInputBindings. */
		bindingDescription.stride = sizeof(Vertex); /* Byte stride between consecutive elements within the buffer. Must be less or equal to VkPhysicalDeviceLimits::maxVertexInputBindingStride. */
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{}; /* Structure specifying vertex input attribute description. More details in https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkVertexInputAttributeDescription.html */
		attributeDescriptions[0].binding = 0; /* Binding number which this attribute takes its data from. */
		attributeDescriptions[0].location = 0; /* Shader input location number for this attribute.  ie inPositions location*/
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT; /* Size and type of the vertex attribute data. formats are specified using the same enumeration as color formats. */
		attributeDescriptions[0].offset = offsetof(Vertex, pos); /* Byte offset of this attribute relative to the start of an element in the vertex input binding. */

		attributeDescriptions[1].binding = 0; /* Binding number which this attribute takes its data from. */
		attributeDescriptions[1].location = 1; /* Shader input location number for this attribute.  ie inColor location*/
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT; /* Size and type of the vertex attribute data. formats are specified using the same enumeration as color formats. */
		attributeDescriptions[1].offset = offsetof(Vertex, color); /* Byte offset of this attribute relative to the start of an element in the vertex input binding. */

		attributeDescriptions[2].binding = 0; /* Binding number which this attribute takes its data from. */
		attributeDescriptions[2].location = 2; /* Shader input location number for this attribute.  ie texCoords location*/
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT; /* Size and type of the vertex attribute data. formats are specified using the same enumeration as color formats. */
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord); /* Byte offset of this attribute relative to the start of an element in the vertex input binding. */

		return attributeDescriptions;
	}
	bool operator==(const Vertex& other) const {
		return pos == other.pos && color == other.color && texCoord == other.texCoord;
	}
};

namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}

class Mesh
{
public:
	Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

	/*Mesh* processMesh(aiMesh* mesh);
	void processNode(std::vector<Mesh*>* meshes, aiNode* node, const aiScene* scene);
	std::vector<Mesh*> loadMesh(const std::string& path);*/

	[[nodiscard]] const std::vector<Vertex> getVertices() const { return m_Vertices; }
	[[nodiscard]] const std::vector<uint32_t> getIndices() const { return m_Indices; }

private:
	std::vector<Vertex> m_Vertices;
	std::vector<uint32_t> m_Indices;
};