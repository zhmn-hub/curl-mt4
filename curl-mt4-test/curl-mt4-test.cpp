// curl-mt4.cpp : Defines the entry point for the console application.
//

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <curl-mt4.h>
#include <string>
#include <vector>
#include <iostream>

std::string StrError(void* curl, int code) {
    char buf[256];
    int n = CurlLastError(curl, code, buf, sizeof(buf));
    return std::string(buf, n);
}

int main(int argc, char* argv[])
{
    auto curl = CurlInit();

    if (!curl) {
        std::cerr << "Error initializing curl (CurlInit)!" << std::endl;
        return 1;
    }

    std::string url, post_data;
    CurlMethod method = CurlMethod::GET;
    int opts = 0;

    for (auto i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            std::cerr << "Usage: " << argv[0]
                      << " [-h|--help] [-X POST] [-H Header] [-d JsonPostData]"
                         " [-v [N]] [-t TimeoutSecs] URL"
                      << std::endl;
            return 1;
        } else if (strcmp(argv[i], "-H") == 0 && i < argc-1)
            CurlAddHeaders(curl, argv[++i]);
        else if (strcmp(argv[i], "-X") == 0 && i < argc - 1) {
            if (strcmp(argv[++i], "POST") == 0)
                method = CurlMethod::POST_JSON;
            else {
                std::cerr << "Invalid -X argument (method): " << argv[i] << std::endl;
                return 2;
            }
        } else if (strcmp(argv[i], "-d") == 0 && i < argc-1)
            post_data = argv[++i];
        else if (strcmp(argv[i], "-v") == 0) {
            opts |= OPT_DEBUG;
            int n;
            if (i < argc-1 && argv[i+1][0] != '-') {
                try   { n = std::stoi(argv[++i]); CurlDbgLevel(curl, n); }
                catch (...)
                {
                    std::cerr << "Invalid -v argument (verbose): " << argv[i] << std::endl;
                    return 2;
                }
            }
        } else if (strcmp(argv[i], "-t") == 0 && i < argc - 1) {
            auto timeout = std::stoi(argv[++i]);
            auto res = CurlSetTimeout(curl, timeout);
            if (res != 0)
                std::cerr << "Error setting timeout: " << res << std::endl;
        } else if (argv[i][0] == '-') {
            std::cerr << "Invalid argument: " << argv[i] << " (i=" << i << ", argc=" << argc << ")" << std::endl;
            return 1;
        } else
            url = argv[i];
    }

    int res;

    if ((res = CurlSetURL(curl, url.c_str())) != 0) {
        std::cerr << "Error in CurlSetURL: " << StrError(curl, res) << std::endl;
        return 2;
    }

    int code, length;

    if ((res = CurlExecute(curl, &code, &length, method, opts, post_data.empty() ? nullptr : post_data.c_str())) != 0) {
        std::cerr << "Error in CurlExecute: (" << res << ") " << StrError(curl, res) << std::endl;
        return 2;
    }

    if (OPT_DEBUG & opts) {
        std::string v;
        v.resize(CurlDbgInfoSize(curl));
        CurlDbgInfo(curl, &v[0], v.size());
        std::cerr << "===== Debug Info =====" << std::endl;
        std::cerr.write(&v[0], v.size());
        std::cerr.flush();
        std::cerr << "\n======================" << std::endl;
    }

    std::vector<char> s;
    if (length > 0) {
        s.resize(length);
        CurlGetData(curl, &s[0], length);

        std::cout.write(&s[0], length);
        std::cout << std::endl;
        std::cout.flush();
    }
    else
        std::cout << "Server response: " << code << std::endl;

    return 0;
}
