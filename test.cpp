//Lab 7, 8 and full project cse 461 by michael ingersoll
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <sstream>
#include <cmath>

using namespace std;

class Sdisk
{
    public :
    Sdisk() { }
    
    Sdisk(string diskname);                                         
    Sdisk(string diskname, int numberofblocks, int blocksize);
    int getblock(int blocknumber, string& buffer);
    int putblock(int blocknumber, string buffer);
    int getblocksize() {return blocksize; }                        
    int getnumberofblocks() { return numberofblocks; }              
    string getfilename() { return diskname; }                      
    friend class Shell;
    friend class Filesys;
    friend class Table;
    
    private :
    
    int numberofblocks;                                           
    string diskname;                                              
    string diskname1;
    int blocksize;                                              
};
class Filesys: Sdisk
{
public:
    
    Filesys(string diskname, int numberofblocks, int blocksize);
    void start(Sdisk);
    int fsclose();
    int newfile(string file);
    int rmfile(string file);
    int getfirstblock(string file);
    int addblock(string file, string block);
    int delblock(string file, int blocknumber);
    int readblock(string file, int blocknumber, string& buffer);
    int writeblock(string file, int blocknumber, string buffer);
    int nextblock(string file, int blocknumber);
    bool checkblock(string file, int blocknumber);
    int getblock(string file, int blocknumber,string &buffer);
    vector<string> block(string buffer, int b);
    vector<string> ls();
    friend class Shell;
    friend class Table;
    
    private :
    int fssync();                   
    string buffer;
    int rootsize;                   
    int fatsize;                    
    vector<string> filename;        
    vector<int> firstblock;         
    vector<int> fat;                
};
class Shell: public Filesys
{
public:
    Shell(string filename, int blocksize, int numberofblocks);
    int dir();              
    int add(string file);   
    int del(string file);   
    int type(string file);  
    int copy(string file1, string file2);
    string filename;
    string file_name;
};
class Table: public Filesys
{
    public :
        Table(string diskname,int numberofblocks,int blocksize, string flatfile, string indexfile);
        int Build_Table(string input_file);
        int Search(string value);

    private :
        string flatfile;
        string indexfile;
        int numberofrecords;
        int IndexSearch(string value);
};
Sdisk::Sdisk(string disk)
{
    diskname = disk + ".dat";
    diskname1 = disk + ".spc";
    ifstream ifile(diskname1.c_str());
    
    if(ifile.good())
    {
        ifile >> numberofblocks >> blocksize;
        ifile.close();
    }
    
    else
    {
        cout << "Was unable to open the file" << diskname.c_str() <<endl;
    }
}


Sdisk::Sdisk(string disk, int numberofblocks, int blocksize)
{
    this->diskname = disk + ".dat";
    this->diskname1 = disk + ".spc";
    this->numberofblocks = numberofblocks;
    this->blocksize = blocksize;
    fstream spcfile;
    fstream datfile;
    spcfile.open((this->diskname1).c_str(),ios::in | ios::out);
    datfile.open((this->diskname).c_str(),ios::in | ios::out);
    
    if (spcfile.good() && datfile.good())
    {
        cout << "The disk named: " << diskname.c_str() << " exists and is now ready to be written to." << endl;
    }
    else 
    {
        cout << "The disk: " << diskname.c_str() << "could not be found. " << endl;
        cout << "Both the SPC and DAT file were not found. Creating both now. Please wait...." << endl;
        spcfile.open((this->diskname1).c_str(),ios::out);
        datfile.open((this->diskname).c_str(),ios::out);
        spcfile << numberofblocks << " " << blocksize;
        cout << "The SPC file " << diskname.c_str() << " was created" << endl;
        cout << "The DAT file " << diskname.c_str() << " was created" << endl;
        
        for (int i=0; i<numberofblocks*blocksize; i++)
        {
            datfile.put('#');           
        }
    }
    spcfile.close();
    datfile.close();
    return;
}

int Sdisk::getblock(int blocknumber,string& buffer)
{
    bool good = 0;
    fstream checkfile;
    checkfile.open((this->diskname).c_str(), ios::in | ios::out);
    checkfile.seekp(blocksize * blocknumber,ios::beg);
    if (checkfile.bad())
    {
        cout << "Cannot open the file" << endl;
    }
    else
    {
        for (int i = 0; i < blocksize;i++)
        {
            char y = (char) checkfile.get();
            buffer = buffer + y;
        }
        good = 1;
    }
    checkfile.close();
    return good;
}



int Sdisk::putblock(int blocknumber, string buffer)
{
    bool good = 0;
    fstream checkfile;
    checkfile.open((this->diskname).c_str(), ios::in|ios::out);
    if (checkfile.bad())
    {
        cout << "Cannot open the file" << endl;
    }
    else
    {
        fstream iofile;
        iofile.open((this->diskname).c_str());
        checkfile.seekp(blocksize * blocknumber,ios::beg);
        for (int i=0; i < buffer.size() ;i++)
        {
            checkfile.put(buffer[i]);
            good = 1;
        }
    }
    checkfile.close();
    return good;
}
Filesys::Filesys(string diskname, int numberofblocks, int blocksize):Sdisk(diskname, numberofblocks, blocksize)
{
    rootsize = getblocksize()/12;
    fatsize = (getnumberofblocks()*5) / (getblocksize())+1;
    
    
    for(int i=0; i<rootsize; i++)
    {
        filename.push_back("XXXXXX");
        firstblock.push_back(0);
    }
    
    int k = getnumberofblocks();
    fat.push_back(fatsize + 2);
    
    for (int i = 0; i <= fatsize; i++)
    {
        fat.push_back(0);
        
    }
    
    for(int i = fatsize + 2; i < k; i++)
        
    {
        fat.push_back(i+1);
    }
    fat[fat.size()-1] = 0;
    fssync();
}




int Filesys::fsclose()
{
    fssync();
    return 0;
}




int Filesys::newfile(string file)
{
    bool empty = false;
    
    for (int i= 0; i < filename.size(); i++)
    {
        if (filename[i] == file)
        {
            cout << "File (" << file << ") already exists!" << endl;
            
            ifstream ifile(file.c_str());
            ifile.close();
            return 1;
        }
    }
    for (int i = 0; i < filename.size(); i++)
    {
        if( filename[i] == "XXXXXX")
        {
            
            empty = true;
            filename[i] = file;
            firstblock[i] = 0;
            break;
        }
    }
    
    if(!empty)
    {
        cout << "No empty slots in root directory.\n";
        return 0;
    }
    fssync();
    cout << "Successfully added file: " << file << endl;
    return 1;
    

}


int Filesys::rmfile(string file)
{
    int x= getfirstblock(file);
    
    if(x != 0)
    {
        cout << "File could not be removed. " << endl;
        return 0;
    }
    else
    {
        for(int i=0; i < rootsize; i++)
        {
            if(filename[i]==file)
            {
                cout << "filename rmfile" << endl;
                filename[i]="XXXXXX";
            }
        }
    }
    fssync();
    return 1;
}



int Filesys::getfirstblock(string file)
{
    bool found = false;
    int first_block = 0;
    
    for (int i = 0; i < filename.size(); i++)
    {
        if (filename[i] == file)
        {
            found = true;
            first_block = firstblock[i];
            break;
        }
    }
    if(!found)
    {
        cout << "getFirstBlock(): File was not found!\n";
        return -1;
    }
    fssync();
    return first_block;
}





int Filesys::addblock(string file, string block)
{
    int id = getfirstblock(file);
    int allocate = fat[0];
    bool check = false;
    
    if (allocate == 0)
    {
        cout << "No space available" << endl;
        return -1;
    }
    
    if (allocate == 0)
    {
        cout << "Disk is full" << endl;
        return -1;
    }
    
    if (id == 0)
    {
        for (int i = 0; i < filename.size(); i++)
        {
            if (filename[i] == file)
            {
                
                firstblock[i] = allocate;
                fat[0] = fat[allocate];
                fat[allocate] = 0;
                check = true;
                break;
            }
        }
        if (!check)
        {
            cout << "addBlock(): The file wasn't found." << endl;
            return -2;
        }
    }
    else
    {
        int nextblock = id;
        
        while (fat[nextblock] != 0)
        {
            nextblock = fat[nextblock];
        }
        fat[nextblock] = allocate;
        fat[0] = fat[allocate];
        fat[allocate] = 0;
    }
    
    putblock(allocate, block);
    fssync(); 
    return allocate;
}

int Filesys::delblock(string file, int blocknumber)
{
    int first_block = getfirstblock(file);
    if(checkblock(file,blocknumber) == 0)
    {
        return 0;
    }
    for (int i = 0; i < filename.size(); i++)
    {
        if(firstblock[i] == blocknumber)
        {
            firstblock[i] = fat[blocknumber];
            fat[blocknumber] = fat[0];
            fat[0] = blocknumber;
            
            fssync();
            return 1;
        }
    }
    
    for(int i = 0; i < fat.size();i++)
    {
        if(fat[i] == blocknumber)
        {
            fat[i] = fat[blocknumber];
            fat[blocknumber] = fat[0];
            fat[0] = blocknumber;
            break;
        }
    }
    fssync();
    return 1;
}



int Filesys::readblock(string file, int blocknumber, string& buffer)
{
    Sdisk::getblock(blocknumber,buffer);
    return 1;
}



int Filesys::writeblock(string file, int blocknumber, string buffer)
{
    putblock(blocknumber,buffer);
    return 1;
}



int Filesys::nextblock(string file, int blocknumber)
{
    int blockid = getfirstblock(file);
    while (blockid != blocknumber)
    {
        blockid = fat[blockid];
    }
    return fat[blockid];
}

vector<string> Filesys::block(string buffer, int b)
{

    vector<string> blocks;
    
    int numberofblocks = 0;
    string tempblock;
    
    if (buffer.size() % b == 0)
    {
        numberofblocks = (int) (buffer.size()/b);
    }
    else
    {
        numberofblocks = (int) ((buffer.size()/b ) +1);
    }
    
    for (int i=0; i<numberofblocks; i++)
    {
        tempblock=buffer.substr((unsigned long) (b*i), (unsigned long) b);
        blocks.push_back(tempblock);
    }
    int lastblock= (int) (blocks.size()-1);
    
    for (int i= (int) blocks[lastblock].size(); i < b; i++)
    {
        blocks[lastblock]+="#";
    }
    return blocks;
}



int Filesys::fssync()
{
    
    ostringstream fatstream;
    string fatbuffer;
    
    for(int i = 0; i < getnumberofblocks(); i++)
    {
        fatstream << fat[i]<< " ";
        fatbuffer = fatstream.str();
    }
    
    vector <string> blockbuff = block(fatbuffer, getblocksize());
    
    for (int i=0; i < blockbuff.size(); i++)
    {
        
        putblock(2 + i, blockbuff[i]);
    }
    
    ostringstream rootstream;
    string buffer;
    
    for( int i = 0; i < rootsize; ++i )
    {
        rootstream << filename[i] << " " << firstblock[i] << " ";
        buffer = rootstream.str();
    }
    putblock(1, buffer);
    return 0;
}



bool Filesys::checkblock(string file, int blocknumber)
{
    int blockid = getfirstblock(file);
    if (blockid == -1)
    {
        cout << "File does not exist";
        return false;
    }
    else
    {
        while (blockid != 0)
        {
            if (blockid == blocknumber)
            {
                return true;
            }
            blockid = fat[blockid];
            return false;
        }
    }
    return true;
}


int Filesys::getblock(string file, int blocknumber,string &buffer)
{
    Sdisk::getblock(blocknumber, buffer);
    return 1;
}


vector<string> Filesys::ls()
{
    vector<string> filelist;
    for(int i = 0; i < filename.size(); ++i)
    {
        if (filename[i] != "XXXXXX")
        {
            filelist.push_back(filename[i]);
        }
    }
    return filelist;
}

int Shell::dir()
{
    vector<string> flist = ls();
    for (int i = 0; i < flist.size(); i++)
    {
        cout << flist[i] << endl;
    }
    return 1;
}

Shell::Shell(string filename, int blocksize, int numberofblocks):Filesys(filename,blocksize,numberofblocks)
{
}


int Shell::add(string file)    
{
    int err = newfile(file);
    
    if (err == -1)
    {
        return -1;
    }
    
    string contains;
    int blocknumber =0;
    cout << "Input file contents: " << endl;
    
    getline(cin,contains);
    
    vector<string> blocks = block(contains, 128);
    
    for(int i =0; i < blocks.size(); i++)
    {
        blocknumber = addblock(file, blocks[i]);
    }
    
    return 1;
    
}


int Shell::del(string file)  
{
    
    int currentblock = getfirstblock(file);
    while (currentblock > 0)
    {
        delblock(file, currentblock);
        currentblock = getfirstblock(file);
    }
    rmfile(file);
    
    return 1;
}

int Shell::type(string file) 
{
    int currentblock = getfirstblock(file);
    while(currentblock > 0)
    {
        string buffer;
        readblock(file, currentblock, buffer);
        currentblock = nextblock(file, currentblock);
        
        if(currentblock == 0)
        {
            while(buffer.back() == '#')
            {
                buffer = buffer.substr(0,buffer.length() - 1);
            }
            cout << buffer;
        }
        cout << endl;
    }
    
    return 1;
}

int Shell::copy(string file1, string file2) 
{
    del(file2); 
    newfile(file2);
    int currentblock = getfirstblock(file1);
    if (currentblock == -1)
    {
        return 0;
    }
    
    while(currentblock > 0)
    {
        string buffer;
        readblock(file1, currentblock, buffer);
        addblock(file2, buffer);
        currentblock = nextblock(file1, currentblock);
    }
    return 1;
}

Table::Table(string diskname,int numberofblocks,int blocksize, string flatfile, string indexfile):Filesys(diskname,blocksize,numberofblocks)
{
    this->indexfile = indexfile;
    this->flatfile = flatfile;
    newfile(flatfile);
    newfile(indexfile);
}



int Table::Build_Table(string input_file)
{
    string block, record, date;
    unsigned long x;
    vector<string> blocks;
    stringstream ss;
    ifstream file(input_file.c_str());
    
    if(!file.good())
    {
        cout << "build_table(): Error could not open input file\n";
        return -1;
    }
    
    
    getline(file, record);
    while(!file.eof())
    {
        x = record.find("*");
        date = record.substr(0,x);
        x = date.find(" ");
        if(x != string::npos)
            date = date.substr(0,x);
        
        ss << date << " " ;
        
        int flatfile_block_num = addblock(flatfile,record);
        ss << flatfile_block_num << " ";
        addblock(indexfile, ss.str());
        block.clear();
        ss.str("");
        ss.clear();
        
        getline(file, record);
    }
    return 1;
  
}
        
        

int Table::Search(string value)
{
    
    string buffer, date, end, place, type, ref, desc;
    vector <string> rec;
    
    
    signed long block_number = IndexSearch(value), lastPos, pos;
    
    if(block_number == -1)
    {
        cout << "The record could not be found. " << endl;
        return -1;
    }
    getblock(flatfile,block_number,buffer);
    
    lastPos = buffer.find_first_not_of("*",0);
    pos = buffer.find_first_of("*", lastPos);
    
    while (pos != string::npos || lastPos != string::npos)
    {
        rec.push_back( buffer.substr(lastPos, pos-lastPos));
        lastPos = buffer.find_first_not_of("*", pos);
        pos = buffer.find_first_of("*", lastPos);
    }
    
    cout << "Record found: " << endl;
    cout << "Date: " << rec.at(0) << endl;
    cout << "End: " << rec.at(1) << endl;
    cout << "Type: " << rec.at(2) << endl;
    cout << "Place: " << rec.at(3) << endl;
    cout << "Reference: " << rec.at(4) << endl;
    cout << "Description: " << rec.at(5) << endl;
    
    return 0;
}

        
int Table::IndexSearch(string value)
{
    //root, value;
    
    int current_block = getfirstblock(indexfile), blk_num;

    

    while( current_block != 0)
    {
        string block, date;
        stringstream ss;
        getblock(indexfile, current_block, block);
        ss.str(block);
        ss >> date >> blk_num;
        
        if(date == value)
        {
            cout << "blk_num " << blk_num << endl;
            return blk_num;
        }
        
        current_block = nextblock(indexfile, current_block);
    }
    
    return -1;
    return current_block;
}
int main()
{
	Table TBL("disk1", 256, 128, "flatfile","indexfile");
	TBL.Build_Table("data.txt");
	//Table.Build_Table("data.txt");

	while(true)
		{
			cout << "Input the date you wish to search for: ";
			string x;
			cin >> x;
			TBL.Search(x);
			TBL.fsclose();
		}
	return 0;
}
