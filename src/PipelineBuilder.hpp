#pragma once

#include "vulkan/vulkan.h"

#include <string>
#include <vector>
#include <memory>

namespace karhu
{
    class Device;

    struct PipelineStruct
    {
        VkViewport viewport{};
        float viewportWidth;
        float viewportheight;
        VkRect2D scissor{};
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        VkPipelineDynamicStateCreateInfo dynamiccreateinfo{};
        VkPipelineViewportStateCreateInfo viewportState{};
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        VkPipelineMultisampleStateCreateInfo multisampling{};
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        VkPipelineColorBlendStateCreateInfo colorBlending{};
        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        VkGraphicsPipelineCreateInfo pipelineInfo{};
        VkRenderPass renderPass;
    };

    struct Pipeline
    {
        VkDevice m_device;
        VkPipelineLayout m_pipelineLayout;
        VkPipeline m_graphicsPipeline;
        std::vector<VkDynamicState> m_dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
    };
    
    class PipelineBuilder
    {
        public:
            // PipelineBuilder(Device& device);
            virtual ~PipelineBuilder() {}

            // PipelineBuilder(const PipelineBuilder&) = delete;
            // void operator=(const PipelineBuilder&) = delete;
            // PipelineBuilder(PipelineBuilder&&) = delete;
            // PipelineBuilder& operator=(PipelineBuilder&&) = delete;

            virtual void createPipeline(PipelineStruct pipelineStruct,
                    const std::string& vertfilePath,
                    const std::string& fragfilePath) const = 0;

            virtual void bind(VkCommandBuffer commandBuffer) const = 0;

            // const VkPipeline& getPipeline() const  { return m_graphicsPipeline; }
            // const VkPipelineLayout& getPipelineLayout() const { return m_pipelineLayout; }
        private:
            // static std::vector<char> readFile(const std::string& fileName);
            // VkShaderModule createShaderModule(const std::vector<char>& code);
        private:
            // VkPipelineLayout m_pipelineLayout;
            // VkPipeline m_graphicsPipeline;
            
            // std::vector<VkDynamicState> m_dynamicStates = {
            //     VK_DYNAMIC_STATE_VIEWPORT,
            //     VK_DYNAMIC_STATE_SCISSOR
            // };
            
            // Device& m_device;
    };

    class NormalPipelineBuilder : public PipelineBuilder
    {
        public:
            NormalPipelineBuilder();
            ~NormalPipelineBuilder();

            void createPipeline(PipelineStruct pipelineStruct,
                    const std::string& vertfilePath,
                    const std::string& fragfilePath) const override;

            void bind(VkCommandBuffer commandbuffer) const override;

            std::unique_ptr<Pipeline>& getHandle() { return m_pipeline; }
        private:
            static std::vector<char> readFile(const std::string& fileName);
            VkShaderModule createShaderModule(const std::vector<char>& code) const;
        private:
            std::unique_ptr<Pipeline> m_pipeline;

    };

    class CubePipelineBuilder : public PipelineBuilder
    {
        public:
            CubePipelineBuilder();
            ~CubePipelineBuilder();

            void createPipeline(PipelineStruct pipelineStruct,
                    const std::string& vertfilePath,
                    const std::string& fragfilePath) const override;

            void bind(VkCommandBuffer commandBuffer) const override;

            std::unique_ptr<Pipeline>& getHandle() { return m_pipeline; }

        private:
            static std::vector<char> readFile(const std::string& fileName);
            VkShaderModule createShaderModule(const std::vector<char>& code) const;
        private:
            std::unique_ptr<Pipeline> m_pipeline;
    };

    
} // karhu namespace
