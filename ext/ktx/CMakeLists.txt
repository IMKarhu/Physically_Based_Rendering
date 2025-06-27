set(KTX_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(KTX_SOURCES
    ${KTX_DIR}/lib/texture.c
	#${KTX_DIR}/lib/texture1.c
	#${KTX_DIR}/lib/texture2.c
    ${KTX_DIR}/lib/hashlist.c
    ${KTX_DIR}/lib/checkheader.c
    ${KTX_DIR}/lib/swap.c
    ${KTX_DIR}/lib/memstream.c
    ${KTX_DIR}/lib/filestream.c
    ${KTX_DIR}/lib/vkloader.c)

add_library(ktxx STATIC ${KTX_SOURCES})

target_link_libraries(ktxx Vulkan::Vulkan)