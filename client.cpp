#include "conn.hpp"

static void repl(int fd);

int main(int argc, char *argv[]) {
    const char        *host = "127.0.0.1";
    uint16_t           port = 8000;
    struct sockaddr_in addr;
    int                fd;

    if(argc != 1 && argc != 3) {
        fprintf(stderr, "Usage: %s [<ip> <port>]\n", argv[0]);
        return -1;
    }

    if(argc == 3) {
        host = argv[1];
        if(toUInt16(argv[2], &port) == -1) {
            fprintf(stderr, "Wrong parametr <port>\n");
            return -1;
        }
    }

    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    if(inet_pton(AF_INET, host, &addr.sin_addr) < 1) {
        fprintf(stderr, "     - Host error.\n");
        return -1;
    }

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd == -1) {
        fprintf(stderr, "     - Socket error.\n");
        return -1;
    }

    if(connect(fd, (struct sockaddr *)&addr, sizeof addr) == -1) {
        fprintf(stderr, "%4d - Connect error.\n", fd);
        if(close(fd))
            fprintf(stderr, "%4d - Close error.\n", fd);
        return -1;
    }

    repl(fd);

    if(shutdown(fd, 2) == -1) {
        fprintf(stderr, "%4d - Shutdown error.\n", fd);
        if(close(fd))
            fprintf(stderr, "%4d - Close error.\n", fd);
        return -1;
    }

    if(close(fd)) {
        fprintf(stderr, "%4d - Close error.\n", fd);
        return -1;
    }

    return 0;
}

#define BUFLEN 50

static bool readLine(std::vector<char> &l) {
    char   buf[BUFLEN];    
    size_t len;

    l.clear();    

    for(;;) {
        if(!fgets(buf, sizeof buf, stdin))
            return false;

        len = strlen(buf);
       
        l.insert(l.end(), buf, buf + len); 

        if(buf[len - 1] == '\n') {
            l.push_back('\0');
            break;
        } 
    }

    return true;
}

static bool writeRead(int fd, const char *txt);
static bool readLine(std::vector<char> &l);

static void repl(int fd) {
    for(;;) {
        printf("> ");
   
        std::vector<char> l;
        if(!readLine(l))
            return;

        if(!writeRead(fd, &l[0]))
            return;
    }
}

#define LINESIZE 70

static bool writeAll(int fd, const char *buf, size_t len);
static bool readAll(int fd, char *buf, size_t len);

static bool writeRead(int fd, const char *txt) {
    try {
        uint32_t len = strlen(txt) + 1;

        if(!writeAll(fd, (const char *)&len, sizeof len))
            throw "     - Write(length) error.";

        if(!writeAll(fd, txt, len))
            throw "     - Write(text) error.";

        if(!readAll(fd, (char *)&len, sizeof len))
            throw "     - Read(length) error.";

        std::vector<char> buf(len, 0);

        if(!readAll(fd, &buf[0], len))
            throw "     - Read(text) error.";

        len = (uint32_t)strlen(&buf[0]);

        printf("[%u] ", len);

        if(len > LINESIZE) {
            buf[LINESIZE] = '\0';
            printf("%s ...\n\n", &buf[0]);
        }
        else
            puts(&buf[0]);

        return true;
    }
    catch(const char *err) {
        fprintf(stderr, "%s\n", err);
    }
    catch(std::bad_alloc) {
        fprintf(stderr, "Memory allocation error\n");
    }

    return false;
}

static bool writeAll(int fd, const char *buf, size_t len) {
    for(size_t cur = 0; len - cur;) {
        ssize_t s = write(fd, buf + cur, len - cur);
        if(s > 0) {
            fprintf(stderr, "     - Write %d bytes\n", (int)s);
            cur += s;
        }
        else
            return false;
    }
    return true;
}

static bool readAll(int fd, char *buf, size_t len) {
    for(size_t cur = 0; len - cur;) {
        ssize_t s = read(fd, buf + cur, len - cur);
        if(s > 0) {
            fprintf(stderr, "     - Read %d bytes\n", (int)s);
            cur += s;
        } 
        else
            return false;
    }
    return true;
}

