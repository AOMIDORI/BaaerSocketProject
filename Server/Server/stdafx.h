#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include <string>
#include <map>
#include <vector>
#include <list>

#include <chrono>
#include <ctime>
#include <thread>
#include <mutex>

#include <fstream>
#include <iostream>

#include "Message.h"

using namespace std;

#define DEFAULT_BUFLEN 512
#define DEFAULT_BUNDLE 10

