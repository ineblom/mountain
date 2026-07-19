# Dependencies

Third-party source and headers used by Mountain live in this directory.

- `freetype/`: FreeType 2.14.3 source, built locally by `freetype/build.sh`.
- `vulkan/`: Vulkan API headers used to compile the renderer.
- `wayland/`: generated XDG shell protocol source and header.

Platform frameworks supplied as part of the operating system are not copied
here. Runtime implementations such as a Linux Vulkan loader, MoltenVK, Wayland,
and xkbcommon are tracked separately until their source and transitive build
requirements are vendored.
