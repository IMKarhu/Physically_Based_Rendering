#include "kModel.hpp"
#include <iostream>

#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_IMPLEMENTATION
#include <tiny_gltf.h>


namespace karhu
{
	vkglTFModel::vkglTFModel(Vulkan_Device& device)
		:m_VkDevice(device)
		,m_Texture(device)
	{
	}

	vkglTFModel::~vkglTFModel()
	{
		/*vkDestroyBuffer(m_VkDevice.m_Device, m_IndexBuffer.m_Buffer, nullptr);
		vkFreeMemory(m_VkDevice.m_Device, m_IndexBuffer.m_BufferMemory, nullptr);
		vkDestroyBuffer(m_VkDevice.m_Device, m_VertexBuffer.m_Buffer, nullptr);
		vkFreeMemory(m_VkDevice.m_Device, m_VertexBuffer.m_BufferMemory, nullptr);
		*/
	}

	void vkglTFModel::loadNode(const auto& inputNode, const auto& input, vkglTFModel::Node* parent, std::vector<uint32_t>& indexBuffer, std::vector<Vertex>& vertexBuffer)
	{
		vkglTFModel::Node* node = new vkglTFModel::Node{};
		node->m_Matrix = glm::mat4(1.0f);
		node->m_Parent = parent;

		// Get the local node matrix
		// It's either made up from translation, rotation, scale or a 4x4 matrix
		if (inputNode.translation.size() == 3) {
			node->m_Matrix = glm::translate(node->m_Matrix, glm::vec3(glm::make_vec3(inputNode.translation.data())));
		}
		if (inputNode.rotation.size() == 4) {
			glm::quat q = glm::make_quat(inputNode.rotation.data());
			node->m_Matrix *= glm::mat4(q);
		}
		if (inputNode.scale.size() == 3) {
			node->m_Matrix = glm::scale(node->m_Matrix, glm::vec3(glm::make_vec3(inputNode.scale.data())));
		}
		if (inputNode.matrix.size() == 16) {
			node->m_Matrix = glm::make_mat4x4(inputNode.matrix.data());
		};

		// Load node's children
		if (inputNode.children.size() > 0) {
			for (size_t i = 0; i < inputNode.children.size(); i++) {
				loadNode(input.nodes[inputNode.children[i]], input, node, indexBuffer, vertexBuffer);
			}
		}

		// If the node contains mesh data, we load vertices and indices from the buffers
		// In glTF this is done via accessors and buffer views
		if (inputNode.mesh > -1) {
			const tinygltf::Mesh mesh = input.meshes[inputNode.mesh];
			// Iterate through all primitives of this node's mesh
			for (size_t i = 0; i < mesh.primitives.size(); i++) {
				const tinygltf::Primitive& glTFPrimitive = mesh.primitives[i];
				uint32_t firstIndex = static_cast<uint32_t>(indexBuffer.size());
				uint32_t vertexStart = static_cast<uint32_t>(vertexBuffer.size());
				uint32_t indexCount = 0;
				// Vertices
				{
					const float* positionBuffer = nullptr;
					const float* normalsBuffer = nullptr;
					const float* texCoordsBuffer = nullptr;
					size_t vertexCount = 0;

					// Get buffer data for vertex positions
					if (glTFPrimitive.attributes.find("POSITION") != glTFPrimitive.attributes.end()) {
						const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("POSITION")->second];
						const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
						positionBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
						vertexCount = accessor.count;
					}
					// Get buffer data for vertex normals
					if (glTFPrimitive.attributes.find("NORMAL") != glTFPrimitive.attributes.end()) {
						const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("NORMAL")->second];
						const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
						normalsBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
					}
					// Get buffer data for vertex texture coordinates
					// glTF supports multiple sets, we only load the first one
					if (glTFPrimitive.attributes.find("TEXCOORD_0") != glTFPrimitive.attributes.end()) {
						const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("TEXCOORD_0")->second];
						const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
						texCoordsBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
					}

					// Append data to model's vertex buffer
					for (size_t v = 0; v < vertexCount; v++) {
						Vertex vert{};
						vert.pos = glm::vec4(glm::make_vec3(&positionBuffer[v * 3]), 1.0f);
						vert.normal = glm::normalize(glm::vec3(normalsBuffer ? glm::make_vec3(&normalsBuffer[v * 3]) : glm::vec3(0.0f)));
						vert.uv = texCoordsBuffer ? glm::make_vec2(&texCoordsBuffer[v * 2]) : glm::vec3(0.0f);
						vert.color = glm::vec3(1.0f);
						vertexBuffer.push_back(vert);
					}
				}
				// Indices
				{
					const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.indices];
					const tinygltf::BufferView& bufferView = input.bufferViews[accessor.bufferView];
					const tinygltf::Buffer& buffer = input.buffers[bufferView.buffer];

					indexCount += static_cast<uint32_t>(accessor.count);

					// glTF supports different component types of indices
					switch (accessor.componentType) {
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
						const uint32_t* buf = reinterpret_cast<const uint32_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
						for (size_t index = 0; index < accessor.count; index++) {
							indexBuffer.push_back(buf[index] + vertexStart);
						}
						break;
					}
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
						const uint16_t* buf = reinterpret_cast<const uint16_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
						for (size_t index = 0; index < accessor.count; index++) {
							indexBuffer.push_back(buf[index] + vertexStart);
						}
						break;
					}
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
						const uint8_t* buf = reinterpret_cast<const uint8_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
						for (size_t index = 0; index < accessor.count; index++) {
							indexBuffer.push_back(buf[index] + vertexStart);
						}
						break;
					}
					default:
						std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
						return;
					}
				}
				Primitive primitive{};
				primitive.m_FirstIndex = firstIndex;
				primitive.m_IndexCount = indexCount;
				primitive.m_Materialindex = glTFPrimitive.material;
				node->m_Mesh.m_Primitives.push_back(primitive);
			}
		}

		if (parent) {
			parent->m_Children.push_back(node);
		}
		else {
			m_Nodes.push_back(node);
		}
	}
	void vkglTFModel::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, std::vector<VkDescriptorSet> sets, uint32_t index)
	{
		for (auto& node : m_Nodes)
		{
			if (node->m_Mesh.m_Primitives.size() > 0)
			{
				glm::mat4 nodeMatrix = node->m_Matrix;
				Node* currentParent = node->m_Parent;
				while (currentParent) {
					nodeMatrix = currentParent->m_Matrix * nodeMatrix;
					currentParent = currentParent->m_Parent;
				}
				// Pass the final matrix to the vertex shader using push constants
				//vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &nodeMatrix);
				for (Primitive& primitive : node->m_Mesh.m_Primitives) {
					if (primitive.m_IndexCount > 0) {
						// Get the texture index for this primitive
						//Texture texture = textures[materials[primitive.materialIndex].baseColorTextureIndex];
						// Bind the descriptor for the current primitive's texture
						vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &sets[index], 0, nullptr);
						vkCmdDrawIndexed(commandBuffer, primitive.m_IndexCount, 1, primitive.m_FirstIndex, 0, 0);
					}
				}
			}
			for (auto& child : node->m_Children)
			{
				draw(commandBuffer, pipelineLayout, sets, index);
			}
		}
	}

	void vkglTFModel::loadgltfFile(std::string fileName, std::vector<uint32_t>& indexBuffer, std::vector<Vertex>& vertexBuffer)
	{
		tinygltf::Model input;
		tinygltf::TinyGLTF gltfContext;
		std::string error, warning;


		bool fileLoad = gltfContext.LoadASCIIFromFile(&input, &error, &warning, fileName);

		if (fileLoad)
		{
			const tinygltf::Scene& scene = input.scenes[0];
			for (size_t i = 0; i < scene.nodes.size(); i++)
			{
				const tinygltf::Node node = input.nodes[scene.nodes[i]];
				loadNode(node, input, nullptr, indexBuffer, vertexBuffer);
			}
			
		}
	}
	
} // namespace karhu