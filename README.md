## TODO
* Write an actual readme
* Add a gitignore for imgui.ini
* Figure out why the close button doesn't work
    - There's a whole bunch of validation errors generated when this happens so something isn't stopping properly
* Explore using the standard RenderPass workflow by recreating the graphics pipelines every frame. There is apparently a way to get around completely rebuilding by using the pipeline cache
    - Nonono, you just redo the vertex command buffer, that should be enough. If the pointers for the vertex and index buffers don't change then it might even be enough to modify the contents of the buffers.
    - Okay, try that first. Revert all the dynamic rendering stuff for now, and just move the top point of the triangle back and forth on a sinewave pattern.

Launch must be relative to the root directory (the one this file is in), so after building with `make`, run with `build/vk_template`
Enable validation layers by setting the following environment variable:
```sh
export VK_LOADER_LAYERS_ENABLE=*api_dump,*validation
```
`*api_dump` is extremely verbose, usually better to skip that one

