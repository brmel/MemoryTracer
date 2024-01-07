//=========================================================================================================
// Author : Brahim Redouane Mellah
// Year   : 2023
//
//=========================================================================================================
#include "main.h"

std::map<Z_STRING, Z_STRING> MapDevicePathToDrivePathCache;

//===============================================================================================================
Z_STRING MapDevicePathToDrivePath(const Z_STRING& path)
	{
	// NT API Support:
	//   5.0  GetLogicalDriveStrings
	//   5.0  QueryDosDevice

	if(MapDevicePathToDrivePathCache.size() == 0)
		{
		// Construct the cache of device paths to drive letters (e.g.
		// "\Device\HarddiskVolume1\" -> "C:\", "\Device\CdRom0\" -> "D:\").
		Z_STRING drives(27, '\0');
		Z_UINT32 DrivesSize = (Z_UINT32)drives.size();
		Z_INT drives_length = GetLogicalDriveStrings(DrivesSize, &*drives.begin());
		if(drives_length)
			{
			drives.resize(drives_length);
			Z_STRING::size_type start = 0;
			Z_STRING::size_type end = drives.find(_T('\0'));
			while(end < drives.size())
				{
				Z_STRING drive = drives.substr(start, end - start - 1);
				Z_STRING device(MAX_PATH, '\0');
				Z_UINT32 DeviceSize = (Z_UINT32)device.size();
				Z_INT device_length = QueryDosDevice(drive.c_str(), &*device.begin(), DeviceSize);
				if(device_length)
					{
					device.resize(device_length - 2);
					device += '\\';
					drive += '\\';
					MapDevicePathToDrivePathCache[device] = drive;
					}
				else
					{
					std::tcerr << "QueryDosDevice(" << drive << ") failed: " << std::hex << std::setw(8) << std::setfill(_T('0')) << GetLastError() << std::endl;
					}
				start = end + 1;
				end = drives.find(_T('\0'), start);
				}
			}
		else
			{
			std::tcerr << "GetLogicalDriveStrings failed: " << std::hex << std::setw(8) << std::setfill(_T('0')) << GetLastError() << std::endl;
			}
		}

	// Replace a matching device path with the appropriate drive letter.
	for(std::map<Z_STRING, Z_STRING>::iterator map = MapDevicePathToDrivePathCache.begin(); map != MapDevicePathToDrivePathCache.end(); map++)
		{
		if(path.compare(0, (*map).first.size(), (*map).first) == 0)
			{
			return (*map).second + path.substr((*map).first.size());
			}
		}

	// No match, maybe it doesn't have a device path, or maybe we don't know
	// about that drive (possibly only mounted to a directory).
	return path;
	}