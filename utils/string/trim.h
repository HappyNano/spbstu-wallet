#include <string>

namespace cxx {

    /**
     * @brief Trims whitespace characters from the left side (beginning) of a string.
     *
     * This function modifies the input string in-place, removing all leading whitespace
     * characters including spaces, tabs, newlines, carriage returns, and other whitespace
     * as defined by std::isspace().
     *
     * @param s Reference to the string to be modified.
     */
    void ltrim(std::string & s);

    /**
     * @brief Trims whitespace characters from the right side (end) of a string.
     *
     * This function modifies the input string in-place, removing all trailing whitespace
     * characters including spaces, tabs, newlines, carriage returns, and other whitespace
     * as defined by std::isspace().
     *
     * @param s Reference to the string to be modified.
     */
    void rtrim(std::string & s);

    /**
     * @brief Creates a new string with whitespace characters trimmed from the left side.
     *
     * This function creates and returns a new string with all leading whitespace
     * characters removed while leaving the original string unchanged.
     *
     * @param s The input string to process.
     * @return std::string A new string with leading whitespace removed.
     */
    std::string ltrimCopy(std::string s);

    /**
     * @brief Creates a new string with whitespace characters trimmed from the right side.
     *
     * This function creates and returns a new string with all trailing whitespace
     * characters removed while leaving the original string unchanged.
     *
     * @param s The input string to process.
     * @return std::string A new string with trailing whitespace removed.
     */
    std::string rtrimCopy(std::string s);

    /**
     * @brief Trims whitespace characters from both sides of a string.
     *
     * This function modifies the input string in-place, removing all leading and
     * trailing whitespace characters.
     *
     * @param s Reference to the string to be modified.
     */
    void trim(std::string & s);

    /**
     * @brief Creates a new string with whitespace characters trimmed from both sides.
     *
     * This function creates and returns a new string with all leading and trailing
     * whitespace characters removed while leaving the original string unchanged.
     *
     * @param s The input string to process.
     * @return std::string A new string with both leading and trailing whitespace removed.
     */
    std::string trimCopy(std::string s);

} // namespace cxx
