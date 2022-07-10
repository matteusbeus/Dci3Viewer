#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tools.h"
#include "hw_md.h"
#include "cdfh.h"

#define MAX_ENTRIES 256

extern unsigned short bitmap;

typedef struct
{
    int offset;
    int length;
    char flags;
    char name[247];
} entry_t;

entry_t gDirTable[MAX_ENTRIES];

short gCount = 0;
short gFirst = 0;
short gCurrent = 0;
short gStage = 0;

char *temp = (char *)0x0C0000;

//----------------------------------------------------------------------

int getCurrDir(void)
{
    int r = 0;
    int i = 0;

    while (!r)
    {
        r = next_dir_entry();
        if (!r && memcmp(global_vars->DENTRY_NAME, ".", 2))
        {
            gDirTable[i].offset = global_vars->DENTRY_OFFSET;
            gDirTable[i].length = global_vars->DENTRY_LENGTH;
            gDirTable[i].flags = global_vars->DENTRY_FLAGS >> 8;
            strcpy(gDirTable[i].name, global_vars->DENTRY_NAME);
            i++;
        }

        if (r == ERR_NO_MORE_ENTRIES)
            r = next_dir_sec();
    }

    return i;
}

void show_image_animation()
{

    unsigned short y, w, h, fileNo;
    unsigned int ticks = 0;
    unsigned short *image;
   
    memset((void *)0x0C0000, 0, 198*240*2); // clear frame buffer
    switch_banks();

    unsigned short *buf = (unsigned short *)0x200000; // word ram on MD side
    send_md_cmd2(MD_CMD_DMA_SCREEN, (int)buf, 1);

    fileNo = 1;

    while (fileNo < gCount)
    {
        
        unsigned short *src;
        unsigned short *dst = (unsigned short *)0x0C0000; // word ram on CD side (in 1M mode)

        if (strstr(gDirTable[fileNo].name, ".DCI3") != 0) {

            image = (unsigned short *)malloc(gDirTable[fileNo].length + 2047);

            memset(image, 0, gDirTable[fileNo].length + 2047);

            read_cd(gDirTable[fileNo].offset, (gDirTable[fileNo].length + 2047) >> 11, (void *)image);
            
        } else {

            if (strstr(gDirTable[fileNo].name, ".LZ4W") != 0) {

                unsigned short *imageCompressed = (unsigned short *)malloc(gDirTable[fileNo].length + 2047);
            
                image = (unsigned short *)malloc((160*200*2) + 4);

                memset(imageCompressed, 0, gDirTable[fileNo].length + 2047);

                memset(image, 0, (160*200*2) + 4);

                read_cd(gDirTable[fileNo].offset, (gDirTable[fileNo].length + 2047) >> 11, (void *)imageCompressed);

                lz4w_unpack(imageCompressed, image);

                free(imageCompressed);

            } else {

                sprintf(temp, "unrecognised image format\n");
                switch_banks();
                do_md_cmd4(MD_CMD_PUT_STR, 0x200000, TEXT_RED, 1, 24);
                while (1);

            }

        }

        // Load X frames to Prog RAM      
        w = image[0];
        h = image[1];
        src = &image[2];      
        
        // clear bitmap
        memset(dst, 0, 198*240*2); 
        
        // offset to center display
        dst += 4 + (224 - h) / 2 * 198 + (160 - w) / 2; 

        // copy to Word RAM
        for (y=0; y<h; y++)
            memcpy(&dst[y*198], &src[y*w], w*2);

        // wait for vblank
        while (ticks >= GET_TICKS) ;
        ticks = GET_TICKS;

        // switch Word RAM bank
        switch_banks();

        free(image);

        // increment frame
        fileNo++;

    }
    
    while (!(GET_PAD(0) & SEGA_CTRL_BUTTONS)) ;
    wait_md_cmd();
    while (GET_PAD(0) & SEGA_CTRL_BUTTONS) ;

    gStage = 0;
    
}

void show_image()
{
    
    unsigned short *image;
    unsigned short *src;
    unsigned short *buf = (unsigned short *)0x200000; // word ram on MD side
    unsigned short *dst = (unsigned short *)0x0C0000; // word ram on CD side (in 1M mode)
    unsigned short y, w, h;

    if (strstr(gDirTable[gCurrent].name, ".DCI3") != 0) {

        image = (unsigned short *)malloc(gDirTable[gCurrent].length + 2047);

        memset(image, 0, gDirTable[gCurrent].length + 2047);

        read_cd(gDirTable[gCurrent].offset, (gDirTable[gCurrent].length + 2047) >> 11, (void *)image);
        
    } else {

        if (strstr(gDirTable[gCurrent].name, ".LZ4W") != 0) {

            unsigned short *imageCompressed = (unsigned short *)malloc(gDirTable[gCurrent].length + 2047);
           
            image = (unsigned short *)malloc((160*200*2) + 4);

            memset(imageCompressed, 0, gDirTable[gCurrent].length + 2047);

            memset(image, 0, (160*200*2) + 4);

            read_cd(gDirTable[gCurrent].offset, (gDirTable[gCurrent].length + 2047) >> 11, (void *)imageCompressed);

            lz4w_unpack(imageCompressed, image);

            free(imageCompressed);

        } else {

            sprintf(temp, "unrecognised image format\n");
            switch_banks();
            do_md_cmd4(MD_CMD_PUT_STR, 0x200000, TEXT_RED, 1, 24);
            while (1);

        }

    }

    w = image[0];
    h = image[1];
    src = &image[2];

    memset(dst, 0, 198*240*2); // clear bitmap

    // offset to center display
    dst += 4 + (224 - h) / 2 * 198 + (160 - w) / 2; 

    for (y=0; y<h; y++)
        memcpy(&dst[y*198], &src[y*w], w*2);    
    
    switch_banks();
    
    send_md_cmd2(MD_CMD_DMA_SCREEN, (int)buf, 1);

    while (!(GET_PAD(0) & SEGA_CTRL_BUTTONS)) ;
    wait_md_cmd();
    while (GET_PAD(0) & SEGA_CTRL_BUTTONS) ;

    free(image);

    gStage = 0;

}

int main(void)
{
    int i, j;
    unsigned short buttons, changed, previous = 0;

    strcpy(temp, "DCI3 Viewer");
    switch_banks();
    do_md_cmd4(MD_CMD_PUT_STR, 0x200000, TEXT_WHITE, 15, 1);

    init_cd(); // init CD
    set_cwd("/"); // set current directory to root

    gCount = getCurrDir();

    while (1)
    {

        if (gStage == 0) 
        {

            char *p = temp;

            // update display with directory entries
            memset(p, 0, 20*64*2); // 20*64*2 = 2560 bytes?

            if (gCount)
                for (i = 0; i < 20; i++)
                {
                    if ((gFirst + i) >= gCount)
                        break;

                    p = &temp[i*64*2 + 2]; // left justified
                    if (gDirTable[gFirst + i].flags & 2)
                    {
                        // directory
                        *p++ = ((gFirst + i) == gCurrent) ? TEXT_WHITE >> 8 : TEXT_GREEN >> 8;
                        *p++ = '[' - 0x20;
                        for (j = 0; j < 36; j++)
                        {
                            if (gDirTable[gFirst + i].name[j] == '\0')
                                break;
                            *p++ = ((gFirst + i) == gCurrent) ? TEXT_WHITE >> 8 : TEXT_GREEN >> 8;
                            *p++ = gDirTable[gFirst + i].name[j] - 0x20;
                        }
                        *p++ = ((gFirst + i) == gCurrent) ? TEXT_WHITE >> 8 : TEXT_GREEN >> 8;
                        *p++ = ']' - 0x20;
                    }
                    else
                    {
                        for (j = 0; j < 38; j++)
                        {
                            if (gDirTable[gFirst + i].name[j] == '\0')
                                break;
                            *p++ = ((gFirst + i) == gCurrent) ? TEXT_WHITE >> 8 : TEXT_GREEN >> 8;
                            *p++ = gDirTable[gFirst + i].name[j] - 0x20;
                        }
                    }
                }

            switch_banks();

            sub_delay(1); // wait until vblank to update vram

            i = do_md_cmd3(MD_CMD_COPY_VRAM, 0xC000 + 3*64*2, 0x200000, 20*64); // update plane A name table (20 lines of text)

            if (i < 0)
            {
                if (i == -1)
                    strcpy(temp, "unknown command");
                else if (i == -2)
                    strcpy(temp, "wait cmd done timeout");
                else if (i == -3)
                    strcpy(temp, "wait cmd ack timeout");
                else
                    strcpy(temp, "unknown error");
                switch_banks();
                do_md_cmd4(MD_CMD_PUT_STR, 0x200000, TEXT_RED, 1, 24);
                sub_delay(120);
                strcpy(temp, "                                       ");
                switch_banks();
                do_md_cmd4(MD_CMD_PUT_STR, 0x200000, TEXT_GREEN, 1, 24);
            }

            sub_delay(7);
            
            gStage = 1;
        
        } 
        else if (gStage == 1) 
        {

            buttons = GET_PAD(0);

            changed = (buttons & SEGA_CTRL_BUTTONS) ^ previous;

            if (buttons & SEGA_CTRL_DOWN)
            {
                // DOWN pressed
                gCurrent++;
                if ((gCurrent - gFirst) >= 20)
                    gFirst = gCurrent;

                if (gCurrent >= gCount)
                    gFirst = gCurrent = 0; // wrap around to first entry

                gStage = 0;
            }

            if (buttons & SEGA_CTRL_UP)
            {
                // UP pressed
                gCurrent--;
                if (gCurrent < gFirst)
                {
                    gFirst = gCurrent - 19;
                    if (gFirst < 0)
                        gFirst = 0;
                }

                if (gCurrent < 0)
                {
                    gCurrent = gCount - 1; // wrap around to last entry
                    gFirst = gCurrent - 19;
                    if (gFirst < 0)
                        gFirst = 0;
                }

                gStage = 0;
            }

            if (!changed)
                continue; // no change in buttons
                
            previous = buttons & SEGA_CTRL_BUTTONS;

            if (changed & SEGA_CTRL_A)
            {
                if (!(buttons & SEGA_CTRL_A))
                {
                    // A just released
                    if (gDirTable[gCurrent].flags & 2)
                    {
                        // directory
                        global_vars->CWD_OFFSET = gDirTable[gCurrent].offset;
                        global_vars->CWD_LENGTH = gDirTable[gCurrent].length;
                        global_vars->CURR_OFFSET = -1;
                        first_dir_sec();
                        gCount = getCurrDir();
                        gFirst = gCurrent = 0;
                        gStage = 0;
                    }
                    else
                    {
                        show_image();
                    }
                }
            } 

            if (changed & SEGA_CTRL_B)
            {
                if (!(buttons & SEGA_CTRL_B))
                {
                    // A just released
                    if (gDirTable[gCurrent].flags & 2)
                    {
                        // directory
                        global_vars->CWD_OFFSET = gDirTable[gCurrent].offset;
                        global_vars->CWD_LENGTH = gDirTable[gCurrent].length;
                        global_vars->CURR_OFFSET = -1;
                        first_dir_sec();
                        gCount = getCurrDir();
                        gFirst = gCurrent = 0;
                        gStage = 0;
                    }
                    else
                    {
                        show_image_animation();
                    }
                }
            } 
            
        } 

    }

    return 0;
}

