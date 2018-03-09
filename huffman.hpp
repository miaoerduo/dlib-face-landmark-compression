//
//  huffman.hpp
//  dlib_utils
//
//  Created by zhaoyu on 2018/1/8.
//  Copyright © 2018 zhaoyu. All rights reserved.

#ifndef huffman_h
#define huffman_h

#include <vector>
#include <unordered_map>
#include <algorithm>

namespace med {
    
    // A Huffman Tree Node
    struct HuffmanTree {
        int c; // character in an alphabet
        unsigned long cfreq; // frequency of c.
        struct HuffmanTree *left;
        struct HuffmanTree *right;
        HuffmanTree(int c, int cfreq, struct HuffmanTree *left=NULL,
                    struct HuffmanTree *right=NULL) :
        c(c), cfreq(cfreq), left(left), right(right) {
        }
        ~HuffmanTree() {
            delete left, delete right;
        }
        // Compare two tree nodes
        class Compare {
        public:
            bool operator()(HuffmanTree *a, HuffmanTree *b) {
                return a->cfreq > b->cfreq;
            }
        };
    };
    
    /**
     * Builds a Huffman Tree from an input of alphabet C, where C is a vector
     * of (character, frequency) pairs.
     */
    HuffmanTree *build_tree(std::vector< std::pair<int, unsigned long> > &alph) {
        // First build a min-heap
        // Build leaf nodes first
        std::priority_queue<HuffmanTree *, std::vector<HuffmanTree *>, HuffmanTree::Compare > alph_heap;
        for (auto it = alph.begin(); it != alph.end(); ++ it) {
            HuffmanTree *leaf = new HuffmanTree(it->first, it->second);
            alph_heap.push(leaf);
        }
        
        // HuffmanTree algorithm: Merge two lowest weight leaf nodes until
        // only one node is left (root).
        HuffmanTree *root = NULL;
        while (alph_heap.size() > 1) {
            HuffmanTree *l, *r;
            l = alph_heap.top();
            alph_heap.pop();
            r = alph_heap.top();
            alph_heap.pop();
            root = new HuffmanTree(0, l->cfreq + r->cfreq, l, r);
            alph_heap.push(root);
        }
        
        return root;
    }
    
    void destroy_tree(HuffmanTree *root) {
        delete root;
    }
    
    typedef std::vector<bool> code_t;
    typedef std::unordered_map<int, code_t> codetable;
    /**
     * Makes a lookup table (std::unordered_map) of (c -> code) from a HuffmanTree, where
     * code is an unsigned long representing the binary code.
     */
    std::unordered_map<int, code_t> build_lookup_table(HuffmanTree *htree) {
        codetable lookup;
        std::deque< std::pair<HuffmanTree *, code_t> > q;
        
        q.push_back(std::make_pair(htree, code_t()));
        while (!q.empty()) {
            HuffmanTree *node, *lc, *rc;
            code_t code;
            node = q.front().first;
            code = q.front().second;
            q.pop_front();
            lc = node->left;
            rc = node->right;
            if (lc) {
                // HuffmanTree is always full (either no children or two children)
                // Left child is appended a 0 and right child a 1.
                code_t code_cp(code);
                q.push_back(std::make_pair(lc, (code.push_back(0), code)));
                q.push_back(std::make_pair(rc, (code_cp.push_back(1), code_cp)));
            } else {
                // Leaf node: contains the character
                lookup.insert(std::make_pair(node->c, code));
            }
        }
        
        return lookup;
    }
    
    HuffmanTree * build_tree_from_lookup_table(const codetable &m) {
        HuffmanTree *root = new HuffmanTree(0, 0);
        for (auto &it: m) {
            HuffmanTree * t = root;
            for (int idx = 0; idx < it.second.size(); ++ idx) {
                if (it.second[idx] == false) {
                    // left
                    if (!t->left) t->left = new HuffmanTree(0, 0);
                    t = t->left;
                } else {
                    // right
                    if (!t->right) t->right = new HuffmanTree(0, 0);
                    t = t->right;
                }
            }
            t->c = it.first;
        }
        return root;
    }
}

#endif /* huffman_h */
