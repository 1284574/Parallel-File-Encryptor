#ifndef IO_HPP
#define IO_HPP

// Include the standard file-stream library so we can read/write files
#include <fstream>
// Include string support for handling file paths
#include <string>
// Include iostream in case the implementation needs console I/O (e.g., error messages)
#include <iostream>

/**
 * @class IO
 * @brief A simple RAII-style wrapper around std::fstream for file I/O.
 *
 * This class opens a file in its constructor and closes it in its destructor,
 * ensuring that the file resource is properly released when the object goes out of scope.
 */
class IO
{
public:
    /**
     * @brief Constructs an IO object and opens the given file.
     * @param file_path The path to the file to open.
     *
     * The constructor should open file_stream (e.g., file_stream.open(file_path, std::ios::in | std::ios::out))
     * or throw an exception if the file cannot be opened.
     */
    IO(const std::string &file_path);

    /**
     * @brief Destroys the IO object and closes the file.
     *
     * The destructor automatically closes file_stream if it is open,
     * preventing resource leaks.
     */
    ~IO();

    /**
     * @brief Provides access to the underlying std::fstream.
     * @return A copy (or reference) to the managed file stream.
     *
     * This getter allows callers to perform read/write operations directly
     * on the file. It could return by reference to avoid copying the stream.
     */
    std::fstream getFileStream();

private:
    /**
     * @brief The file stream managed by this IO class.
     *
     * This member holds the std::fstream object that is opened in the constructor
     * and closed in the destructor. You can use this stream for reading/writing.
     */
    std::fstream file_stream;
};

#endif // IO_HPP
