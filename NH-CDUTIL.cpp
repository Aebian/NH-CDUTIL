// nethavn - CDROM UTIL Main FIle

#include <Windows.h>
#include <string>
#include <iostream>
#include <vector>
#include <atlstr.h>

#define SCSI_IOCTL_DATA_OUT             0 // Give data to SCSI device (e.g. for writing)
#define SCSI_IOCTL_DATA_IN              1 // Get data from SCSI device (e.g. for reading)
#define SCSI_IOCTL_DATA_UNSPECIFIED     2 // No data (e.g. for ejecting)

#define MAX_SENSE_LEN 18 //Sense data max length 
#define IOCTL_SCSI_PASS_THROUGH_DIRECT  0x4D014 

typedef unsigned short  USHORT;
typedef unsigned char   UCHAR;
typedef unsigned long   ULONG;
typedef void* PVOID;

typedef struct _SCSI_PASS_THROUGH_DIRECT
{
    USHORT Length;
    UCHAR ScsiStatus;
    UCHAR PathId;
    UCHAR TargetId;
    UCHAR Lun;
    UCHAR CdbLength;
    UCHAR SenseInfoLength;
    UCHAR DataIn;
    ULONG DataTransferLength;
    ULONG TimeOutValue;
    PVOID DataBuffer;
    ULONG SenseInfoOffset;
    UCHAR Cdb[16];
}
SCSI_PASS_THROUGH_DIRECT, * PSCSI_PASS_THROUGH_DIRECT;

typedef struct _SCSI_PASS_THROUGH_DIRECT_AND_SENSE_BUFFER
{
    SCSI_PASS_THROUGH_DIRECT sptd;
    UCHAR SenseBuf[MAX_SENSE_LEN];
}
T_SPDT_SBUF;

int GetDriveStatus(TCHAR* nDrive)
{
    std::string mDrive = nDrive;
    if (mDrive.contains(":"))
    {
        mDrive = mDrive.erase(mDrive.size() - 1);
    }
    std::string dvdDriveLetter = mDrive.append(":");

    if (dvdDriveLetter.empty()) return -1;

    std::string strDvdPath = "\\\\.\\"
        + dvdDriveLetter;

    HANDLE hDevice;               // handle to the drive to be examined     
    int iResult = -1;             // results flag
    ULONG ulChanges = 0;
    DWORD dwBytesReturned;
    T_SPDT_SBUF sptd_sb;          //SCSI Pass Through Direct variable.  
    byte DataBuf[8];            //Buffer for holding data to/from drive.   

    hDevice = CreateFile(strDvdPath.c_str(),               // drive                        
        0,                                // no access to the drive                        
        FILE_SHARE_READ,                  // share mode                        
        NULL,                             // default security attributes                        
        OPEN_EXISTING,                    // disposition                        
        FILE_ATTRIBUTE_READONLY,          // file attributes                        
        NULL);

    // If we cannot access the DVD drive 
    if (hDevice == INVALID_HANDLE_VALUE)
    {
        return -1;
    }

    // A check to see determine if a DVD has been inserted into the drive only when iResult = 1.  
    // This will do it more quickly than by sending target commands to the SCSI
    iResult = DeviceIoControl((HANDLE)hDevice,              // handle to device                             
        IOCTL_STORAGE_CHECK_VERIFY2, // dwIoControlCode                             
        NULL,                        // lpInBuffer                             
        0,                           // nInBufferSize                             
        &ulChanges,                  // lpOutBuffer                             
        sizeof(ULONG),               // nOutBufferSize                             
        &dwBytesReturned,           // number of bytes returned                             
        NULL);                      // OVERLAPPED structure   

    CloseHandle(hDevice);

    // Don't request the tray status as we often don't need it      
    if (iResult == 1)  return 2;

    hDevice = CreateFile(strDvdPath.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_READONLY,
        NULL);

    if (hDevice == INVALID_HANDLE_VALUE)
    {
        return -1;
    }

    sptd_sb.sptd.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
    sptd_sb.sptd.PathId = 0;
    sptd_sb.sptd.TargetId = 0;
    sptd_sb.sptd.Lun = 0;
    sptd_sb.sptd.CdbLength = 10;
    sptd_sb.sptd.SenseInfoLength = MAX_SENSE_LEN;
    sptd_sb.sptd.DataIn = SCSI_IOCTL_DATA_IN;
    sptd_sb.sptd.DataTransferLength = sizeof(DataBuf);
    sptd_sb.sptd.TimeOutValue = 2;
    sptd_sb.sptd.DataBuffer = (PVOID) & (DataBuf);
    sptd_sb.sptd.SenseInfoOffset = sizeof(SCSI_PASS_THROUGH_DIRECT);
    sptd_sb.sptd.Cdb[0] = 0x4a;
    sptd_sb.sptd.Cdb[1] = 1;
    sptd_sb.sptd.Cdb[2] = 0;
    sptd_sb.sptd.Cdb[3] = 0;
    sptd_sb.sptd.Cdb[4] = 0x10;
    sptd_sb.sptd.Cdb[5] = 0;
    sptd_sb.sptd.Cdb[6] = 0;
    sptd_sb.sptd.Cdb[7] = 0;
    sptd_sb.sptd.Cdb[8] = 8;
    sptd_sb.sptd.Cdb[9] = 0;
    sptd_sb.sptd.Cdb[10] = 0;
    sptd_sb.sptd.Cdb[11] = 0;
    sptd_sb.sptd.Cdb[12] = 0;
    sptd_sb.sptd.Cdb[13] = 0;
    sptd_sb.sptd.Cdb[14] = 0;
    sptd_sb.sptd.Cdb[15] = 0;

    ZeroMemory(DataBuf, 8);
    ZeroMemory(sptd_sb.SenseBuf, MAX_SENSE_LEN);

    //Send the command to drive - request tray status for drive 
    iResult = DeviceIoControl((HANDLE)hDevice,
        IOCTL_SCSI_PASS_THROUGH_DIRECT,
        (PVOID)&sptd_sb,
        (DWORD)sizeof(sptd_sb),
        (PVOID)&sptd_sb,
        (DWORD)sizeof(sptd_sb),
        &dwBytesReturned,
        NULL);

    CloseHandle(hDevice);

    if (iResult)
    {
        if (DataBuf[5] == 0) iResult = 0;        // DVD tray closed    
        else if (DataBuf[5] == 1) iResult = 1;   // DVD tray open  
        else return iResult = 2;                   // DVD tray closed, media present  
    }

    return iResult;
}

void BayUtility(TCHAR* drive, DWORD command)
{
    // Not used here, only for debug
    MCIERROR mciError = 0;

    // Flags for MCI command
    DWORD mciFlags = MCI_WAIT | MCI_OPEN_SHAREABLE |
        MCI_OPEN_TYPE | MCI_OPEN_TYPE_ID | MCI_OPEN_ELEMENT;

    // Open drive device and get device ID
    MCI_OPEN_PARMS mciOpenParms = { 0 };
    mciOpenParms.lpstrDeviceType = (LPCTSTR)MCI_DEVTYPE_CD_AUDIO;
    mciOpenParms.lpstrElementName = drive;
    mciError = mciSendCommand(0,
        MCI_OPEN, mciFlags, (DWORD_PTR)&mciOpenParms);

    // Eject or close tray using device ID
    MCI_SET_PARMS mciSetParms = { 0 };
    mciFlags = MCI_WAIT | command; // command is sent by caller
    mciError = mciSendCommand(mciOpenParms.wDeviceID,
        MCI_SET, mciFlags, (DWORD_PTR)&mciSetParms);

    // Close device ID
    mciFlags = MCI_WAIT;
    MCI_GENERIC_PARMS mciGenericParms = { 0 };
    mciError = mciSendCommand(mciOpenParms.wDeviceID,
        MCI_CLOSE, mciFlags, (DWORD_PTR)&mciGenericParms);
}

void BayHole(DWORD driveAction)
{
    int nPos = 0;
    UINT nCount = 0;
    TCHAR szDrive[4];
    strcpy_s(szDrive, "?:\\");

    DWORD dwDriveList = ::GetLogicalDrives();

    while (dwDriveList) {
        if (dwDriveList & 1)
        {
            szDrive[0] = 0x41 + nPos;
            if (::GetDriveType(szDrive) == DRIVE_CDROM)
                BayUtility(szDrive, driveAction);
        }
        dwDriveList >>= 1;
        nPos++;
    }
}

void BayControl(int bNum, TCHAR* nDrive)
{
    switch (bNum)
    {
    case 0:
        BayUtility(nDrive, MCI_SET_DOOR_OPEN);
        break;
    case 1:
        BayUtility(nDrive, MCI_SET_DOOR_CLOSED);
        break;
    case 2:
        BayHole(MCI_SET_DOOR_OPEN);
        break;
    case 3:
        BayHole(MCI_SET_DOOR_CLOSED);
        break;
    default:
        std::cout << "No Argument Specified";
    }
}

int main(int argc, TCHAR* argv[])
{

    std::vector<std::string> args;

    for (int i = 0; i < argc; ++i)
        args.emplace_back(argv[i]);

    if (argc == 1)
    {
        std::cout << "Not enough arguments passed, available arguments are: open, close, status, dynamic";
    }

    if (std::count(args.begin(), args.end(), "open"))
    {
        if (argv[2])
        {
            BayControl(0, argv[2]);
            std::cout << "Drive Bay has been opened";
        }
        else
        {
            std::cout << "No drive specified, opening all drive bays";
            BayControl(2, 0);
        }
    }

    if (std::count(args.begin(), args.end(), "close"))
    {
        if (argv[2])
        {
            BayControl(1, argv[2]);
            std::cout << "Drive Bay has been closed";
        }
        else
        {
            std::cout << "No drive specified, closing all drive bays";
            BayControl(3, 0);
        }
    }

    if (std::count(args.begin(), args.end(), "dynamic"))
    {
        if (argc == 3)
        {
            if (GetDriveStatus(argv[2]) == 0 or 2)
            {
                BayControl(0, argv[2]);
            }
            else if (GetDriveStatus(argv[2]) == 1)
            {
                BayControl(3, 0);
            }
        }
        else
        {
            std::cout << "The dynamic function needs a drive letter to operate.";
        }
    }

    if (std::count(args.begin(), args.end(), "status"))
    {
        if (argc == 3)
        {
            std::cout << GetDriveStatus(argv[2]);
        }
        else
        {
            std::cout << "A drive letter is needed to show you the status.";
        }
    }
}