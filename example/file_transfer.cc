#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "log/logger.h"
#include "net/tcp_listener.h"
#include "runtime/event_loop.h"

using namespace baize;
using namespace baize::net;
using namespace baize::runtime;
using namespace baize::time;

const char* img =
    "   _            __  _   \n"
    "  | |          / _|| |  \n"
    "  | |__   ____| |_ | |_ \n"
    "  | '_ \\ |_  /|  _|| __|\n"
    "  | |_) | / / | |  | |_ \n"
    "  |_.__/ /___||_|   \\__|\n"
    "   baize file transfer  \n"
    "                        \n"
    " usage:                 \n"
    "     bzft put [filename]\n"
    "     bzft get [ip]      \n";

class BigFileReader
{
public:
    BigFileReader(string filename)
    {
        fd_ = ::open(filename.c_str(), O_RDONLY);
        if (fd_ < 0) {
            LOG_FATAL << "file reader open failed";
        }

        struct stat file_stat;
        int err = ::stat(filename.c_str(), &file_stat);
        if (err < 0) {
            LOG_FATAL << "file stat err";
        }

        if (S_ISDIR(file_stat.st_mode)) {
            LOG_FATAL << "no support dir transfer";
        }

        file_size_ = static_cast<uint64_t>(file_stat.st_size);
    }

    ~BigFileReader()
    {
        if (fd_ >= 0) {
            ::close(fd_);
        }
    }

    uint64_t file_size() { return file_size_; }

    int ReadFile(Buffer& buf) { return buf.ReadFd(fd_); }

private:
    int fd_ = -1;
    uint64_t file_size_ = 0;
};

class FileTransferServer
{
public:
    FileTransferServer(uint16_t port, string filename, string filemd5)
      : listener_(port),
        filename_(filename),
        filemd5_(filemd5),
        file_reader_(filename),
        timer_([this] { return OnTimer(); })
    {
    }

    void Start()
    {
        TcpStreamSptr stream = listener_.AsyncAccept();
        if (!stream) {
            LOG_ERROR << "server accept failed";
            return;
        }

        Buffer buf(65536);
        buf.Append("bzfthead");
        buf.Append(static_cast<uint8_t>(filename_.length()));
        buf.Append(filename_);
        buf.Append(static_cast<uint8_t>(filemd5_.length()));
        buf.Append(filemd5_);
        buf.Append("bzftbody");
        buf.Append(HostToNetwork64(file_reader_.file_size()));

        last_time = Timestamp::Now();
        timer_.Start(1000);

        while (1) {
            int rn = file_reader_.ReadFile(buf);
            if (rn == 0) {
                timer_.Stop();
                break;
            } else if (rn < 0) {
                LOG_FATAL << "read file err";
            }

            int wn = stream->AsyncWrite(buf.read_index(), buf.readable_bytes());
            if (wn != buf.readable_bytes()) {
                LOG_FATAL << "stream write err";
            }

            total_send_bytes += wn;

            buf.TakeAll();
        }

        PrintStat();

        stream->ShutdownWrite();
        assert(buf.readable_bytes() == 0);

        bool timeout = false;
        int rn = stream->AsyncRead(buf, 3000, timeout);
        if (timeout || rn < 0 || rn > 0) {
            LOG_FATAL << "wait peer close timeout, file transfer may err";
        }
        if (rn == 0) {
            LOG_INFO << "file transfer succeed!";
        }
    }

    int OnTimer()
    {
        PrintStat();
        return kTimer1S;
    }

    void PrintStat()
    {
        Timestamp now = Timestamp::Now();
        int64_t passed_ms = now.ms() - last_time.ms();
        last_time = now;

        uint64_t send_bytes = total_send_bytes - last_send_bytes;
        last_send_bytes = total_send_bytes;

        double passed_second = static_cast<double>(passed_ms) / 1000;
        double bw =
            static_cast<double>(send_bytes) / passed_second / 1024 / 1024;

        double file_size =
            static_cast<double>(file_reader_.file_size()) / 1024 / 1024;
        double send_size = static_cast<double>(total_send_bytes) / 1024 / 1024;
        double percent = send_size / file_size * 100;

        string s;
        log::StringAppend(
            s,
            "send bandwidth %.2lf MiB/s, file total size %.2lf MiB, has "
            "sent %.2lf MiB(%.2lf%%)",
            bw,
            file_size,
            send_size,
            percent);

        LOG_INFO << s;
    }

private:
    TcpListener listener_;
    string filename_;
    string filemd5_;
    BigFileReader file_reader_;
    Timer timer_;

    uint64_t total_send_bytes = 0;
    uint64_t last_send_bytes = 0;
    Timestamp last_time;
};

class FileWriter
{
public:
    FileWriter() {}

    ~FileWriter()
    {
        if (fd_ >= 0) {
            close(fd_);
        }
    }

    void Init(string filename)
    {
        fd_ = ::open(filename.c_str(), O_WRONLY | O_CREAT);
        if (fd_ < 0) {
            LOG_SYSFATAL << "file writer init err";
        }
    }

    void Write(StringPiece data)
    {
        ssize_t rn = write(fd_, data.data(), data.size());
        if (static_cast<int>(rn) != data.size()) {
            LOG_FATAL << "file writer write err";
        }
    }

    void Flush()
    {
        int err = fsync(fd_);
        if (err < 0) {
            LOG_FATAL << "sync err";
        }
    }

private:
    int fd_ = -1;
};

class FileTransferClient
{
public:
    FileTransferClient() : timer_([this] { return OnTimer(); }) {}

    void Start(string ip)
    {
        TcpStreamSptr stream = TcpStream::AsyncConnect(ip.c_str(), 6543);
        if (!stream) {
            LOG_FATAL << "connect to " << ip << " error";
        }

        last_time = Timestamp::Now();
        timer_.Start(1000);

        bool start_write_file = false;
        Buffer buf(65536);
        while (1) {
            int rn = stream->AsyncRead(buf);
            if (rn == 0) {
                PrintStat();
                // file_writer_.Flush();
                LOG_INFO << "recv file succeed!!";
                break;
            } else if (rn < 0) {
                LOG_FATAL << "stream read err";
            }

            total_read_bytes += rn;

            if (!start_write_file) {
                StringPiece msg = buf.slice();
                const char* index = msg.Find("bzftbody");
                if (index == msg.end()) continue;
                if (msg.size() < index - msg.begin() + 8 + 8) continue;

                const char* pos = msg.begin();
                int left_len = msg.size();
                if (string(pos, 8) != "bzfthead") {
                    LOG_FATAL << "client parse err";
                }
                pos += 8;
                left_len -= 8;

                uint8_t filename_len = *reinterpret_cast<const uint8_t*>(pos);
                if (left_len < filename_len + 1) {
                    LOG_FATAL << "client parse err";
                }
                pos += 1;
                left_len -= 1;
                filename_ = string(pos, filename_len);
                pos += filename_len;
                left_len -= filename_len;

                uint8_t filemd5_len = *reinterpret_cast<const uint8_t*>(pos);
                if (left_len < filemd5_len + 1) {
                    LOG_FATAL << "client parse err";
                }
                pos += 1;
                left_len -= 1;
                filemd5_ = string(pos, filemd5_len);
                pos += filemd5_len;
                left_len -= filemd5_len;

                if (left_len < 8 + 8) {
                    LOG_FATAL << "client parse err";
                }

                if (string(pos, 8) != "bzftbody") {
                    LOG_FATAL << "client parse err";
                }
                pos += 8;
                left_len -= 8;

                uint64_t file_size = *reinterpret_cast<const uint64_t*>(pos);
                file_size_ = NetworkToHost64(file_size);
                pos += 8;
                left_len -= 8;

                buf.Take(msg.size() - left_len);
                start_write_file = true;

                LOG_INFO << "filename: " << filename_
                         << ", filemd5: " << filemd5_
                         << ", filesize: " << (file_size_ / 1024 / 1024)
                         << " MiB";

                FileWriter pathfile;
                pathfile.Init(".bzft.pathfile");
                pathfile.Write(filename_);

                FileWriter md5file;
                md5file.Init(".bzft.md5");
                md5file.Write(filemd5_);

                file_writer_.Init(filename_);
            } else {
                file_writer_.Write(buf.slice());
                buf.TakeAll();
            }
        }
    }

    int OnTimer()
    {
        PrintStat();
        return kTimer1S;
    }

    void PrintStat()
    {
        Timestamp now = Timestamp::Now();
        int64_t passed_ms = now.ms() - last_time.ms();
        last_time = now;

        uint64_t read_bytes = total_read_bytes - last_read_bytes;
        last_read_bytes = total_read_bytes;

        double passed_second = static_cast<double>(passed_ms) / 1000;
        double bw =
            static_cast<double>(read_bytes) / passed_second / 1024 / 1024;

        double file_size = static_cast<double>(file_size_) / 1024 / 1024;
        double read_size = static_cast<double>(total_read_bytes) / 1024 / 1024;
        double percent = read_size / file_size * 100;

        string s;
        log::StringAppend(
            s,
            "read bandwidth %.2lf MiB/s, file total size %.2lf MiB, has "
            "read %.2lf MiB(%.2lf%%)",
            bw,
            file_size,
            read_size,
            percent);

        LOG_INFO << s;
    }

private:
    FileWriter file_writer_;
    Timer timer_;
    string filename_;
    string filemd5_;
    uint64_t file_size_ = 0;
    uint64_t total_read_bytes = 0;
    uint64_t last_read_bytes = 0;
    Timestamp last_time;
};

void print_command_usage()
{
    fprintf(stderr, "usage:\n");
    fprintf(stderr, "  bzft put [filename]\n");
    fprintf(stderr, "  bzft get [ip]\n");
    exit(0);
}

int main(int argc, char* argv[])
{
    fprintf(stderr, "%s", img);
    if (argc < 3) {
        print_command_usage();
    }

    baize::runtime::EventLoop loop;
    string role = argv[1];

    if (role == "put") {
        if (argc != 4) {
            print_command_usage();
        }
        loop.Do(
            [=] {
                FileTransferServer server(6543, argv[2], argv[3]);
                server.Start();
                exit(0);
            },
            "bzft-server");
    } else if (role == "get") {
        if (argc != 3) {
            print_command_usage();
        }
        loop.Do(
            [=] {
                FileTransferClient client;
                client.Start(argv[2]);
                exit(0);
            },
            "bzft-client");
    } else {
        print_command_usage();
    }

    loop.Start();
}