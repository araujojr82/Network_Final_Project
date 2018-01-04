#ifndef _cBuffer_HG
#define _cBuffer_HG

#include <string>

// This Buffer class serializes and deserializes integers, shorts and chars
class cBuffer 
{
public:
    cBuffer(unsigned int size);

    // Returns the Buffer size
    unsigned int bufferSize() { return this->m_buffer.size(); }

    // Set an entire Buffer (receive mode)
    void setBuffer(char* rcvBuffer, int bufferLenght);

    // Returns the Buffer itself
    char* getBuffer();

    // Clears the Buffer
    void clearBuffer();

    // Serializes a Little Endian int and writes it as a Big Endian
    // to the Buffer at the specific index
    void serializeIntLE(unsigned int index, int value);

    // Serializes a Little Endian int and writes it as a Big Endian
    // to the Buffer at the at the current m_writeIndex
    void serializeIntLE(int value);

    // Serializes a Little Endian short and writes it as a Big Endian
    // to the Buffer at the specific index
    void serializeShortLE(unsigned int index, short value);

    // Serializes a Little Endian short and writes it as a Big Endian
    // to the Buffer at the at the current m_writeIndex
    void serializeShortLE(short value);

    // Serializes a char and writes it to the Buffer at the specific index
    void serializeChar(unsigned int index, char value);

    // Serializes a char and writes it to the Buffer at current m_writeIndex
    void serializeChar(char value);


    // Deserializes a Big Endian Int at an specif index in the Buffer
    // and returns it as Little Endian (Assumes Little Endian Machines)
    int deserializeIntBE(unsigned int index);

    // Deserializes a Big Endian Int at an specif index in the Buffer
    // and returns it as Little Endian (Assumes Little Endian Machines)
    //and updates the m_readIndex
    int deserializeIntBE();

    // Deserializes a Big Endian Short at an specif index in the Buffer
    // and returns it as Little Endian (Assumes Little Endian Machines)
    short deserializeShortBE(unsigned int index);

    // Deserializes a Big Endian short at an specif index in the Buffer
    // and returns it as Little Endian (Assumes Little Endian Machines)
    // and updates the m_readIndex
    short deserializeShortBE();

    // Deserializes a char at the given index
    char deserializeChar(unsigned int index);

    // Deserializes a char at the m_readIndex
    char deserializeChar();

private:
    std::string m_buffer;       // The serialized buffer
    char* m_retBuffer;          // Returned buffer
    unsigned int m_readIndex;   // The head of the read index
    unsigned int m_writeIndex;  // The head of the write index
};

#endif