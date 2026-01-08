# Rendering/renderer
- [ ] * Implement textures
	- [ ] Texture compression?
- [ ] * Render queue sorting (shader, mesh)
	- Render data (vao) should \*probably* exist in the render item itself, makes sorting so much easier and offloads responsibility of fetching the mesh data away from the renderer, cleaner
- [ ] Implement separate render thread
- [ ] Implement shadows, light
# 2D/UI
- [ ] * Simple text rendering
	- [ ] Font rendering
	- [ ] Basic 2D rendering (overlay?)
# Assets
- [ ] * Better asset loading
	- [ ] API should be like `debug::`, `transform::` APIs
- [ ] *** Dependency graph for asset loading, use files instead of hardcoding
- [ ] 