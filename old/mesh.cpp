#include "mesh.h"

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
	:m_Vertices(vertices),
	m_Indices(indices)
{
}

//Mesh* Mesh::processMesh(aiMesh* mesh)
//{
//	std::vector<Vertex> vertices;//TODO 1: add data containers for vertices and indices
//	std::vector<unsigned int> indices;
//	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
//	{
//		Vertex vertex; //temporable container for the data of each loop
//		glm::vec3 vector;//TODO 2: load data from the Assimp mesh to our containers
//
//		vector.x = mesh->mVertices[i].x;
//		vector.y = mesh->mVertices[i].y;
//		vector.z = mesh->mVertices[i].z;
//		vertex.pos = vector;
//
//		vector.x = mesh->mNormals[i].x;
//		vector.y = mesh->mNormals[i].y;
//		vector.z = mesh->mNormals[i].z;
//		vertex.normals = vector;
//
//		if (mesh->mTextureCoords[0])
//		{
//			glm::vec2 vec;
//			vec.x = mesh->mTextureCoords[0][i].x;
//			vec.y = mesh->mTextureCoords[0][i].y;
//			vertex.texCoord = vec;
//		}
//		else
//		{
//			vertex.texCoord = glm::vec2(0.0f, 0.0f);
//		}
//		vertices.push_back(vertex);
//	}
//	//Now retrieve the corresponding vertex indices
//	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
//	{
//		aiFace face = mesh->mFaces[i];
//		for (unsigned int j = 0; j < face.mNumIndices; j++)
//		{
//			indices.push_back(face.mIndices[j]);
//		}
//	}
//
//	return new Mesh(vertices, indices);//TODO 3: return with new Mesh of our own
//}
//
//void Mesh::processNode(std::vector<Mesh*>* meshes, aiNode* node, const aiScene* scene)
//{
//	// process each mesh located at the current node
//	for (unsigned int i = 0; i < node->mNumMeshes; i++)
//	{
//		// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
//		// the node object only contains indices to index the actual objects in the scene.
//		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
//		meshes->push_back(processMesh(mesh));
//	}
//	// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
//	for (unsigned int i = 0; i < node->mNumChildren; i++)
//	{
//		processNode(meshes, node->mChildren[i], scene);
//	}
//}
//
//std::vector<Mesh*> Mesh::loadMesh(const std::string& path)
//{
//	std::vector<Mesh*> meshes;//TODO 1: create the container that will be returned by this function
//	//read file with Assimp
//	Assimp::Importer importer;
//	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
//	//Check for errors
//	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
//	{
//		printf("Error loading model file \"%s\": \"%s\" ", path.c_str(), importer.GetErrorString());
//		return meshes;
//	}
//	// retrieve the directory path of the filepath
//	std::string dir = path.substr(0, path.find_last_of('/'));
//	//TODO 2: process Assimp's root node recursively
//	processNode(&meshes, scene->mRootNode, scene);
//	return meshes;
//}
