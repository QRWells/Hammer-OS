RootDirSectors          equ     0xe
SectorNumOfRootDirStart equ     0x13
SectorNumOfFAT1Start    equ     0x1
SectorBalance           equ     0x11

OEMName             db      "HMOSBOOT"
BytesPerSector      dw      0x200
SectorsPerCluster   db      0x1
ReservedSectors     dw      0x1
FATs                db      0x2
RootDirEntries      dw      0xe0
Sectors             dw      0xb40
MediaType           db      0xf0
FATSectors          dw      0x9
SectorsPerTrack     dw      0x12
Heads               dw      0x2
HiddenSectors       dd      0
TolSec32            dd      0
DriveNumber         db      0
Reserved            db      0
BootSignature       db      0x29
VolumeId            dd      0
VolumeLabel         db      "BOOT LOADER"
FATTypeLabel        db      "FAT12   "