/**
 * @file
 * @brief Huffman encoding tree implementation based on std::priority_queue
 * @author Max Fomichev
 * @date 19.12.2016
 * @copyright Apache License v.2 (http://www.apache.org/licenses/LICENSE-2.0)
*/

#include <vector>
#include <queue>

#include "huffmanTree.hpp"

namespace w2v {
    void huffmanTree_t::buildTree(const std::vector<std::size_t> &_input, std::shared_ptr<node_t> &_tree) const {
        std::priority_queue<std::shared_ptr<node_t>, std::vector<std::shared_ptr<node_t>>, nodeCmp_t> tree;

        for (std::size_t i = 0; i < _input.size(); ++i) {
            tree.emplace(std::shared_ptr<leaf_t>(new leaf_t(i, _input[i])));
        }
        std::size_t branchID = 0;
        while (tree.size() > 1) {
            std::shared_ptr<node_t> left = tree.top();
            tree.pop();

            std::shared_ptr<node_t> right = tree.top();
            tree.pop();

            tree.emplace(std::shared_ptr<branch_t>(new branch_t(left, right, branchID++)));
        }
        _tree = tree.top();
        tree.pop();
    }

    void huffmanTree_t::generateCodes(const std::shared_ptr<node_t> &_node,
                       const huffmanCode_t &_code,
                       const huffmanPoint_t &_point) {
        if (const leaf_t *leaf = dynamic_cast<const leaf_t *>(_node.get())) {
            m_tree[leaf->m_index] = huffmanData_t(_code, _point);

        }  else if (const branch_t *branch = dynamic_cast<const branch_t *>(_node.get())) {
            auto leftPrefix = _code;
            leftPrefix.push_back(false);
            auto leftPoint = _point;
            leftPoint.push_back(branch->m_id);
            generateCodes(branch->m_left, leftPrefix, leftPoint);

            auto rightPrefix = _code;
            rightPrefix.push_back(true);
            auto rightPoint = _point;
            rightPoint.push_back(branch->m_id);
            generateCodes(branch->m_right, rightPrefix, rightPoint);
        }
    }
}