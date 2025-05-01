#ifndef LOGS_BUFFER_CLASS_TEMPLATE
#define LOGS_BUFFER_CLASS_TEMPLATE

#include <span>
#include <iostream>
#include <sstream>
#include <ctime>
#include <vector>
#include <string>

using namespace std;

class terminal_output_catcher : public streambuf {
    streambuf* original_stream;
    ostringstream buffer;
    ostream& stream;
    const bool is_error;
    vector<string> messages;
    vector<string> timestamps;
    vector<uint8_t> is_error_vec;

    string get_date() {
        time_t now = time(0);
        char buffer_char[80];
        strftime(buffer_char, sizeof(buffer_char), "%Y-%m-%d %H:%M:%S", localtime(&now));
        return string(buffer_char);
    }

public:
        terminal_output_catcher(ostream& stream, bool error_label) 
        : stream(stream), is_error(error_label) {
        original_stream = stream.rdbuf(this);  // Redirects cout o cerr
    }

    ~terminal_output_catcher() {
        stream.rdbuf(original_stream);  // Restores output to original
    }

    int overflow(int c) override {
        if (c != EOF) {
            char ch = static_cast<char>(c);
            buffer.put(ch);               // Stores message in buffer
            original_stream->sputc(ch);  // Prints message in console
            if (ch == '\n') {
                messages.push_back(buffer.str()); // Stores message in message array
                timestamps.push_back(get_date());
                is_error_vec.push_back(is_error);
                buffer.str("");  
            }
        }
        return c;
    }

    span<const string> get_messages() const {
        return std::span<const string>(messages);
    }

    span<const string> get_timestamps() const {
        return std::span<const string>(timestamps);
    }

    span<const uint8_t> get_error() const {
        return span<const uint8_t>(is_error_vec);
    }

    void clear_log() {
        messages.clear();
        timestamps.clear();
        is_error_vec.clear();
    }
};

#endif