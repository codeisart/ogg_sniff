#include <cstdint>
#include <stdio.h>
#include <vector>
#include <inttypes.h>

#pragma pack(1)
struct OggPage
{
    uint32_t capture_pattern;		// OggS
    uint8_t stream_structure_version;	
    uint8_t header_type_flag;
    uint64_t absolute_granule_position;
    uint32_t stream_serial_number;
    uint32_t page_sequence_number;
    uint32_t page_checksum;
    uint8_t page_segments;
};

#pragma pack()

int main(int argc, char** argv)
{
    if(argc < 2)
    {
	printf("Please supply an ogg file.\n");
	return -1;
    }
    if(FILE* f=fopen(argv[1],"rb"))
    {
	OggPage page;
	while(fread(&page,sizeof(OggPage),1,f) == 1)
	{
	    if(page.capture_pattern != 'SggO' && page.stream_structure_version==0)
	    {
		printf("Not a OGG file\n");
		break;
	    }

	    std::vector<uint8_t> segment_table;
	    segment_table.resize(page.page_segments);
	    fread(segment_table.data(), 1, page.page_segments,f);

	    bool bContinuationFlag = page.header_type_flag&1;
	    bool bBOS = page.header_type_flag&2;
	    bool bEOS = page.header_type_flag&4;

	    int TotalSegmentSizes=0;
	    int PacketCount=0;
	    for(size_t i=0;i<page.page_segments;++i)
	    {
		TotalSegmentSizes+=segment_table[i];
		PacketCount+=segment_table[i]!=255? 1:0;
	    }
	    printf("Page=%u\tChecksum=%u\tGranule=%" PRIu64 "\tPacketCount=%d\tBOS=%d\tEOS=%d\tContinuation=%d\tSize=%d\n", 
		    page.page_sequence_number, page.page_checksum,
		    page.absolute_granule_position, PacketCount,
		    bBOS, bEOS, bContinuationFlag,
		    TotalSegmentSizes+sizeof(OggPage)+segment_table.size()
		    
	    );
	    fseek(f,TotalSegmentSizes, SEEK_CUR);
	}
	fclose(f);
    }
    return 0;
}

