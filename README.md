## TODO
* Write an actual readme
* Add a gitignore for imgui.ini
* Figure out why the close button doesn't work
    - There's a whole bunch of validation errors generated when this happens so something isn't stopping properly

Launch must be relative to the root directory (the one this file is in), so after building with `make`, run with `build/vk_template`
Enable validation layers by setting the following environment variable:
```sh
export VK_LOADER_LAYERS_ENABLE=*api_dump,*validation
```
`*api_dump` is extremely verbose, usually better to skip that one

