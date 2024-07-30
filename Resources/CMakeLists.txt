juce_add_binary_data(M1-Notepad_binary NAMESPACE BinaryData SOURCES mach1logo.png)
target_link_libraries(M1-Notepad PRIVATE M1-Notepad_binary)