#include <iostream>

#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <sys/types.h>
#include <unistd.h>

#include <stdlib.h>
#include <math.h> 

using namespace std;

class MMap
{
public:

    enum VIEW { SHARED, PRIVATE };

    MMap(const char* path, const int len) :
        fd(0),
        filename(path),
        length(len),
        sview(0),
        pview(0)
    {
    }

    ~MMap()
    {
    }

    void mapShared()
    {
        if (sview != 0)
        {
            cerr << "-- shared view already exists" << endl;
            return;
        }
        cout << "- about to map shared view...";
        cin.get();
        fd = open(filename, O_RDWR | O_NOATIME);
        if ( fd <= 0 )
        {
            cerr << "couldn't open " << filename << endl;
            fd = 0; // our sentinel for not opened
            abort();
        }

        unsigned long long filelen = lseek(fd, 0, SEEK_END);
        if (filelen != length)
        {
            cerr << "map file alloc failed, wanted: " << length << " filelen: " << filelen << ' ' << sizeof(size_t) << endl;
            abort();
        }
        lseek( fd, 0, SEEK_SET );

        void *view = mmap(NULL, filelen, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if ( view == MAP_FAILED )
        {
            cerr << "  mmap() failed for " << filename << " len:" << filelen << endl;
            if ( errno == ENOMEM )
            {
                cerr << "mmap failed with out of memory. (64 bit build)" << endl;
            }
            abort();
        }
        sview = (unsigned char *)view;
        cout << "- MAPPED!" << endl;
    }

    void mapPrivate()
    {
        if (pview != 0)
        {
            cerr << "-- private view already exists" << endl;
            return;
        }
        cout << "- about to map private view...";
        cin.get();
        void *view = mmap( /*start*/0 , length , PROT_READ | PROT_WRITE , MAP_PRIVATE | MAP_NORESERVE , fd , 0 );
        if ( view == MAP_FAILED )
        {
            if ( errno == ENOMEM )
            {
                cerr << "mmap private failed with out of memory. (64 bit build)" << endl;
            }
            else
            {
                cerr << "mmap private failed " << endl;
            }
            abort();
        }
        pview = (unsigned char *)view;
        cout << "- MAPPED!" << endl;
    }

    void remapPrivate()
    {
        if (pview == 0)
        {
            cerr << "-- private view does not yet exist" << endl;
            return;
        }
        cout << "- about to remap private view...";
        cin.get();
        void *view = mmap( pview, length , PROT_READ | PROT_WRITE , MAP_PRIVATE | MAP_NORESERVE | MAP_FIXED , fd , 0 );
        if ( view == MAP_FAILED )
        {
            int err = errno;
            cerr  << "13601 Couldn't remap private view: " << err << endl;
            abort();
        }
        if ( view != pview )
        {
            cerr  << "Error remapping private view" << endl;
            abort();
        }
        pview = (unsigned char *)view;
        cout << "- REmapped!" << endl;
    }

    void copyPrivateToShared()
    {
        if (sview == 0 || pview == 0)
        {
            cerr << "-- need to create both views" << endl;
            return;
        }
        cout << "- about to copy private view to shared view...";
        cin.get();
        for (int i = 0; i < length; i++)
        {
            *(sview + i) = *(pview + i);
        }
        cout << "- Done!" << endl;
    }

    void read(VIEW view)
    {
        const unsigned char *const base = (view == SHARED ? sview : pview);
        const char *name = (view == SHARED ? "shared" : "private");
        if (base == 0)
        {
            cerr << "-- " << name << " view doesn't exist" << endl;
            return;
        }
        cout << "- about to read " << name << " view...";
        cin.get();
        unsigned long long total = 0;
        for (int i = 0; i < length; i++)
        {
            total++;
            unsigned long _t = *(base + i);
        }
        cout << "- Done! total: " << total << endl;
    }

    void readWrite(VIEW view, int percentage)
    {
        unsigned char *const base = (view == SHARED ? sview : pview);
        const char *name = (view == SHARED ? "shared" : "private");
        if (base == 0)
        {
            cerr << "-- " << name << " view doesn't exist" << endl;
            return;
        }
        cout << "- about to read/write " << name << " view..." << endl;
        cin.get();
        unsigned long long total_read = 0;
        unsigned long long total_write = 0;
        float read_frequency = (float) percentage / 100;
        float write_frequency = (float) (100 - percentage) / 100;
        read_frequency = read_frequency * length;
        write_frequency = write_frequency * length;
        if(read_frequency > 0) {
          read_frequency = floor(length / read_frequency);
        }
        if(write_frequency > 0) {
          write_frequency = floor(length / write_frequency);
        }
        cout << "read/write granularity: " << read_frequency << "/" << write_frequency << endl;
        for (int i = 0; i < length; i++)
        {
            if(read_frequency != 0 && (read_frequency == 1 || i%((int) read_frequency) == 0)) {
              total_read++;
              unsigned long _t = *(base + i);
            }
            if(write_frequency != 0 && (write_frequency == 1 || i%((int) write_frequency) == 0)) {
              total_write++;
              ++(*(base + i));
            }
        }
        cout << "- Done! read: " << total_read << " / write: " << total_write << endl;
    }

    void write(VIEW view)
    {
        unsigned char *const base = (view == SHARED ? sview : pview);
        const char *name = (view == SHARED ? "shared" : "private");
        if (base == 0)
        {
            cerr << "-- " << name << " view doesn't exist" << endl;
            return;
        }
        cout << "- about to write " << name << " view...";
        cin.get();
        for (int i = 0; i < length; i++)
        {
            ++(*(base + i));
        }
        cout << "- Done!" << endl;
    }

    int getFileLength() {
        return length;
    }

private:

    int fd;

    const int length;
    const char *filename;

    unsigned char *sview;
    unsigned char *pview;
};

long getFileSize(std::string filepath) {
    struct stat stat_buf;
    int rc = stat(filepath.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

int main()
{
    unsigned int percentage;
    MMap mmap("/tmp/1GB", getFileSize("/tmp/1GB"));
    cout << "File size: " << mmap.getFileLength() << endl;

    string input = "?";
    while (true)
    {
        switch (input.c_str()[0])
        {
        case '?':
            cout <<
                 "1: map shared" << endl <<
                 "2: map private" << endl <<
                 "3: remap private" << endl <<
                 "4: copy private to shared" << endl <<
                 "5: read shared" << endl <<
                 "6: write shared" << endl <<
                 "7: read private" << endl <<
                 "8: write private" << endl <<
                 "9: read/write shared" << endl <<
                 "a: read/write private" << endl <<
                 "?: <this help>" << endl;
            break;
        case '1': mmap.mapShared(); break;
        case '2': mmap.mapPrivate(); break;
        case '3': mmap.remapPrivate(); break;
        case '4': mmap.copyPrivateToShared(); break;
        case '5': mmap.read(MMap::SHARED); break;
        case '6': mmap.write(MMap::SHARED); break;
        case '7': mmap.read(MMap::PRIVATE); break;
        case '8': mmap.write(MMap::PRIVATE); break;
        case '9':
            cout << "Percentage of reads: ";
            cin >> percentage;
            if(percentage > 100 || percentage < 0) {
              cout << "Invalid percentage!";
            } else {
              mmap.readWrite(MMap::SHARED, percentage);
            }
            break;
        case 'a':
            cout << "Percentage of reads: ";
            cin >> percentage;
            if(percentage > 100 || percentage < 0) {
              cout << "Invalid percentage!";
            } else {
              mmap.readWrite(MMap::PRIVATE, percentage);
            }
            break;
        default: exit(0);
        }
        cout << "choice? ";
        getline(cin, input);
    }

    return 0;
}
