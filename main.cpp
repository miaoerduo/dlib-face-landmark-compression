//
//  main.cpp
//  dlib_utils
//
//  Created by zhaoyu on 2018/1/8.
//  Copyright © 2018年 zhaoyu. All rights reserved.
//

#include <iostream>

#include <model_utils.hpp>
#include <dlib/image_processing.h>

int main(int argc, const char * argv[]) {
    
    if (argc != 3) {
        std::cout << "Usage: ./main.out src_path dest_path" << std::endl;
        return 0;
    }
    
    dlib::shape_predictor sp;
    
    // 加载模型
    dlib::deserialize(argv[1]) >> sp;
    
    // 压缩模型
    med::save_shape_predictor_model(sp, argv[2], 0.0001, 512);
    
    // 加载压缩模型
    dlib::shape_predictor sp2;
    med::load_shape_predictor_model(sp2, argv[2]);
    
    return 0;
}
