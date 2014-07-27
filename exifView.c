#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct
{
    unsigned short SOF;
    unsigned short APP1_Marker;
    unsigned short len_of_APP1;
    char exif_str[4];
    unsigned short null_term;
    char IIorMM[2];
    unsigned short version_number;
    unsigned int offst_to_exif;
} EXIF_TAG;


typedef struct
{
    unsigned short tag_iden;
    unsigned short data_type;
    unsigned int num_of_items;
    unsigned int valueOffset;
} TIFF_TAG;

typedef struct
{
    unsigned int num;
    unsigned int denom;
} FRACT;



void printExif(EXIF_TAG *);
void printTiff(TIFF_TAG *);


int main(int argc, const char * argv[])
{

    if(argc != 2) {
        printf("Invalid input.");
        exit(0);
    }



    FILE *fp;

    fp = fopen(argv[1], "rb");
    if(fp == NULL) printf("Cannot find file. Try again.\n");

    EXIF_TAG pic;
    fread(&pic, 20, 1, fp);


    if(pic.APP1_Marker != 0xe1ff)
    {
        printf("APP1 Marker is misaligned in file. We do not support files with APP0 sections.\n");
        exit(0);
    }

    if(pic.IIorMM[0] != 0x49 && pic.IIorMM[1] != 0x49)
    {
        printf("We do not support endianness. Bummer.\n");
        exit(0);
    }

    if(pic.exif_str[0] != 0x45 && pic.exif_str[1] != 0x78 && pic.exif_str[2] != 0x69 && pic.exif_str[3] != 0x66)
    {
        printf("I can't find the EXIF string. Please validate that you are using the correct file.\n");
        exit(0);
    }

    short count;
    fread(&count, 2,1,fp);


    TIFF_TAG myTags[count];
    fread(myTags, sizeof(myTags),1,fp);


    for(int i = 0; i < count; i++)
    {
        int stringLen =  myTags[i].num_of_items;
        char theString[stringLen + 1]; // Create a string big enough to fit the entire string and 1 null character.

        switch(myTags[i].tag_iden)
        {
            case 0x010F:
                fseek(fp, myTags[i].valueOffset + 12, SEEK_SET);

                fread(theString, stringLen-1,1,fp); // Read the string into the array

                theString[stringLen] = '\0'; // INSERT NULL CHARACTER

                printf("Manufacturer: \t%s\n",theString);
                break;

            case 0x0110:
                fseek(fp, myTags[i].valueOffset + 12, SEEK_SET);

                fread(theString, stringLen,1,fp); // Read the string into the array

                theString[stringLen] = '\0'; // INSERT NULL CHARACTER

                printf("Model: \t\t%s\n",theString);
                break;
        }

        if(myTags[i].tag_iden == 0x8769)
        {
            fseek(fp, myTags[i].valueOffset + 12, SEEK_SET);
            break;
        }

    }

    fread(&count, sizeof(short), 1, fp);

    TIFF_TAG arr[count];
    fread(arr, sizeof(arr), 1, fp);

    for(int i = 0; i < count; i++)
    {
        switch (arr[i].tag_iden)
        {
            case 0xA002: //0xA002 4 (32-bit integer) Width in pixels
                printf("Width: \t\t%d pixels\n", arr[i].valueOffset);
                break;

            case 0xA003: //0xA003 4 (32-bit integer) Height in pixels
                printf("Height: \t%d pixels\n", arr[i].valueOffset);
                break;

            case 0x8827: //0x8827 3 (16-bit integer) ISO speed
                printf("ISO: \t\tISO %d\n", arr[i].valueOffset);
                break;

            case 0x829A: //0x829a 5 (fraction of 2 32-bit unsigned integers) Exposure speed
                fseek(fp, arr[i].valueOffset + 12, SEEK_SET);
                FRACT exposureSpeed;
                fread(&exposureSpeed, sizeof(FRACT), 1, fp);
                printf("Exposure Speed: %d/%d second\n", exposureSpeed.num, exposureSpeed.denom);

                break;

            case 0x829D: //0x829d 5 (fraction of 2 32-bit unsigned integers) F-stop

                fseek(fp, arr[i].valueOffset + 12, SEEK_SET);
                FRACT fStop;
                fread(&fStop, sizeof(FRACT), 1, fp);

                double toDisplay = ((float)fStop.num)/((float)fStop.denom);

                printf("F-Stop: \tf/%.1f\n", toDisplay);

                break;

            case 0x920A: // 0x920A 5 (fraction of 2 32-bit unsigned integers) Lens focal length

                fseek(fp, arr[i].valueOffset + 12, SEEK_SET);
                FRACT lensFocal;
                fread(&lensFocal, sizeof(FRACT), 1, fp);

                int result = lensFocal.num/lensFocal.denom;

                printf("Focal Length: \t%d mm\n", result);

                break;

            case 0x9003: // 0x9003 2 (ASCII String) Date taken

                fseek(fp, arr[i].valueOffset + 12 , SEEK_SET);

                char theString[arr[i].num_of_items + 1]; // Create a string big enough to fit the entire string and 1 null character.
                fread(theString, arr[i].num_of_items,1,fp); // Read the string into the array

                theString[arr[i].num_of_items] = '\0'; // INSERT NULL CHARACTER

                printf("Date Taken: \t%s\n",theString);
                break;

        }
    }





    fclose(fp);

    return 0;
}


void readStringFromTag(TIFF_TAG *myTiffTag) {

}

void printExif(EXIF_TAG *myExifTag)
{
    printf("-----------------------------");
    printf("SOF:      %x\n"  ,   myExifTag->   SOF);
    printf("APP1:     %x\n"  ,   myExifTag->   APP1_Marker);
    printf("Len:      %x\n"  ,   myExifTag->   len_of_APP1);
    printf("Exif:     %s\n"  ,   myExifTag->   exif_str);
    printf("II or MM: %c%c\n",   myExifTag->   IIorMM[0], myExifTag->IIorMM[1]);
    printf("Version:  %d\n"  ,   myExifTag->   version_number);
    printf("Offset:   %d\n"  ,   myExifTag->   offst_to_exif);
    return;
}

void printTiff(TIFF_TAG *myTiffTag)
{
    printf("-----------------------------\n");
    printf("Tag ID: %x\n",          myTiffTag->   tag_iden);
    printf("Data Type: %x\n",       myTiffTag->   data_type);
    printf("# of Items: %x\n",      myTiffTag->   num_of_items);
    printf("Value of Offset: %x\n", myTiffTag->   valueOffset);
}