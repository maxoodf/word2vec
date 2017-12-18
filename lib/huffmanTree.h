/**
 * @file
 * @brief Huffman encoding tree implementation based on std::priority_queue
 * @author Max Fomichev
 * @date 19.12.2016
 * @copyright Apache License v.2 (http://www.apache.org/licenses/LICENSE-2.0)
*/

#ifndef WORD2VEC_HUFFMANTREE_H
#define WORD2VEC_HUFFMANTREE_H

#include <memory>

namespace w2v {
    /**
     * @brief huffmanTree class - Huffman encoding tree implementation based on std::priority_queue
     *
     * Input for a huffmanTree object is a vector of frequencies where vector index is the key and value is frequency
     * corresponding to the key. Output is a tree node with binary code (huffmanCode_t) and neighbors branch
     * IDs (huffmanPoint_t)
     * Read more - https://mitpress.mit.edu/sicp/full-text/sicp/book/node41.html
    */
    class huffmanTree_t final {
    public:
        using huffmanCode_t = std::vector<bool>; ///< huffman binary code type
        using huffmanPoint_t = std::vector<std::size_t>; ///< huffman parent branch IDs type
        /// Huffman tree output data structure
        struct huffmanData_t final {
            huffmanCode_t huffmanCode; ///< Huffman binary code
            huffmanPoint_t huffmanPoint; ///< Huffman tree parent branch IDs

            /// Constructs an empty object
            huffmanData_t() = default;
            /// Constructs an object with _huffmanCode and _huffmanPoint
            huffmanData_t(const huffmanCode_t &_huffmanCode,
                          const huffmanPoint_t &_huffmanPoint):
                    huffmanCode(_huffmanCode), huffmanPoint(_huffmanPoint) {}
        };

    private:
        /// Encoding tree node, base class
        class node_t  {
        public:
            const std::size_t m_frequency; ///< frequency value

        public:
            virtual ~node_t() = default;

            // copying prohibited
            node_t(const node_t &) = delete;
            void operator=(const node_t &) = delete;

        protected:
            explicit node_t(std::size_t _frequency): m_frequency(_frequency) {}
        };

        /// Structure to compare tree nodes
        struct nodeCmp_t final {
            inline bool operator()(const std::shared_ptr<node_t> &_left,
                                   const std::shared_ptr<node_t> &_right) const noexcept {
                return _left->m_frequency > _right->m_frequency;
            }
        };

        /// Encoding tree branch
        class branch_t final: public node_t {
        public:
            const std::shared_ptr<node_t> m_left; ///< left node
            const std::shared_ptr<node_t> m_right; ///< right node
            const std::size_t m_id; ///< branch ID

            /**
             * Constructs a branch object with neighbor nodes and unique ID
             * @param _left left node
             * @param _right right node
             * @param _id unique branch ID
             */
            branch_t(const std::shared_ptr<node_t> &_left,
                     const std::shared_ptr<node_t> &_right,
                     std::size_t _id):
                    node_t(_left->m_frequency + _right->m_frequency), m_left(_left), m_right(_right), m_id(_id) {}

            // copying prohibited
            branch_t(const branch_t &) = delete;
            void operator=(const branch_t &) = delete;
        };

        /// Encoding tree leaf
        class leaf_t final: public node_t {
        public:
            const std::size_t m_index; ///< input vector index (key of frequency value)

            /**
             * Constructs a leaf with input index and frequency
             * @param _index input vector index
             * @param _frequency input frequency of the _index element
             */
            leaf_t(std::size_t _index, std::size_t _frequency): node_t(_frequency), m_index(_index) {}

            // copying prohibited
            leaf_t(const leaf_t &) = delete;
            void operator=(const leaf_t &) = delete;
        };

        std::vector<huffmanData_t> m_tree; ///< Huffman tree (vector) where m_tree indexes are input vector indexes

    public:
        /**
         * Constructs a huffmanTree object for input frequencies vector
         * @param _input Input vector of frequencies to be encoded
         * @throws std::exception in case of a member initialisztion or tree building failed
         */
        explicit huffmanTree_t(const std::vector<std::size_t> &_input): m_tree(_input.size()) {
            std::shared_ptr<node_t> tree;
            buildTree(_input, tree);
            generateCodes(tree, huffmanCode_t(), huffmanPoint_t());
        }

        // copying prohibited
        huffmanTree_t(const huffmanTree_t &) = delete;
        void operator=(const huffmanTree_t &) = delete;

        /**
         *
         * @param[in] _index frequency index
         * @returns pointer to a huffmanData object with binary code and parent node IDs
         */
        inline const huffmanData_t *huffmanData(std::size_t _index) const noexcept {
            if (_index < m_tree.size()) {
                return &m_tree[_index];
            } else {
                return nullptr;
            }
        }

    private:
        void buildTree(const std::vector<std::size_t> &_input,
                       std::shared_ptr<node_t> &_tree) const;

        void generateCodes(const std::shared_ptr<node_t> &_node,
                           const huffmanCode_t &_code,
                           const huffmanPoint_t &_point);
    };
}

#endif // WORD2VEC_HUFFMANTREE_H
