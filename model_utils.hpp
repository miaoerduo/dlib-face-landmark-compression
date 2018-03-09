//
//  model_utils.hpp
//  dlib_utils
//
//  Created by zhaoyu on 2018/1/8.
//  Copyright © 2018 zhaoyu. All rights reserved.
//

#ifndef model_utils_h
#define model_utils_h

#include <vector>
#include <unordered_map>
#include <fstream>
#include <cmath>
#include <dlib/image_processing.h>
#include <huffman.hpp>


namespace med {
    
    /**
     *  data type definition
     */
    typedef char        int8;
    typedef short       int16;
    typedef int         int32;
    typedef long long   int64;
    typedef float       float32;
    typedef double      float64;
    typedef unsigned char       uint8;
    typedef unsigned short      uint16;
    typedef unsigned int        uint32;
    typedef unsigned long long  uint64;
    
    /**
     *  utils for reading and writing
     */
    template <typename T>
    void write_single_value(std::ofstream &os, const T &data) {
        os.write(reinterpret_cast<const char *>(&data), sizeof(T));
    }
    
    template <typename T>
    void read_single_value(std::ifstream &is, T &data) {
        is.read(reinterpret_cast<char *>(&data), sizeof(T));
    }
    
    void write_char_vec(std::ofstream &os, const std::vector<char> &data) {
        os.write(&data[0], data.size());
    }
    
    void read_char_vec(std::ifstream &is, std::vector<char> &data, unsigned long size) {
        std::vector<char> _data(size, 0);
        is.read(reinterpret_cast<char *>(&_data[0]), size * sizeof(char));
        data.swap(_data);
    }
    
    void read_float_vec(std::ifstream &is, std::vector<float> &data, unsigned long size) {
        std::vector<float> _data(size, 0);
        is.read(reinterpret_cast<char *>(&_data[0]), size * sizeof(float));
        data.swap(_data);
    }
    
    void bits_to_chars(const std::vector<bool> &bits, std::vector<char> &out) {
        unsigned long bits_num = bits.size();
        std::vector<char> data((bits_num + 7)/ 8, 0);
        
        for (int idx = 0; idx < bits_num; ++ idx) {
            int c = idx / 8;
            int p = idx % 8;
            int v = bits[idx];
            data[c] |= (v << (7 - p));
        }
        out.swap(data);
    }
    
    void chars_to_bits(const std::vector<char> &chars, std::vector<bool> &out, unsigned long bit_num) {
        std::vector<bool> data(bit_num, false);
        for (int idx = 0; idx < bit_num; ++ idx) {
            int c = idx / 8;
            int p = idx % 8;
            data[idx] = chars[c] & (1 << (7 - p));
        }
        out.swap(data);
    }
    
    /**
     *  compress shape predictor model
     */
    void save_shape_predictor_model(
        dlib::shape_predictor &sp,
        const std::string &save_path,
        const float _prune_thresh=0.0001,
        const unsigned long long _quantization_num=512,
        const unsigned long long _version=0) {
        
        using namespace std;
        std::ofstream os(save_path, std::ofstream::binary);
        
        /**
         *  const value
         */
        const uint64 version = _version;
        const uint64 cascade_depth = sp.forests.size();
        const uint64 num_trees_per_cascade_level = sp.forests[0].size();
        const uint64 num_leaves = sp.forests[0][0].num_leaves();
        const uint64 tree_depth = static_cast<uint64>(std::log(num_leaves) / std::log(2));
        const uint64 feature_pool_size = sp.anchor_idx[0].size();
        const uint64 landmark_num = sp.initial_shape.size() / 2;
        const uint64 quantization_num = _quantization_num;
        const float32 prune_thresh = _prune_thresh;
        
        
        /**
         *  save const value
         */
        unsigned long data_length = 7 * sizeof(uint64) + sizeof(float32);
        write_single_value(os, data_length);
        write_single_value(os, version);
        write_single_value(os, cascade_depth);
        write_single_value(os, num_trees_per_cascade_level);
        write_single_value(os, tree_depth);
        write_single_value(os, feature_pool_size);
        write_single_value(os, landmark_num);
        write_single_value(os, quantization_num);
        write_single_value(os, prune_thresh);
        
        /**
         *  initial_shape
         */
        data_length = landmark_num * 2 * sizeof(float32);
        write_single_value(os, data_length);
        
        for (auto it = sp.initial_shape.begin(); it != sp.initial_shape.end(); ++ it) {
            auto data = static_cast<float32>(*it);
            write_single_value(os, data);
        }
        
        /**
         *  anchor_idx
         */
        data_length = cascade_depth * feature_pool_size * sizeof(uint8);
        write_single_value(os, data_length);
        
        for (int r = 0; r < cascade_depth; ++ r) {
            for (int c = 0; c < feature_pool_size; ++ c) {
                uint8 idx = static_cast<uint8>(sp.anchor_idx[r][c]);
                write_single_value(os, idx);
            }
        }
        
        /**
         *  deltas
         */
        data_length = cascade_depth * feature_pool_size * 2 * sizeof(float32);
        write_single_value(os, data_length);
        for (int r = 0; r < cascade_depth; ++ r) {
            for (int c = 0; c < feature_pool_size; ++ c) {
                write_single_value(os, static_cast<float32>(sp.deltas[r][c](0)));
                write_single_value(os, static_cast<float32>(sp.deltas[r][c](1)));
            }
        }
        
        /**
         *  forests
         */
        
        /**
         *  splits
         */
        data_length = cascade_depth * num_trees_per_cascade_level * (num_leaves - 1) * (sizeof(uint16) + sizeof(uint16) + sizeof(float32));
        write_single_value(os, data_length);
        for (int r = 0; r < cascade_depth; ++ r) {
            for (int c = 0; c < num_trees_per_cascade_level; ++ c) {
                auto &tree = sp.forests[r][c];
                auto &splits = tree.splits;
                
                for (auto &split: splits) {
                    uint16 idx1 = static_cast<uint16>(split.idx1);
                    uint16 idx2 = static_cast<uint16>(split.idx2);
                    float32 thresh = static_cast<float32>(split.thresh);
                    write_single_value(os, idx1);
                    write_single_value(os, idx2);
                    write_single_value(os, thresh);
                }
            }
        }
        
        /**
         *  leaf values
         */
        
        /*** step 1 prune ***/
        float leaf_min_value = 100000., leaf_max_value = -100000.;
        const unsigned long leaf_value_num = landmark_num * 2;
        for (int r = 0; r < cascade_depth; ++ r) {
            for (int c = 0; c < num_trees_per_cascade_level; ++ c) {
                auto &tree = sp.forests[r][c];
                auto &leaf_values = tree.leaf_values;
                for (auto &leaf_value: leaf_values) {
                    for (int idx = 0; idx < leaf_value_num; ++ idx) {
                        float v = leaf_value(idx);
                        if (v > leaf_max_value) leaf_max_value = v;
                        if (v < leaf_min_value) leaf_min_value = v;
                        if (std::fabs(v) < prune_thresh) leaf_value(idx) = 0.;
                    }
                }
            }
        }
        
        /*** step2 quantization ***/
        float32 quantization_precision = (leaf_max_value - leaf_min_value) / quantization_num;
        
        unordered_map<int, unsigned long> quantization_frequency;   // count frequency
        
        for (int r = 0; r < cascade_depth; ++ r) {
            for (int c = 0; c < num_trees_per_cascade_level; ++ c) {
                auto &tree = sp.forests[r][c];
                auto &leaf_values = tree.leaf_values;
                for (auto &leaf_value: leaf_values) {
                    for (int idx = 0; idx < leaf_value_num; ++ idx) {
                        int quantization_value = static_cast<int>(std::round(leaf_value(idx) / quantization_precision));
                        ++ quantization_frequency[quantization_value];
                    }
                }
            }
        }
        
        /*** step3 huffman ***/
        
        vector<pair<int, unsigned long> >cfvec(quantization_frequency.begin(), quantization_frequency.end());
        
        HuffmanTree *htree = build_tree(cfvec);
        codetable ctbl = build_lookup_table(htree);
        
        destroy_tree(htree);
        htree = NULL;
        
        /*** step4 save the code table ***/
        
        data_length = sizeof(float32) + sizeof(uint64);    //quantization_precision + ctbl size
        for (auto it: ctbl) {
            // value + code_size + code_t
            data_length += sizeof(int) + sizeof(uint8) + (it.second.size() + 7) / 8;
        }
        write_single_value(os, data_length);
        write_single_value(os, quantization_precision);
        uint64 ctbl_size = ctbl.size();
        write_single_value(os, ctbl_size);
        
        // code table content
        for (auto it: ctbl) {
            int k = it.first;
            vector<bool> bits = it.second;
            uint8 bits_num = static_cast<uint8>(bits.size());
            std::vector<char> data;
            bits_to_chars(bits, data);
            write_single_value(os, k);
            write_single_value(os, bits_num);
            write_char_vec(os, data);
        }
        
        /*** step5 encode ***/
        vector<bool> encode_data_bin;
        for (int r = 0; r < cascade_depth; ++ r) {
            for (int c = 0; c < num_trees_per_cascade_level; ++ c) {
                auto &tree = sp.forests[r][c];
                auto &leaf_values = tree.leaf_values;
                for (auto &leaf_value: leaf_values) {
                    for (int idx = 0; idx < leaf_value_num; ++ idx) {
                        int quantization_value = static_cast<int>(std::round(leaf_value(idx) / quantization_precision));
                        auto code = ctbl[quantization_value];
                        encode_data_bin.insert(encode_data_bin.end(), code.begin(), code.end());
                    }
                }
            }
        }
        
        std::vector<char> encode_data;
        bits_to_chars(encode_data_bin, encode_data);
        
        data_length = encode_data.size();
        write_single_value(os, data_length);
        write_char_vec(os, encode_data);
    }
    
    /**
     *  load a compressed shape predictor model
     */
    void load_shape_predictor_model(dlib::shape_predictor &sp, const std::string&filename) {
        
        using namespace std;
        std::ifstream is(filename, std::ofstream::binary);
        
        /**
         *  constant values
         */
        uint64 data_length;
        uint64 version;
        uint64 cascade_depth;
        uint64 num_trees_per_cascade_level;
        uint64 tree_depth;
        uint64 feature_pool_size;
        uint64 landmark_num;
        uint64 quantization_num;
        float32 prune_thresh;
        
        read_single_value(is, data_length);
        read_single_value(is, version);
        read_single_value(is, cascade_depth);
        read_single_value(is, num_trees_per_cascade_level);
        read_single_value(is, tree_depth);
        read_single_value(is, feature_pool_size);
        read_single_value(is, landmark_num);
        read_single_value(is, quantization_num);
        read_single_value(is, prune_thresh);
        
        /**
         *  initial shape
         */
        read_single_value(is, data_length);
        
        sp.initial_shape.set_size(landmark_num * 2, 1);
        
        {
            std::vector<float> data;
            read_float_vec(is, data, landmark_num * 2);
            for (int idx = 0; idx < landmark_num * 2; ++ idx) {
                sp.initial_shape(idx) = data[idx];
            }
        }
        
        /**
         *  anchor_idx
         */
        read_single_value(is, data_length);
        sp.anchor_idx = vector<vector<unsigned long> >(
            cascade_depth, vector<unsigned long>(feature_pool_size, 0));
        
        {
            vector<char> data;
            for (int r = 0; r < cascade_depth; ++ r) {
                read_char_vec(is, data, feature_pool_size);
                for (int c = 0; c < feature_pool_size; ++ c) {
                    sp.anchor_idx[r][c] = static_cast<unsigned long>(data[c]);
                }
            }
        }
        
        /**
         *  deltas
         */
        read_single_value(is, data_length);
        sp.deltas = std::vector<std::vector<dlib::vector<float,2> > >(
            cascade_depth, std::vector<dlib::vector<float,2> >(
                feature_pool_size, dlib::vector<float,2>()));

        {
            std::vector<float> data;
            read_float_vec(is, data, cascade_depth * feature_pool_size * 2);
            for (int r = 0; r < cascade_depth; ++ r) {
                for (int c = 0; c < feature_pool_size; ++ c) {
                    int idx = r * feature_pool_size + c;
                    sp.deltas[r][c](0) = data[idx * 2];
                    sp.deltas[r][c](1) = data[idx * 2 + 1];
                }
            }
        }
        
        /**
         *  forests
         */
        
        /**
         *  splits
         */
        
        read_single_value(is, data_length);
        
        sp.forests = std::vector<std::vector<dlib::impl::regression_tree> >(
            cascade_depth, std::vector<dlib::impl::regression_tree>(
                num_trees_per_cascade_level, dlib::impl::regression_tree()));
        
        unsigned long split_num = static_cast<unsigned long>(std::round(pow(2, tree_depth) - 1));
        uint16 idx1, idx2;
        float32 thresh;
        for (int r = 0; r < cascade_depth; ++ r) {
            for (int c = 0; c < num_trees_per_cascade_level; ++ c) {
                auto &tree = sp.forests[r][c];
                auto &splits = tree.splits;
                dlib::impl::split_feature fea;
                for (int idx = 0; idx < split_num; ++ idx) {
                    read_single_value(is, idx1);
                    read_single_value(is, idx2);
                    read_single_value(is, thresh);
                    fea.idx1 = static_cast<unsigned long>(idx1);
                    fea.idx2 = static_cast<unsigned long>(idx2);
                    fea.thresh = static_cast<float>(thresh);
                    splits.push_back(fea);
                }
                splits.reserve(splits.size());
            }
        }
        
        
        /**
         *  leaf values
         */
        
        /*** code table ***/
        read_single_value(is, data_length);
        float32 quantization_precision = 0.;
        read_single_value(is, quantization_precision);
        uint64 ctbl_size;
        read_single_value(is, ctbl_size);
        
        unordered_map<int, std::vector<bool> > ctbl;
        
        while (ctbl_size > 0) {
            int k;
            uint8 bit_num;
            std::vector<char> data;
            read_single_value(is, k);
            read_single_value(is, bit_num);
            read_char_vec(is, data, (bit_num + 7) / 8);
            std::vector<bool> bits;
            chars_to_bits(data, bits, bit_num);
            ctbl[k] = bits;
            -- ctbl_size;
        }
        
        auto *huffman_tree = build_tree_from_lookup_table(ctbl);
        
        /*** read and decode leaf values ***/
        read_single_value(is, data_length);
        
        {
            std::vector<char> leaf_value_chars;
            read_char_vec(is, leaf_value_chars, data_length);
            
            uint64 num_leaves = static_cast<unsigned long>(std::round(pow(2, tree_depth)));
            uint64 offset = 0;
            
            for (int c = 0; c < cascade_depth; ++ c) {
                for (int r = 0; r < num_trees_per_cascade_level; ++ r) {
                    auto &tree = sp.forests[c][r];
                    auto &leaf_values = tree.leaf_values;
                    for (int leaf_value_idx = 0; leaf_value_idx < num_leaves; ++ leaf_value_idx) {
                        dlib::matrix<float,0,1> _leaf;
                        _leaf.set_size(landmark_num * 2, 1);
                        
                        for (int _idx = 0; _idx < landmark_num * 2; ++ _idx) {
                            auto *t = huffman_tree;
                            while (t->left || t->right) {
                                int c = offset / 8;
                                int p = offset % 8;
                                bool v = leaf_value_chars[c] & (1 << (7 - p));
                                if (!v) t = t->left;
                                else t = t->right;
                                ++ offset;
                            }
                            _leaf(_idx) = t->c * quantization_precision;
                        }
                        
                        leaf_values.push_back(_leaf);
                    }
                    leaf_values.reserve(leaf_values.size());
                }
            }
        }
        
        destroy_tree(huffman_tree);
    }
    
}

#endif /* model_utils_h */
