juce_add_binary_data(${CMAKE_PROJECT_NAME}_binary NAMESPACE BinaryData SOURCES mach1logo.png)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_PROJECT_NAME}_binary)