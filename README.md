# Evolutionary image compressor
Compresses images using evolutionary computation. 

Image (bmp format only) is approximated as voronoi diagram and then improved either local search, evolutionary algorithm or memetic algorithm. So far local search provides the best results.

Computation uses is accelerated by CUDA.

Documentation can be found ![here][1].

## Example outputs 
![](test_images/Mona_Lisa.bmp)
![](test_images/kubismus_krajina.bmp)
![](test_images/abstraktni_krivky.bmp)

![](test_output/Mona_lisa.bmp)
![](test_output/kubismus_krajina_3337.bmp)
![](test_output/abstraktni_krivky_1894.bmp)


## License

    Copyright 2016 Jakub Petriska

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
	
	
[1]: https://jakubpetriska.github.io/Evolutionary-image-compressor/doc/html/index.html