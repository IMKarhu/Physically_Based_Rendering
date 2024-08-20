#pragma once
#include <vulkan/vulkan.h>
#include "kBuffer.hpp"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <array>





namespace karhu
{
	struct Vulkan_Device;

	class Texture
	{
	public:
		explicit Texture(std::shared_ptr<struct Vulkan_Device> device);
		~Texture();
		//void loadFile(std::string fileName, VkFormat format);
		void createTexture(VkCommandPool& commandPool);
		void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, 
			VkImage& image, VkDeviceMemory& imageMemory);
		void createTextureImageView();
		VkImageView createImageview(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
		void createSampler();
		void transitionImagelayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkCommandPool& commandPool);
		void copyBufferToImage(VkBuffer vbuffer, VkImage image, uint32_t width, uint32_t height, VkCommandPool& commandPool);

		VkImageView imageView() const { return m_ImageView; }
		VkSampler textureSampler() const { return m_Sampler; }
	private:
		std::shared_ptr<struct Vulkan_Device> m_VkDevice;
		Buffer buffer{ m_VkDevice };
		VkImage m_TextureImage;
		VkDeviceMemory m_TextureMemory;
		VkImageView m_ImageView;
		VkSampler m_Sampler;
		VkDescriptorSet m_DescriptorSet;
		uint32_t m_Width;
		uint32_t m_Height;
	};

	struct Vertex
	{
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec2 texCoords;
		glm::vec3 normal;
		glm::vec2 uv;
		

		static VkVertexInputBindingDescription getBindingDescription()
		{
			VkVertexInputBindingDescription description{};
			description.binding = 0;
			description.stride = sizeof(Vertex);
			description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			return description;
		}
		static std::array<VkVertexInputAttributeDescription, 5> getAttributeDescription()
		{
			std::array<VkVertexInputAttributeDescription, 5> attributeDescription{};
			attributeDescription[0].binding = 0;
			attributeDescription[0].location = 0;
			attributeDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescription[0].offset = offsetof(Vertex, pos);

			attributeDescription[1].binding = 0;
			attributeDescription[1].location = 1;
			attributeDescription[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescription[1].offset = offsetof(Vertex, color);

			attributeDescription[2].binding = 0;
			attributeDescription[2].location = 2;
			attributeDescription[2].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescription[2].offset = offsetof(Vertex, texCoords);

			attributeDescription[3].binding = 0;
			attributeDescription[3].location = 3;
			attributeDescription[3].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescription[3].offset = offsetof(Vertex, normal);

			attributeDescription[4].binding = 0;
			attributeDescription[4].location = 4;
			attributeDescription[4].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescription[4].offset = offsetof(Vertex, uv);

			return attributeDescription;
		}
	};

	class vkglTFModel
	{
	public:
		explicit vkglTFModel(std::shared_ptr<struct Vulkan_Device> device);
		

		struct
		{
			VkBuffer m_Buffer;
			VkDeviceMemory m_BufferMemory;
		}m_VertexBuffer;

		struct
		{
			VkBuffer m_Buffer;
			VkDeviceMemory m_BufferMemory;
		}m_IndexBuffer;

		struct Primitive
		{
			uint32_t m_FirstIndex;
			uint32_t m_IndexCount;
			int32_t  m_Materialindex;
		};

		struct Mesh
		{
			std::vector<Primitive> m_Primitives;
		};

		struct Node {
			Node* m_Parent;
			std::vector<Node*> m_Children;
			Mesh m_Mesh;
			glm::mat4 m_Matrix;
			~Node() {
				for (auto& child : m_Children) {
					delete child;
				}
			}
		};

		//std::vector<Vertex> m_Vertices;
		std::vector<Node*> m_Nodes;
		Texture m_Texture;

		void loadNode(const auto& inputNode, const auto& input, vkglTFModel::Node* parent, std::vector<uint32_t>& indexBuffer,
			std::vector<Vertex>& vertexBuffer);

		void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout,std::vector<VkDescriptorSet> sets, uint32_t index);

		void loadgltfFile(std::string fileName, std::vector<uint32_t>& indexBuffer, std::vector<Vertex>& vertexBuffer);
		
	private:
		std::shared_ptr<struct Vulkan_Device> m_VkDevice;
		
	};
} // namespace karhu