# Buddhabrot
## Compiling
`clang++ buddha.cpp -std=c++11 -o buddha`
## Generating Buddhabrot Data
`./buddha image_width image_height number_samples red_threshold green_threshold blue_threshold`
This will generate a `.buddhadata` file.
Example: `./buddha 2000 2000 10000000 500000 5000 500`
## Rendering Image
`./buddha image_width image_height`
The `image_width` and `image_height` values should be the same as the ones used to generate the `.buddhadata` file.
Example `./buddha 2000 2000`
