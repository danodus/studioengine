//
//  server.cpp
//  MelobaseCore
//
//  Created by Daniel Cliche on 2014-06-15.
//  Copyright (c) 2014-2021 Daniel Cliche. All rights reserved.
//

#ifdef _WIN32
#include <winsock2.h>
//#include <in6addr.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include <stdio.h>
#include <string.h>

#include <condition_variable>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

#include "platform.h"
#include "server.h"
#include "uricodec.h"
#include "utils.h"

using namespace MelobaseCore;

const int kMinAPI = 5;
const int kMaxAPI = 8;

// ---------------------------------------------------------------------------------------------------------------------
std::vector<std::string> pathComponents(const std::string path) { return stringComponents(path, '/', true); }

// ---------------------------------------------------------------------------------------------------------------------
std::map<std::string, std::string> queryMap(const std::string query) {
    std::map<std::string, std::string> map;

    std::vector<std::string> components = stringComponents(query, '&');

    for (std::string component : components) {
        size_t p = component.find_first_of('=');
        if (p != std::string::npos) {
            std::string key = component.substr(0, p);
            std::string value = MDStudio::uriDecode(component.substr(p + 1));
            map[key] = value;
        }
    }

    return map;
}

// ---------------------------------------------------------------------------------------------------------------------
void folderXML(int api, std::stringstream& ss, std::shared_ptr<SequencesFolder> folder, bool isIDIncluded) {
    if (isIDIncluded) {
        ss << "<folder>";
        ss << "<id>" << std::to_string(folder->id) << "</id>";
    }
    ss << "<parentid>" << std::to_string(folder->parentID) << "</parentid>";
    ss << "<date>" << std::to_string(folder->date) << "</date>";
    ss << "<name>" << encodeXMLString(folder->name) << "</name>";
    ss << "<rating>" << folder->rating << "</rating>";
    ss << "<version>" << std::to_string((unsigned int)(folder->version)) << "</version>";
    if (isIDIncluded) ss << "</folder>";
}

// ---------------------------------------------------------------------------------------------------------------------
void sequenceXML(int api, std::stringstream& ss, std::shared_ptr<Sequence> sequence, bool isIDIncluded,
                 bool areEventsIncluded) {
    if (isIDIncluded) {
        ss << "<sequence>";
        ss << "<id>" << std::to_string(sequence->id) << "</id>";
    }
    if (api >= 6 && sequence->folder != nullptr) {
        ss << "<folderid>" << std::to_string(sequence->folder->id) << "</folderid>";
    }
    ss << "<date>" << std::to_string(sequence->date) << "</date>";
    ss << "<name>" << encodeXMLString(sequence->name) << "</name>";
    ss << "<rating>" << sequence->rating << "</rating>";
    ss << "<version>" << std::to_string((unsigned int)(sequence->version)) << "</version>";
    ss << "<dataVersion>" << std::to_string((unsigned int)(sequence->dataVersion)) << "</dataVersion>";
    ss << "<playcount>" << std::to_string(sequence->playCount) << "</playcount>";
    if (api >= 8) {
        ss << "<annotations>";
        auto v = MelobaseCore::getSequenceAnnotationsBlob(sequence.get());
        std::string s;
        MelobaseCore::base64Encode(s, v);
        ss << s;
        ss << "</annotations>";
    }
    if (areEventsIncluded) {
        ss << "<tickperiod>" << std::to_string(sequence->data.tickPeriod) << "</tickperiod>";

        if (api >= 6) {
            ss << "<data>";

            auto v = MelobaseCore::getSequenceDataBlob(sequence, api > 6);
            std::string s;
            MelobaseCore::base64Encode(s, v);
            ss << s;

            ss << "</data>";
        } else {
            // Deprecated
            ss << "<events>";
            bool isFirst = true;

            // Convert to MDStudio sequence
            auto studioSequence = getStudioSequence(sequence);

            // Convert to single track
            MDStudio::convertSequenceToSingleTrack(studioSequence);

            for (MDStudio::Event event : studioSequence->data.tracks[0].events) {
                if (!isFirst) {
                    ss << "_";
                } else {
                    isFirst = false;
                }
                ss << std::to_string(event.type) << "_";
                ss << std::to_string(event.channel) << "_";
                ss << std::to_string(event.tickCount) << "_";
                ss << std::to_string(event.param1) << "_";
                ss << std::to_string(event.param2);
            }
            ss << "</events>";
        }
    }
    if (isIDIncluded) ss << "</sequence>";
}

// ---------------------------------------------------------------------------------------------------------------------
std::shared_ptr<Sequence> getSequenceFromQuery(std::map<std::string, std::string> queryMap, std::string* content,
                                               bool* areEventsIncluded, bool* isFolderIncluded, int* apiVersion) {
    *areEventsIncluded = false;
    *apiVersion = 0;

    int dataFormat = 0;

    std::shared_ptr<Sequence> sequence = std::shared_ptr<Sequence>(new Sequence());

    if (queryMap.count("api") > 0) *apiVersion = std::atoi(queryMap["api"].c_str());

    if (queryMap.count("date") > 0) sequence->date = std::atof(queryMap["date"].c_str());
    if (queryMap.count("folderid") > 0) {
        *isFolderIncluded = true;
        sequence->folder = std::make_shared<SequencesFolder>();
        sequence->folder->id = std::atol(queryMap["folderid"].c_str());
    }
    if (queryMap.count("name") > 0) sequence->name = queryMap["name"];
    if (queryMap.count("rating") > 0) sequence->rating = std::atof(queryMap["rating"].c_str());
    if (queryMap.count("playcount") > 0) sequence->playCount = std::atoi(queryMap["playcount"].c_str());
    if (queryMap.count("tickperiod") > 0) sequence->data.tickPeriod = std::atof(queryMap["tickperiod"].c_str());
    if (queryMap.count("version") > 0) sequence->version = std::atof(queryMap["version"].c_str());
    if (queryMap.count("dataVersion") > 0) sequence->dataVersion = std::atof(queryMap["dataVersion"].c_str());

    if (queryMap.count("annotations") > 0) {
        auto v = MelobaseCore::base64Decode(queryMap["annotations"].c_str());
        MelobaseCore::setSequenceAnnotationsFromBlob(sequence.get(), v.data(), v.size());
    }

    if (queryMap.count("dataFormat") > 0) dataFormat = std::atoi(queryMap["dataFormat"].c_str());

    if (dataFormat >= 1) {
        if (content->length() > 0) {
            *areEventsIncluded = true;

            // The content contains the data in base 64 format
            auto v = MelobaseCore::base64Decode(content->c_str());
            MelobaseCore::setSequenceDataFromBlob(sequence, &v[0], v.size());
        } else {
            *areEventsIncluded = false;
        }

    } else {
        // Deprecated data format in text values seperated by underscores

        auto studioSequence = std::make_shared<MDStudio::Sequence>();

        std::string* events = nullptr;

        if (queryMap.count("events") > 0) {
            *areEventsIncluded = true;
            events = &queryMap["events"];
        }

        if ((*apiVersion >= 4) && (content->size() > 0)) {
            *areEventsIncluded = true;
            events = content;
        }

        if (events) {
            std::vector<std::string> params = stringComponents(*events, '_');
            std::vector<std::string>::iterator it = params.begin();
            while (it != params.end()) {
                MDStudio::Event event;
                event.type = std::atoi(it->c_str());
                it++;
                event.channel = std::atoi(it->c_str());
                it++;
                event.tickCount = std::atoi(it->c_str());
                it++;
                event.param1 = std::atoi(it->c_str());
                it++;
                event.param2 = std::atoi(it->c_str());
                it++;
                studioSequence->data.tracks[0].events.push_back(event);
            }

            // Convert to Melobase sequence
            auto melobaseSequence = getMelobaseCoreSequence(studioSequence);
            sequence->data.tracks = melobaseSequence->data.tracks;
        }
    }

    return sequence;
}

// ---------------------------------------------------------------------------------------------------------------------
std::shared_ptr<SequencesFolder> getFolderFromQuery(std::map<std::string, std::string> queryMap, std::string* content,
                                                    int* apiVersion) {
    *apiVersion = 0;

    std::shared_ptr<SequencesFolder> folder = std::shared_ptr<SequencesFolder>(new SequencesFolder());

    if (queryMap.count("api") > 0) *apiVersion = std::atoi(queryMap["api"].c_str());

    if (queryMap.count("date") > 0) folder->date = std::atof(queryMap["date"].c_str());

    if (queryMap.count("name") > 0) folder->name = queryMap["name"];

    if (queryMap.count("rating") > 0) folder->rating = std::atof(queryMap["rating"].c_str());

    if (queryMap.count("version") > 0) folder->version = std::atof(queryMap["version"].c_str());

    if (queryMap.count("parentid") > 0) folder->parentID = std::atol(queryMap["parentid"].c_str());

    return folder;
}

// ---------------------------------------------------------------------------------------------------------------------
int handleFoldersRequest(struct mg_connection* conn) {
    const struct mg_request_info* request_info = mg_get_request_info(conn);
    MelobaseCore::Server* server = (MelobaseCore::Server*)request_info->user_data;

    std::vector<std::string> pathComponents = ::pathComponents(request_info->uri);
    std::map<std::string, std::string> queryMap =
        ::queryMap(request_info->query_string ? request_info->query_string : "");

    // Get the request version from the query
    int api = 0;

    if (queryMap.count("api") > 0) {
        if (!queryMap["api"].empty()) api = std::stoi(queryMap["api"]);
    }

    // We clamp the API to the maximum API supported by the server.
    // The client will decide if the response is acceptable for an older API than his.
    if (api >= kMaxAPI) api = kMaxAPI;

    std::string action;
    std::string requestMethod = request_info->request_method;
    if (requestMethod == "GET") {
        if (pathComponents.size() < 4) {
            action = "index.xml";
        } else {
            action = pathComponents[3];
        }

        if (action == "index.xml") {
            std::stringstream ss;
            ss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
            ss << "<folders api=\"" << std::to_string(api) << "\" maxAPI=\"" << kMaxAPI << "\">";

            std::vector<std::shared_ptr<SequencesFolder>> folders = server->sequencesDB()->getFolders(nullptr);
            for (std::shared_ptr<SequencesFolder> folder : folders) {
                folderXML(api, ss, folder, true);
            }

            ss << "</folders>";

            std::string reply = ss.str();

            // Send HTTP reply to the client
            mg_printf(conn,
                      "HTTP/1.1 200 OK\r\n"
                      "Content-Type: text/xml\r\n"
                      "Content-Length: %lu\r\n"  // Always set Content-Length
                      "\r\n"
                      "%s",
                      reply.length(), reply.c_str());

            return 1;
        } else {
            UInt64 id = std::atoll(action.c_str());
            std::shared_ptr<SequencesFolder> folder = server->sequencesDB()->getFolderWithID(id);
            std::stringstream ss;
            if (folder) {
                ss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
                if (api > 0) {
                    ss << "<folder api=\"" << std::to_string(api) << std::string("\">");
                } else {
                    ss << "<folder>";
                }

                folderXML(api, ss, folder, false);

                ss << "</folder>";
            }

            std::string reply = ss.str();

            // Send HTTP reply to the client
            mg_printf(conn,
                      "HTTP/1.1 200 OK\r\n"
                      "Content-Type: text/xml\r\n"
                      "Content-Length: %lu\r\n"  // Always set Content-Length
                      "\r\n"
                      "%s",
                      reply.length(), reply.c_str());

            return 1;
        }
    } else if (requestMethod == "POST") {
        if (pathComponents.size() < 4) {
            //
            // Add a new folder
            //

            int apiVersion = 0;

            std::string content;
            char buf[256];
            int n = 0;
            while ((n = mg_read(conn, buf, sizeof(buf) - 1)) > 0) {
                buf[n] = '\0';
                content += buf;
            }

            std::shared_ptr<SequencesFolder> folder = getFolderFromQuery(queryMap, &content, &apiVersion);

            std::condition_variable cv;
            std::atomic<bool> isFinished(false);
            MDStudio::Platform::sharedInstance()->invoke([&]() {
                if (server->sequencesDB()->undoManager()) server->sequencesDB()->undoManager()->disableRegistration();
                server->sequencesDB()->addFolder(folder, true, false);
                if (server->sequencesDB()->undoManager()) server->sequencesDB()->undoManager()->enableRegistration();
                isFinished = true;
                cv.notify_one();
            });

            // Wait until finished
            std::mutex m;
            std::unique_lock<std::mutex> lk(m);
            cv.wait(lk, [&] { return isFinished == true; });

            std::string reply;

            // Send HTTP reply to the client
            mg_printf(conn,
                      "HTTP/1.1 200 OK\r\n"
                      "Content-Type: text/xml\r\n"
                      "Content-Length: %lu\r\n"  // Always set Content-Length
                      "\r\n"
                      "%s",
                      reply.length(), reply.c_str());

            return 1;
        } else {
            action = pathComponents[3];

            //
            // Update an existing folder
            //

            UInt64 id = std::atoll(action.c_str());
            std::shared_ptr<SequencesFolder> folder = server->sequencesDB()->getFolderWithID(id);
            if (folder) {
                int apiVersion = 0;

                std::string content;
                char buf[256];
                int n = 0;
                while ((n = mg_read(conn, buf, sizeof(buf) - 1)) > 0) {
                    buf[n] = '\0';
                    content += buf;
                }

                std::shared_ptr<SequencesFolder> newFolder = getFolderFromQuery(queryMap, &content, &apiVersion);
                newFolder->id = id;
                std::condition_variable cv;
                std::atomic<bool> isFinished(false);
                MDStudio::Platform::sharedInstance()->invoke([&]() {
                    if (server->sequencesDB()->undoManager())
                        server->sequencesDB()->undoManager()->disableRegistration();
                    // We do not allow sequence data updates from the client unless the API is 3 or above
                    server->sequencesDB()->updateFolder(newFolder, true, false);
                    if (server->sequencesDB()->undoManager())
                        server->sequencesDB()->undoManager()->enableRegistration();
                    isFinished = true;
                    cv.notify_one();
                });

                // Wait until finished
                std::mutex m;
                std::unique_lock<std::mutex> lk(m);
                cv.wait(lk, [&] { return isFinished == true; });
            }

            std::string reply;

            // Send HTTP reply to the client
            mg_printf(conn,
                      "HTTP/1.1 200 OK\r\n"
                      "Content-Type: text/xml\r\n"
                      "Content-Length: %lu\r\n"  // Always set Content-Length
                      "\r\n"
                      "%s",
                      reply.length(), reply.c_str());

            return 1;
        }
    }

    return 0;
}

// ---------------------------------------------------------------------------------------------------------------------
int handleSequencesRequest(struct mg_connection* conn) {
    const struct mg_request_info* request_info = mg_get_request_info(conn);
    MelobaseCore::Server* server = (MelobaseCore::Server*)request_info->user_data;

    std::vector<std::string> pathComponents = ::pathComponents(request_info->uri);
    std::map<std::string, std::string> queryMap =
        ::queryMap(request_info->query_string ? request_info->query_string : "");

    // Get the request version from the query
    int api = 0;

    if (queryMap.count("api") > 0) {
        if (!queryMap["api"].empty()) api = std::stoi(queryMap["api"]);
    }

    // We clamp the API to the maximum API supported by the server.
    // The client will decide if the response is acceptable for an older API than his.
    if (api >= kMaxAPI) api = kMaxAPI;

    std::string action;
    std::string requestMethod = request_info->request_method;
    if (requestMethod == "GET") {
        if (pathComponents.size() < 4) {
            action = "index.xml";
        } else {
            action = pathComponents[3];
        }

        if (action == "index.xml") {
            std::stringstream ss;
            ss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
            if (api > 0) {
                ss << "<sequences api=\"" << std::to_string(api) << "\" maxAPI=\"" << kMaxAPI << "\">";
            } else {
                ss << "<sequences>";
            }

            std::vector<std::shared_ptr<Sequence>> sequences = server->sequencesDB()->getSequences();
            for (std::shared_ptr<Sequence> sequence : sequences) {
                sequenceXML(api, ss, sequence, true, false);
            }

            ss << "</sequences>";

            std::string reply = ss.str();

            // Send HTTP reply to the client
            mg_printf(conn,
                      "HTTP/1.1 200 OK\r\n"
                      "Content-Type: text/xml\r\n"
                      "Content-Length: %lu\r\n"  // Always set Content-Length
                      "\r\n"
                      "%s",
                      reply.length(), reply.c_str());

            return 1;
        } else {
            UInt64 id = std::atoll(action.c_str());
            std::shared_ptr<Sequence> sequence = server->sequencesDB()->getSequenceWithID(id);
            std::stringstream ss;
            if (sequence) {
                ss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
                if (api > 0) {
                    ss << "<sequence api=\"" << std::to_string(api) << std::string("\">");
                } else {
                    ss << "<sequence>";
                }

                bool areEventsIncluded = true;
                if (queryMap.count("areEventsIncluded") > 0)
                    areEventsIncluded = queryMap["areEventsIncluded"] == "true";

                if (areEventsIncluded) server->sequencesDB()->readSequenceData(sequence);

                sequenceXML(api, ss, sequence, false, areEventsIncluded);

                ss << "</sequence>";
            }

            std::string reply = ss.str();

            // Send HTTP reply to the client
            mg_printf(conn,
                      "HTTP/1.1 200 OK\r\n"
                      "Content-Type: text/xml\r\n"
                      "Content-Length: %lu\r\n"  // Always set Content-Length
                      "\r\n"
                      "%s",
                      reply.length(), reply.c_str());

            return 1;
        }
    } else if (requestMethod == "POST") {
        if (pathComponents.size() < 4) {
            //
            // Add a new sequence
            //

            bool areEventsIncluded = false;
            bool isFolderIncluded = false;
            int apiVersion = 0;

            std::string content;
            char buf[256];
            int n = 0;
            while ((n = mg_read(conn, buf, sizeof(buf) - 1)) > 0) {
                buf[n] = '\0';
                content += buf;
            }

            std::shared_ptr<Sequence> sequence =
                getSequenceFromQuery(queryMap, &content, &areEventsIncluded, &isFolderIncluded, &apiVersion);

            // If the folder is not provided, we set it to the defaut sequences folder
            if (!isFolderIncluded) {
                sequence->folder = std::make_shared<SequencesFolder>();
                sequence->folder->id = SEQUENCES_FOLDER_ID;
            }

            std::condition_variable cv;
            std::atomic<bool> isFinished(false);
            MDStudio::Platform::sharedInstance()->invoke([&]() {
                if (server->sequencesDB()->undoManager()) server->sequencesDB()->undoManager()->disableRegistration();
                server->sequencesDB()->addSequence(sequence, true, false);
                if (server->sequencesDB()->undoManager()) server->sequencesDB()->undoManager()->enableRegistration();
                isFinished = true;
                cv.notify_one();
            });

            // Wait until finished
            std::mutex m;
            std::unique_lock<std::mutex> lk(m);
            cv.wait(lk, [&] { return isFinished == true; });

            std::string reply;

            // Send HTTP reply to the client
            mg_printf(conn,
                      "HTTP/1.1 200 OK\r\n"
                      "Content-Type: text/xml\r\n"
                      "Content-Length: %lu\r\n"  // Always set Content-Length
                      "\r\n"
                      "%s",
                      reply.length(), reply.c_str());

            return 1;
        } else {
            action = pathComponents[3];

            //
            // Update an existing sequence
            //

            UInt64 id = std::atoll(action.c_str());
            std::shared_ptr<Sequence> sequence = server->sequencesDB()->getSequenceWithID(id);
            if (sequence) {
                // Read the sequence data in order to have the sequence ID
                server->sequencesDB()->readSequenceData(sequence);

                bool areEventsIncluded = false;
                bool isFolderIncluded = false;
                int apiVersion = 0;

                std::string content;
                char buf[256];
                int n = 0;
                while ((n = mg_read(conn, buf, sizeof(buf) - 1)) > 0) {
                    buf[n] = '\0';
                    content += buf;
                }

                std::shared_ptr<Sequence> newSequence =
                    getSequenceFromQuery(queryMap, &content, &areEventsIncluded, &isFolderIncluded, &apiVersion);
                newSequence->id = id;
                newSequence->data.id = sequence->data.id;
                if (!isFolderIncluded) newSequence->folder = sequence->folder;

                std::condition_variable cv;
                std::atomic<bool> isFinished(false);
                MDStudio::Platform::sharedInstance()->invoke([&]() {
                    if (server->sequencesDB()->undoManager())
                        server->sequencesDB()->undoManager()->disableRegistration();
                    // We do not allow sequence data updates from the client unless the API is 3 or above
                    server->sequencesDB()->updateSequences({newSequence}, true, false,
                                                           (apiVersion >= 3) && areEventsIncluded, false);
                    if (server->sequencesDB()->undoManager())
                        server->sequencesDB()->undoManager()->enableRegistration();
                    isFinished = true;
                    cv.notify_one();
                });

                // Wait until finished
                std::mutex m;
                std::unique_lock<std::mutex> lk(m);
                cv.wait(lk, [&] { return isFinished == true; });
            }

            std::string reply;

            // Send HTTP reply to the client
            mg_printf(conn,
                      "HTTP/1.1 200 OK\r\n"
                      "Content-Type: text/xml\r\n"
                      "Content-Length: %lu\r\n"  // Always set Content-Length
                      "\r\n"
                      "%s",
                      reply.length(), reply.c_str());

            return 1;
        }
    }

    return 0;
}

// ---------------------------------------------------------------------------------------------------------------------
int handleIndexRequest(struct mg_connection* conn) {
    std::stringstream ss;

    ss << "Melobase server is available.\nCompatibility: minAPI=" << kMinAPI << ", maxAPI=" << kMaxAPI;

    std::string reply = ss.str();

    // Send HTTP reply to the client
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: text/plain\r\n"
              "Content-Length: %lu\r\n"  // Always set Content-Length
              "\r\n"
              "%s",
              reply.length(), reply.c_str());

    // Returning non-zero tells mongoose that our function has replied to
    // the client, and mongoose should not send client any more data.

    return 1;
}

// ---------------------------------------------------------------------------------------------------------------------
int handleSaveRequest(struct mg_connection* conn) {
    const struct mg_request_info* request_info = mg_get_request_info(conn);
    MelobaseCore::Server* server = (MelobaseCore::Server*)request_info->user_data;

    server->notifyDidRequestSave();

    std::string reply;

    // Send HTTP reply to the client
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: text/plain\r\n"
              "Content-Length: %lu\r\n"  // Always set Content-Length
              "\r\n"
              "%s",
              reply.length(), reply.c_str());

    // Returning non-zero tells mongoose that our function has replied to
    // the client, and mongoose should not send client any more data.

    return 1;
}

// ---------------------------------------------------------------------------------------------------------------------
// This function will be called by mongoose on every new request.
static int begin_request_handler(struct mg_connection* conn) {
    const struct mg_request_info* request_info = mg_get_request_info(conn);
    MelobaseCore::Server* server = (MelobaseCore::Server*)request_info->user_data;

    server->notifyDidBeginRequestHandling();

    std::vector<std::string> pathComponents = ::pathComponents(request_info->uri);

    int ret = 0;

    if (pathComponents.size() >= 2) {
        if (pathComponents[0] == "/") {
            if (pathComponents[1] == "sequences") {
                ret = handleSequencesRequest(conn);
            } else if (pathComponents[1] == "folders") {
                ret = handleFoldersRequest(conn);
            } else if (pathComponents[1] == "save") {
                ret = handleSaveRequest(conn);
            }
        }
    } else {
        ret = handleIndexRequest(conn);
    }

    server->notifyDidEndRequestHandling();

    return ret;
}

// ---------------------------------------------------------------------------------------------------------------------
MelobaseCore::Server::Server(SequencesDB* sequencesDB) : _sequencesDB(sequencesDB) {
    _svr = nullptr;
    _mgContext = nullptr;
}

// ---------------------------------------------------------------------------------------------------------------------
bool MelobaseCore::Server::start(int port) {
#if defined(_WIN32) || defined(_LINUX)

#ifdef _WIN32
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    // Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h
    wVersionRequested = MAKEWORD(2, 2);

    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        /* Tell the user that we could not find a usable */
        /* Winsock DLL.                                  */
        printf("WSAStartup failed with error: %d\n", err);
        return false;
    }
#endif

    // create host entries
    char hn[1024];
    hn[1023] = '\0';
    gethostname(hn, 1023);

    // Strip the suffix if present in the host name
    for (char* p = hn; *p != '\0'; p++) {
        if (*p == '.') {
            *p = '\0';
            break;
        }
    }

    char hostname[1024 + 6];
    snprintf(hostname, 1024 + 6, "%s.local", hn);

    struct hostent* phe = gethostbyname(hostname);
    if (phe == 0) {
        printf("Bad host lookup\n");
#ifdef _WIN32
        WSACleanup();
#endif
        return false;
    }

    if (phe->h_addr_list[0] == 0) {
        printf("No address found\n");
#ifdef _WIN32
        WSACleanup();
#endif
        return false;
    }

    struct in_addr addr;
    memcpy(&addr, phe->h_addr_list[0], sizeof(struct in_addr));

    _svr = mdnsd_start();

    if (_svr == NULL) {
        printf("mdnsd_start() error\n");
#ifdef _WIN32
        WSACleanup();
#endif
        return false;
    }

    printf("mdnsd_start OK.");
#ifdef _WIN32
    mdnsd_set_hostname(_svr, hostname, addr.S_un.S_addr);
#else
    mdnsd_set_hostname(_svr, hostname, addr.s_addr);
#endif

    const char* txt[] = {"path=/", NULL};

    char serviceName[1024 + 9];
    snprintf(serviceName, 1024 + 9, "%s@Melobase", hn);

    struct mdns_service* svc = mdnsd_register_svc(_svr, serviceName, "_http._tcp.local", port, NULL, txt);
    mdns_service_destroy(svc);

#endif

    //
    // Web server
    //

    char serverPortString[8];
    snprintf(serverPortString, sizeof(serverPortString) - 1, "%d", port);

    const char* options[] = {"listening_ports", serverPortString, "num_threads", "5", "enable_keep_alive", "yes", NULL};

    memset(&_mgCallbacks, 0, sizeof(_mgCallbacks));
    _mgCallbacks.begin_request = begin_request_handler;

    _mgContext = mg_start(&_mgCallbacks, this, options);

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::Server::stop() {
    // Stop the web server
    if (_mgContext) {
        mg_stop(_mgContext);
        _mgContext = nullptr;
    }
#if defined(_WIN32) || defined(_LINUX)
    // Stop MDNS
    if (_svr) {
        mdnsd_stop(_svr);
        _svr = nullptr;
    }
#ifdef _WIN32
    WSACleanup();
#endif

#endif
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::Server::notifyDidBeginRequestHandling() {
    if (_didBeginRequestHandlingFn) {
        MDStudio::Platform::sharedInstance()->invoke([=] { _didBeginRequestHandlingFn(this); });
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::Server::notifyDidEndRequestHandling() {
    if (_didEndRequestHandlingFn) {
        MDStudio::Platform::sharedInstance()->invoke([=] { _didEndRequestHandlingFn(this); });
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::Server::notifyDidRequestSave() {
    if (_didRequestSaveFn) {
        MDStudio::Platform::sharedInstance()->invoke([=] { _didRequestSaveFn(this); });
    }
}
