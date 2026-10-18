# Huffman_compression
A C++ command-line tool that compresses and decompresses files using Huffman coding, a lossless compression technique.

## How it works
1. Reads the input file and counts how often each character appears
2. Builds a Huffman tree using a min-heap priority queue, repeatedly merging the two least frequent nodes
3. Generates prefix codes by traversing the tree, so frequent characters get shorter binary codes
4. Writes a header (the frequency table and total bit count), followed by the encoded data packed into bytes
5. To decompress, rebuilds the same tree from the header and decodes the bit stream back to the original file

Ties between equal frequencies are broken deterministically, so the compressor and decompressor always build an identical tree.
Concepts used
Greedy algorithms, binary trees, priority queues, bit manipulation, binary file I/O

