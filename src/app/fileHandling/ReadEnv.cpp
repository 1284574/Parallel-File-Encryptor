#include <iostream>    // For console I/O (not used here but often useful for diagnostics)
#include <string>      // For std::string
#include <fstream>     // For std::fstream
#include <IO.hpp>      // Our RAII wrapper around std::fstream
#include <sstream>     // For std::stringstream

/**
 * @class ReadEnv
 * @brief Reads the contents of a “.env” file and returns it as a string.
 *
 * This small helper class encapsulates the logic to:
 *  1. Open the “.env” file via our IO wrapper
 *  2. Slurp its entire contents into an std::stringstream
 *  3. Extract and return the resulting std::string
 */
class ReadEnv
{
public:
    /**
     * @brief Loads the entire contents of the .env file.
     *
     * @return A std::string containing the raw text of the “.env” file.
     *
     * Steps performed:
     * 1.  Define the file path: here, the literal ".env" in the current working directory.
     * 2.  Construct an IO object to open the file; its constructor should open the stream.
     * 3.  Call IO::getFileStream() to obtain a std::fstream by moving out the internal stream.
     * 4.  Declare a std::stringstream buffer to accumulate the file data.
     * 5.  Use buffer << f_stream.rdbuf() to read all characters from the fstream into the buffer.
     * 6.  Extract the complete contents via buffer.str() into a std::string.
     * 7.  Return the content string.
     *
     * Note:
     * - If opening the file fails, the IO constructor or the fstream may be in a fail state.
     *   In a production version, you’d check `if (!f_stream)` and handle errors.
     * - After moving the stream out of the IO object, the internal stream inside `io`
     *   is left in a valid but unspecified state; it will be closed in IO’s destructor.
     */
    std::string getenv()
    {
        // 1. Path to the environment file
        std::string env_path = ".env";

        // 2. Open the file via our RAII wrapper
        IO io(env_path);

        // 3. Obtain the underlying std::fstream by move semantics
        std::fstream f_stream = io.getFileStream();

        // 4. Create a buffer to read the entire file content
        std::stringstream buffer;

        // 5. Read all data from the file into the stringstream
        buffer << f_stream.rdbuf();

        // 6. Extract the accumulated data as a std::string
        std::string content = buffer.str();

        // 7. Return the file contents to the caller
        return content;
    }
};
