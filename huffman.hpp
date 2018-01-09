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
#include <string>
#include <cassert>
#include <stdexcept>

namespace med {
    
    using namespace std;
    
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
    HuffmanTree *build_tree(vector< pair<int, unsigned long> > &alph) {
        // First build a min-heap
        // Build leaf nodes first
        priority_queue<HuffmanTree *, vector<HuffmanTree *>, HuffmanTree::Compare > alph_heap;
        for (vector< pair<int, unsigned long> >::iterator it = alph.begin();
             it != alph.end(); ++it) {
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
    
    typedef vector<bool> code_t;
    typedef unordered_map<int, code_t> codetable;
    /**
     * Makes a lookup table (std::unordered_map) of (c -> code) from a HuffmanTree, where
     * code is an unsigned long representing the binary code.
     */
    unordered_map<int, code_t> build_lookup_table(HuffmanTree *htree) {
        codetable lookup;
        deque< pair<HuffmanTree *, code_t> > q;
        
        q.push_back(make_pair(htree, code_t()));
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
                q.push_back(make_pair(lc, (code.push_back(0), code)));
                q.push_back(make_pair(rc, (code_cp.push_back(1), code_cp)));
            } else {
                // Leaf node: contains the character
                lookup.insert(make_pair(node->c, code));
            }
        }
        
        return lookup;
    }
    
    HuffmanTree * build_tree_from_lookup_table(const codetable &m) {
        using namespace std;
        HuffmanTree *root = new HuffmanTree(0, 0);
        for (auto &it: m) {
            int k = it.first;
            code_t &bits = it.second;
            HuffmanTree * t = root;
            for (int idx = 0; idx < bits.size(); ++ idx) {
                if (bits[idx] == false) {
                    // left
                    if (!t->left) t->left = new HuffmanTree(0, 0);
                    t = t->left;
                } else {
                    // right
                    if (!t->right) t->right = new HuffmanTree(0, 0);
                    t = t->right;
                }
            }
            t->c = k;
        }
        return root;
    }

    
    /**
     * Encodes an input int array. returns a byte vector.
     */
    code_t encode(vector<int> input, codetable &lookup) {
        code_t result;
        
        for (auto it = input.begin(); it != input.end(); ++it) {
            code_t b = lookup[*it];
            result.insert(result.end(), b.begin(), b.end());
        }
        
        return result;
    }
    
    /**
     * Look up the next valid code in @biter using @htree and returns the
     * resulting string. Note the iterator @biter is advanced by the actual
     * length of the next valid code, which varies.
     */
    char code_lookup(code_t::iterator &biter, const code_t::iterator &biter_end,
                     const HuffmanTree *htree) {
        const HuffmanTree *node = htree;
        
        while (true) {
            if (!node->left) {
                // Huffman tree is full: always contains both children or none.
                break;
            }
            if (biter == biter_end) {
                throw std::out_of_range("No more bits");
            }
            if (*biter) {
                node = node->right;
            } else {
                node =node->left;
            }
            ++biter;
        }
        
        return node->c;
    }
    
    /**
     * Decodes a compressed string represented by a bit vector (vector<char>)
     * @compressed, using a HuffmanTree @htree.
     * Returns the original string.
     */
    string decode(code_t &compressed, const HuffmanTree *htree) {
        string result;
        
        code_t::iterator biter = compressed.begin();
        while (true) {
            try {
                result += code_lookup(biter, compressed.end(), htree);
            } catch (const std::out_of_range &oor) {
                // Iterator exhausted.
                break;
            }
        }
        
        return result;
    }
    
}

#endif /* huffman_h */
