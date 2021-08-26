# dlib-face-landmark-compression

Dlib中的人脸landmark检测的模型压缩

具体介绍见博客：[https://www.miaoerduo.com/2018/01/08/dlib-landmark-model-compression/](https://www.miaoerduo.com/2018/01/08/dlib-landmark-model-compression/)

使用前，需要修改dlib的源码：
DLIB_PATH/include/dlib/image_processing/image_processing/shape_predictor.h

修改 shape_predictor 类的成员变量的属性为public。

**另外，有同学反映说不能正常加载模型，后来换了dlib 19.8之后，就可以了。这里也请大家出现类似的问题注意一下。**

**请注意，压缩之后的模型，存储格式发生变化，因此是不能通过dlib原本的方式去加载的，要换成下面的方式：**

```
dlib::shape_predictor sp;
med::load_shape_predictor_model(sp, "/path/to/compressed_model");
```

需要使用该项目的同学，只需要加`huffman.hpp`和`model_utils.hpp`加入项目中即可。

编译方式如下，建议每个人先运行`main.cpp`的Demo：

```
git clone https://github.com/miaoerduo/dlib-face-landmark-compression.git
cd dlib-face-landmark-compression
g++ main.cpp -o main.bin -O2 -I ./ -I DLIB_PATH/include -L DLIB_PATH/lib -ldlib -std=c++11
./main.bin src_dlib_shape_predictor_model dest_model
```

为了方便大家的调试，这里上传一个dlib的68点landmark的原模型和使用main.cpp的代码压缩之后的模型。

链接: https://pan.baidu.com/s/1z1Sh-ljCBrV_Rorsn2eOxA 提取码: t5mc
