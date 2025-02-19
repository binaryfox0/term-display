term-display
=====
`term-display` is a **simulated console graphics** library written in C. It allows you to display a simple pixelated screen on your terminal.

## Feature
Minimal dependencies requirements make it ideal for small, experimental projects.

- Render a pixel-based screen into your terminal.
- Compatible with third-party image loaders such as `stb_image`.

## Building
1. Clone the repository and download [premake5](https://premake.github.io/download)
2. Run `premake5 <action>` in the top directory, `<action>` (can be vs2022, gmake,...)
3. Then run the compiler/project builder based on your previous option.
4. All binaries are deposited in bin/

## Contributions
Contributions are welcomed! Feel free to open an issue or submit a pull request with improvements, bug fixing, and suggestions. Your contributions to this project are very valuable for the future of this project.

## Important Notes
If you are considering using `term-display` in a **serious project**, it's better to use an established graphics API/Library such as:
 - SDL, SFML
 - DirectX/Direct3D 11 (Windows only)
 - Vulkan (cross-platform)
### Issues
Currently known issues with `term-display`
 - **Bad Performance**: Rendering complex or large pixel grids in the terminal using `term-display` can be slower compared to alternatives. It is especially worse on virtual environments such as `termux` on mobile, which isn't fully optimized.
 - **Low Resolution**: Terminal-based output is limited by the size, resolution of the terminal window.
 - Some user event handling problems
 - Bad code structure
